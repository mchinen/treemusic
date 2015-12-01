/*
 *  TreeNode.cpp
 *  treemusic
 *
 *  Created by Michael Chinen on 10/29/08.
 *  Copyright 2008 . All rights reserved.
 *
 */

#include "TreeNode.h"
#include "TreeSynthesizer.h"
#include "handyfuncs.h"
#include "MetaInterpolation.h"

//defaults.
#define kMinDepth 10

#define kMaxDepth 69
#define kMakeChildProbability 0.952
#define kCreateLayerProbability 0.11
#define kCopyNearbyPeerProb 0.15
#define kDeepCopyPeerProb 0.05
#define kCreateMetaInterpProb 0.35
//in seconds:
#define kMinNodeDuration 0.1
#define kAvgInterpsPerNode 1.0
#define kFinalizeSamples 44100

/*
   TreeNode* mParent;
   vector<ChildLayer*> mChildLayers;
   float       mMidPoint;
   Parameter   mParam;
   int         mLayerNumber;
   */
//initialize a tree at a layer number.
TreeNode::TreeNode(TreeNode* parent,int layerNumber)
: mParent(parent),
  mMidPoint(0.5),
  mLayerNumber(layerNumber),
  mNumLayersToCreate(0),
  mNumLayersToControl(0),
  mTopLayer(0),
  mIsRefNode(false)
{
}
  
//delete this node and all of it's children.
TreeNode::~TreeNode()
{
   for(int i=0;i<mChildLayers.size();i++)
   {
      if(mChildLayers[i].left)
         delete mChildLayers[i].left;
      if(mChildLayers[i].right)
         delete mChildLayers[i].right;
   }
}

//reset internal param lists.
void TreeNode::ResetParameters()
{
   for(int i=0;i<mInterpolations.size();i++)
      mInterpolations[i].GetInterpParameters()->ResetCoefficients();
   for(int i=0;i<mMetaInterps.size();i++)
      mMetaInterps[i]->GetInterpParameters()->ResetCoefficients();
   
   for(int i=0;i<mChildLayers.size();i++)
   {
      mChildLayers[i].left->ResetParameters();
      mChildLayers[i].right->ResetParameters();
   }   
}

TreeNode* TreeNode::GetNearbyPeer()
{
   //we go up a random amount of times
   TreeNode* upNode;
   int distanceUp = RandomWalkUp(&upNode);
   
   //go down a similar distance.  With luck we won't select ourselves, but if we do it is alright.
   return upNode->RandomWalkDown(distanceUp);
   
}

//helper for up part.  Returns the number of nodes up we went.  Returns the target by setting result
//distanceTraveled is an internal default parameter that should not be used by the client.
int TreeNode::RandomWalkUp(TreeNode** resultHandle, int distanceTraveled)
{
   //we want to ensure we go at least one.
   if(mParent&& (coinflip() || !distanceTraveled))
   {
      return mParent->RandomWalkUp(resultHandle,distanceTraveled+1);
   }
   else
   {
      *resultHandle = this;
      return distanceTraveled;
   }
}

//helper for down part.  We take a specified number of down traversals unless we hit a leaf.
TreeNode* TreeNode::RandomWalkDown(int distance)
{
   if(distance && mChildLayers.size())
   {
      //select a random layer.
      if(coinflip())
         return mChildLayers[randint(mChildLayers.size())].left->RandomWalkDown(distance-1);
      else
         return mChildLayers[randint(mChildLayers.size())].right->RandomWalkDown(distance-1);
   }
   
   return this;
}



//generate a tree.   
//the top layer represents the next layer to generate.  
//we don't want to have only layer 0 as the root, so 
void TreeNode::Generate(double startTime, double duration, std::vector<ParameterList*> *parameters,int depth,  std::vector<TreeNode*> *activeNodes)
{
   //add this node to the active list while we process it's children.
   bool root = activeNodes == NULL;
   static double totalTime = 10;
//   static float startFreq;
   if(root)
   {
	  activeNodes = new (std::vector<TreeNode*>);
     totalTime=duration;
//     startFreq = (*parameters)[0]->GetValue(eCenterFrequency);
     mNumLayersToControl=  mNumLayersToCreate=kMaxLayers-1;
     
     mLayerNumber=0;
     mTopLayer = 1;
     //we also need to add duplicate parameters for each possible layer.
      for(int i=1;i<kMaxLayers;i++)
         parameters->push_back((*parameters)[0]->Clone());
   }
   //a hack to set the min/max to sweep:
//   parameters->SetParameter(eCenterFrequency,startFreq*(exp2cursor(startTime/totalTime)+0.0001),startFreq*(exp2cursor(startTime/totalTime)+0.0001)*1.5*
//               0.85,
//                  startFreq*(exp2cursor(startTime/totalTime)+0.0001)*1.5*1.15);

   
   //first randomize the end coefficient.  
   Randomize(startTime, duration,activeNodes,parameters,depth);
   //if we have a deep copy ref node, then it has already generated children.
   if(!mIsRefNode)
   {  
      //Attach all meta interps.
      for(int i=0;i<mMetaInterps.size();i++)
      {
         mMetaInterps[i]->Attach();
      }
      //add the newly constructed interp node to the list.  NOTE we need to randomize the node first since randomize uses the list.
      activeNodes->push_back(this); 
      
      //Keep Track of the next layer to create (it will move as we create children (and not contiguously))
      int nextTopLayer=mTopLayer;
      while(depth<kMaxDepth && mNumLayersToCreate >0&& randfloat(1.0)<kCreateLayerProbability/**((float)(depth-kMinDepth)/(kMaxDepth-kMinDepth))*/)
      {
         
         ChildLayer layer;
         
         
         layer.left = new TreeNode(this,nextTopLayer);
         layer.right = new TreeNode(this,nextTopLayer++);
         
         //we also have to decide how many extra layers to give away to this layer.  We subtract this number plus 1 for the current
         //layer from our mNumLayersToCreate, and we increment the top layer number as well so we don't have our layers' layers 
         //generating the same layer number as the current layer.  Confusing?  
         
         //this allows it so that layers can have relationships with other layers than just the first.  There still are constraints,
         //since a higher layer can never generate a lower layer, but this is alright.
         
         int extraLayers = randint(mNumLayersToCreate--); // since the randint param is exclusive, we decrement after.
         layer.left->mNumLayersToCreate=layer.right->mNumLayersToCreate=extraLayers;
         layer.left->mNumLayersToControl=layer.right->mNumLayersToControl=extraLayers;
         layer.left->mTopLayer=layer.right->mTopLayer=nextTopLayer;
         
         //the left one is created with a start at the same time as this node, and ends at start plus the duration up to the midpoint.  
         layer.left->Generate( startTime,  duration*mMidPoint, parameters, depth+1, activeNodes);
         layer.right->Generate(startTime+duration*mMidPoint,duration*(1.0-mMidPoint), parameters, depth+1, activeNodes );
         //i believe the whole struct is shallow copied, which is fine since we are talking about pointers.
         mChildLayers.push_back(layer);
         
         //we need to update the top layers for our future children of the same layer so they don't conflict with the layer we made
         mNumLayersToCreate-=extraLayers;
         nextTopLayer+=extraLayers;
      }
      //now create children.  They should be at least half a sample wide
      if(depth<kMinDepth || (duration>kMinNodeDuration&& depth<kMaxDepth && randfloat(1.0)< kMakeChildProbability*(1.0 - ((float)depth/kMaxDepth))))
      {
         ChildLayer layer;
         layer.left = new TreeNode(this,mLayerNumber);
         layer.right = new TreeNode(this,mLayerNumber);
         
         //must pass the num remaining to create info to children so they can create the remaining layers
         layer.left->mNumLayersToCreate=layer.right->mNumLayersToCreate=mNumLayersToCreate;
         layer.left->mNumLayersToControl=layer.right->mNumLayersToControl=mNumLayersToCreate;//children take the adjusted number.
            layer.left->mTopLayer=layer.right->mTopLayer=nextTopLayer;
            
            layer.left->Generate(startTime,  duration*mMidPoint,parameters, depth+1, activeNodes);
            layer.right->Generate(startTime+duration*mMidPoint,duration*(1.0-mMidPoint),parameters, depth+1, activeNodes);
            //i believe the whole struct is shallow copied, which is fine since we are talking about pointers.
            mChildLayers.push_back(layer);
      }
      
      //remove from the active node list and add the parameter
      activeNodes->pop_back();
      //make the cum. changes for this layer
      
      for(int i=0;i<GetNumInterpolations();i++)
      {
         (*parameters)[mLayerNumber]->AddCoefficient(mInterpolations[i].GetParameter(),mInterpolations[i].GetEndCoefficient(),mLayerNumber,mTopLayer,mNumLayersToControl);
      }
      //and additional ones.  We do this for every one of our layers.
      for(int h=0;h<mNumLayersToControl;h++)
      {
         for(int i=0;i<GetNumInterpolations();i++)
            (*parameters)[h+mTopLayer]->AddCoefficient(mInterpolations[i].GetParameter(),mInterpolations[i].GetEndCoefficient(),mLayerNumber,mTopLayer,mNumLayersToControl);
      }
      
         //DEtach all meta interps.
      for(int i=0;i<mMetaInterps.size();i++)
      {
         mMetaInterps[i]->Detach();
      }
   }
   if(root)
	delete activeNodes;
}

//only leaves do the actual synthesizing - other nodes just accumulate parameter coefficients and add to the active list.
void TreeNode::Synthesize(TreeSynthesizer* synth, ParameterList* parameters, int layerNumber, std::vector<TreeNode*> *activeNodes)
{
   //if we reached a leaf, then it's time to synthesize.
   static long int lastSampleWritten = -1;
   static long int totalSamples = 100;
   static int numNodesProcessed =0;
   static int percentComplete =0;
	//active nodes starts off null.  the root should init it.
	bool root = activeNodes==NULL;
   static double totalTime = 10.0;
//   static float startFreq;
   
   static std::vector<double> instantParams;
   static std::vector<double> beginParams;
   static bool instantParamsCleared = true;
   
	if(root)
   {
		activeNodes = new (std::vector<TreeNode*>);
      lastSampleWritten = -1;
      totalSamples = mInterpolations[0].GetDuration()*synth->GetSampleRate();
      percentComplete = numNodesProcessed=0;
      totalTime = mInterpolations[0].GetDuration();
         //a hack to set the min/max to sweep:
  //    startFreq = parameters->GetValue(eCenterFrequency);
      instantParams.clear();
      beginParams.clear();
      instantParamsCleared=true;
      ResetParameters();
   }  
   
   //attach all meta interps.
   for(int i=0;i<mMetaInterps.size();i++)
   {
      mMetaInterps[i]->Attach();
   }
   
	//first add ourself to the ctive lis
   
   activeNodes->push_back(this); 
   
//   parameters->SetParameter(eCenterFrequency,startFreq*(exp2cursor(mInterp.GetStartTime()/totalTime)+0.0001),startFreq*(exp2cursor(mInterp.GetStartTime()/totalTime)+0.0001)*1.5*
//               0.85,
//                  startFreq*(exp2cursor(mInterp.GetStartTime()/totalTime)+0.0001)*1.5*1.15);


   bool isLeaf =true;
   //pass the synth message on to children on our layer (which includes pre-branch zero)
   //note that we always pass the synth message onto the layer number specified
   //in the args.  If it doesn't exist, we pass it on to the zeroth layer (the original)
   //and if the zeroth layer doesn't exist, we have a leaf.
   for(int pass=0;pass<2;pass++)
   {
      for(int i=0;i<mChildLayers.size();i++)
      {
         //We split at either the target layer number, or the parent, which will be the layernumber of the current node.  
         //this ensures parallel structure up to the point of branching.
         //we want to go to the layer number 
         if( (mChildLayers[i].left->mLayerNumber == layerNumber && pass==0 ) ||
             (pass==1 && mChildLayers[i].left->mNumLayersToControl+mChildLayers[i].left->mTopLayer>layerNumber
                  &&    mChildLayers[i].left->mTopLayer<=layerNumber))
                  
         {
            mChildLayers[i].left->Synthesize(synth,parameters,layerNumber,activeNodes);
            mChildLayers[i].right->Synthesize(synth,parameters,layerNumber,activeNodes);
            
            //get out - we only synthesize one layer at a time.
            isLeaf=false;
            goto afterchildrensynth;
         }
      }
   }

afterchildrensynth:
   if(isLeaf)
   {
      //figure out how many samples.
      double sr = 44100.00;
      long int startSample = mInterpolations[0].GetStartTime()*sr;
      long int duration = mInterpolations[0].GetDuration()*sr;
      
      //copy over the params since they will be hashed up during interpolation.
      if(instantParamsCleared)
      {
         for(int i =0;i<parameters->GetNumParameters();i++)
            instantParams.push_back(parameters->GetValue((Parameter)i));
         for(int i =0;i<parameters->GetNumParameters();i++)
            beginParams.push_back(parameters->GetValue((Parameter)i));
         instantParamsCleared=false;
      }
      else
      {
         for(int i =0;i<parameters->GetNumParameters();i++)
         {
            instantParams[i]=parameters->GetValue((Parameter)i);
            beginParams[i]=parameters->GetValue((Parameter)i);
         }
      }
      //first check to see if we've skipped over some samples - we don't want to cause a gap
      //(this could occur when duration is <1)
      while(lastSampleWritten<startSample-1)
      {
         //Synthesize using the given parameters.
         synth->SynthesizeSample(&instantParams);
         lastSampleWritten++;
      }
      //we compute the parameters at every sample and pass them to the synthesizer.
      while(lastSampleWritten<startSample+duration)
      {
         //get the current interp and concat for each node active.
         int numActiveNodes;
         int numInterps;
         int interpParam;
         TreeNode* activeNode;
         Interpolation* interp;
         numActiveNodes=activeNodes->size();
 
         for(int i=0;i<numActiveNodes;i++)
         {
            activeNode = (*activeNodes)[i];
            numInterps = activeNode->GetNumInterpolations();
            for(int j=0;j<numInterps;j++)
            {
               interp = activeNode->GetInterpolation(j);
               interpParam = interp->GetParameter();
               instantParams[interpParam] = beginParams[interpParam] *interp->ValueAtTime(((double)(lastSampleWritten+1))/sr);
            }
         }
         
         //hack to do a freq sweep over the piece.
       //  instantParams[eCenterFrequency]= mymin(22000, instantParams[eCenterFrequency]* (((double)mymax(1,lastSampleWritten)/totalSamples)+0.001)*2.0);

         synth->SynthesizeSample(&instantParams);
         lastSampleWritten++;
      }
      
      while(lastSampleWritten > totalSamples*(percentComplete/100.0))
      {
         printf("%i%% done, %i nodes processed, current depth = %i\n",++percentComplete,numNodesProcessed,(*activeNodes).size());
      }

   }

	
	//remove from the active node list and add the parameter
   activeNodes->pop_back();
   for(int i=0;i<GetNumInterpolations();i++)
   {
      parameters->AddCoefficient(mInterpolations[i].GetParameter(),mInterpolations[i].GetEndCoefficient(),mLayerNumber,mTopLayer,mNumLayersToControl);
   }
   //DEtach all meta interps.
   for(int i=0;i<mMetaInterps.size();i++)
   {
      mMetaInterps[i]->Detach();
   }


   numNodesProcessed++;
   if(root)
	{
		//finalize (clean up audio so the audio doesn't click/clip at the end
		for(int i=0;i<kFinalizeSamples;i++)
			synth->SynthesizeFinalizeSample(&instantParams);		
      delete activeNodes;
	}
}

//not a real deep copy - truncates and inverts.
void TreeNode::BecomeDeepCopy(TreeNode* target,double startTime, double duration,std::vector<ParameterList*> *parameters,std::vector<TreeNode*> *activeNodes, int depth, int copyDepth)
{
   mMidPoint = target->mMidPoint;
   //copy interps.
   for(int i=0;i<target->mInterpolations.size();i++)
   {
      mInterpolations.push_back( *(target->mInterpolations[i].SimilarClone(mLayerNumber,mTopLayer,mNumLayersToControl,this)));//the shallow copy is okay
      if(coinflip())
         mInterpolations[i].Invert();
      mInterpolations[i].Truncate(startTime, duration,activeNodes, parameters,mLayerNumber,mTopLayer,mNumLayersToControl);//fit in parameters.
   }
   //add the newly constructed interp node to the list.  NOTE we need to randomize the node first since randomize uses the list.
   activeNodes->push_back(this); 
   
   //create different layers, maybe.
   int nextTopLayer=mTopLayer;
   while(depth<kMaxDepth && mNumLayersToCreate >0&& randfloat(1.0)<kCreateLayerProbability/**((float)(depth-kMinDepth)/(kMaxDepth-kMinDepth))*/)
   {
         
      ChildLayer layer;
      layer.left = new TreeNode(this,nextTopLayer);
      layer.right = new TreeNode(this,nextTopLayer++);
         
      int extraLayers = randint(mNumLayersToCreate--); // since the randint param is exclusive, we decrement after.
      layer.left->mNumLayersToCreate=layer.right->mNumLayersToCreate=extraLayers;
      layer.left->mNumLayersToControl=layer.right->mNumLayersToControl=extraLayers;
      layer.left->mTopLayer=layer.right->mTopLayer=nextTopLayer;
      
      //the left one is created with a start at the same time as this node, and ends at start plus the duration up to the midpoint.  
      layer.left->Generate( startTime,  duration*mMidPoint, parameters, depth+1, activeNodes);
      layer.right->Generate(startTime+duration*mMidPoint,duration*(1.0-mMidPoint), parameters, depth+1, activeNodes );
      //i believe the whole struct is shallow copied, which is fine since we are talking about pointers.
      mChildLayers.push_back(layer);
      
      //we need to update the top layers for our future children of the same layer so they don't conflict with the layer we made
      mNumLayersToCreate-=extraLayers;
      nextTopLayer+=extraLayers;
   }
   //add children, copied from the target.  Note that we can only take one layer so we do this randomly 
   if(target->mChildLayers.size() && (copyDepth || copyDepth<0))
   {
      int targetLayer = randint(target->mChildLayers.size());
      ChildLayer layer;
      layer.left = new TreeNode(this,mLayerNumber);
      layer.right = new TreeNode(this,mLayerNumber);
      layer.left->mNumLayersToCreate=layer.right->mNumLayersToCreate=mNumLayersToCreate;
      layer.left->mNumLayersToControl=layer.right->mNumLayersToControl=mNumLayersToCreate;
      layer.left->mTopLayer=layer.right->mTopLayer=nextTopLayer;

      //the left one is created with a start at the same time as this node, and ends at start plus the duration up to the midpoint.  
      layer.left->BecomeDeepCopy(target->mChildLayers[targetLayer].left,startTime, duration*mMidPoint,parameters,activeNodes,depth+1,copyDepth-1);
      layer.right->BecomeDeepCopy(target->mChildLayers[targetLayer].right,startTime+duration*mMidPoint,duration*(1.0-mMidPoint),parameters,activeNodes,depth+1,copyDepth-1);
      //i believe the whole struct is shallow copied, which is fine since we are talking about pointers.
      mChildLayers.push_back(layer);

   }
    //remove from the active node list and add the parameter
   activeNodes->pop_back();
   //make the cum. changes for this layer
   
   for(int i=0;i<GetNumInterpolations();i++)
   {
      (*parameters)[mLayerNumber]->AddCoefficient(mInterpolations[i].GetParameter(),mInterpolations[i].GetEndCoefficient(),mLayerNumber,mTopLayer,mNumLayersToControl);
   }
   //and additional ones.  We do this for every one of our layers.
   for(int h=0;h<mNumLayersToControl;h++)
   {
      for(int i=0;i<GetNumInterpolations();i++)
         (*parameters)[h+mTopLayer]->AddCoefficient(mInterpolations[i].GetParameter(),mInterpolations[i].GetEndCoefficient(),mLayerNumber,mTopLayer,mNumLayersToControl);
   }

}

//randomizes non-child components.
//several methods of randomizing.
//1.the simplest - generate a random number of interpolations for unique parameters.
//2.Find a nearby peer (of similar depth) and copy it's interpolations as closely as possible without exceeding parameter limits.
//3.Copy a nearby peer and all its children as closely as possible.  We cannot take layers, however, as this would complicate things too much.
void TreeNode::Randomize(double startTime, double duration, std::vector<TreeNode*> *activeNodes, std::vector<ParameterList*> *parameters, int depth)
{

   Interpolation interp(mLayerNumber,mTopLayer,mNumLayersToControl,this);
   bool usingCopy = false;
   
   
   mMidPoint = randfloat(1.0);
   
      
   //see if we should rely on nearby peers.  Make higher layers more likely.
   if(randfloat(1.0)<kCopyNearbyPeerProb*(mLayerNumber+1)/kMaxLayers)
   {
      TreeNode* target = GetNearbyPeer();
      //make sure we aren't pointing to ourselves
      if(target!=this)
      {
         if(randfloat(1.0)<kDeepCopyPeerProb)
         {
            usingCopy=true;
            //copy all from many nodes.
            //the actual depth is randomized.
            //we are guaranteed target is not an ancestor of this node due to the way RandomWalk works.
            BecomeDeepCopy(target,startTime,duration,parameters,activeNodes, depth);
            mIsRefNode = true;//flag to not generate more children in Generate()
         }
         //steal all from single node
         else
         {
            usingCopy=true;
            for(int i=0;i<target->mInterpolations.size();i++)
            {
               mInterpolations.push_back(*(target->mInterpolations[i].SimilarClone(mLayerNumber,mTopLayer,mNumLayersToControl,this)));
               
               //the shallow copy is okay, except for the stack of attached meta nodes, so we get rid of them.
               //There may be a more elegant solution.  This is hereby noted.
               
               
               //if(coinflip())
               //   mInterpolations[i].Invert();
               mInterpolations[i].Truncate(startTime, duration,activeNodes, parameters,mLayerNumber,mTopLayer,mNumLayersToControl);//fit in parameters.
            }
         }
      }
   }
   
   if(!usingCopy)
   {
      //see if we should interpolate some aspect of a node above us
      //also, our target must be of our layer number in order for this to be possible.  
      if(randfloat(1.0)<kCreateMetaInterpProb && (*activeNodes).size())
      {
         TreeNode* target = (*activeNodes)[(int) (((*activeNodes).size()-1) - randfloatexp2((*activeNodes).size(), 3))];
         if(target->mLayerNumber==mLayerNumber)
         {
            Interpolation* targetInterp = target->GetInterpolation(randint(target->GetNumInterpolations()));
            MetaInterpolation* metaInterp;
            metaInterp = new MetaInterpolation(targetInterp,mLayerNumber,mTopLayer,mNumLayersToControl,this);
            metaInterp->Randomize(startTime, duration,activeNodes, parameters,mLayerNumber,mTopLayer,mNumLayersToControl);
            mMetaInterps.push_back(metaInterp);
         }
      }
      
      //   mParam = (Parameter)randint(eNumParams);
      bool noneAdded = true;   
      
      for(int i=0;i<(*parameters)[0]->GetNumParameters();i++)
      {
         //coinflip for each parameter?
         if(randfloat(1.0)<kAvgInterpsPerNode/(*parameters)[0]->GetNumParameters())
         {
            interp.Randomize(startTime, duration,activeNodes, parameters,mLayerNumber,mTopLayer,mNumLayersToControl, i);
            mInterpolations.push_back(interp);
            noneAdded=false;
         }
      }
      //ensure at least one per node (we access [0] for some functions)
      if(noneAdded)
      {
         interp.Randomize(startTime, duration,activeNodes, parameters, mLayerNumber,mTopLayer,mNumLayersToControl);
         mInterpolations.push_back(interp);
      }
   }
}
int  TreeNode::GetNumInterpolations()
{
   return mInterpolations.size();
}

//to be called from root:
void TreeNode::PrintFile(const char *fname, int stime)
{
   FILE* fptr;
   fptr = fopen(fname,"wt");
		
	fprintf(fptr,"treemusic tree\n");
      fprintf(fptr,"seed=%i\n",stime);
   fprintf(fptr,"kMinDepth=%i, kMaxLayers=%i, kMaxDepth=%i, kMakeChildProbability=%f, kCreateLayerProbability=%f\n",
                  kMinDepth,kMaxLayers,kMaxDepth,kMakeChildProbability,kCreateLayerProbability);

	
	PrintFileHelper(fptr);
	
	fclose(fptr);
	printf("Wrote %s\n",fname);
}

void TreeNode::PrintLayerFile(const char *fname, int stime)
{
   FILE* fptr;
   fptr = fopen(fname,"wt");
		
	fprintf(fptr,"treemusic tree by Layer\n");
	
   int maxlayer = GetMaxLayerNum();
   for(int i=0;i<maxlayer;i++)
   {
      fprintf(fptr,"tree at layer%i:\n",i);
         fprintf(fptr,"seed=%i\n",stime);
   fprintf(fptr,"kMinDepth=%i, kMaxLayers=%i, kMaxDepth=%i, kMakeChildProbability=%f, kCreateLayerProbability=%f\n",
                  kMinDepth,kMaxLayers,kMaxDepth,kMakeChildProbability,kCreateLayerProbability);

      PrintLayerFileHelper(fptr,i);
      fprintf(fptr,"\n\n");
	}
   
	fclose(fptr);
	printf("Wrote %s\n",fname);
}
  
 
 void TreeNode::PrintGridFile(const char *fname, int stime)
{
   FILE* fptr;
   fptr = fopen(fname,"wt");
		
      
	fprintf(fptr,"treemusic tree by grid\n");
   fprintf(fptr,"seed=%i\n",stime);
   fprintf(fptr,"kMinDepth=%i, kMaxLayers=%i, kMaxDepth=%i, kMakeChildProbability=%f, kCreateLayerProbability=%f\n",
                  kMinDepth,kMaxLayers,kMaxDepth,kMakeChildProbability,kCreateLayerProbability);

   char** grid;
   int gridLength=100;
   
   int maxdepth =GetHeight();
   grid = new char*[maxdepth+1];
   
   for(int i=0;i<maxdepth+1;i++)
   {
      grid[i]=new char[gridLength+1];
      for(int j=0;j<gridLength;j++)
         grid[i][j]='0';
      grid[i][gridLength]=0;//null term.
   }
   
   int maxlayer = GetMaxLayerNum();
   for(int i=0;i<maxlayer;i++)
   {
      for(int j=0;j<maxdepth+1;j++)
         for(int k=0;k<gridLength;k++)
            grid[j][k]='0';
 

      fprintf(fptr,"tree at layer %i:\n",i);
      //the helper just fills the grid.
      PrintGridFileHelper(fptr,i,0.0,1.0,grid,gridLength);
      //now we have to print the array
      //first pass over and change all zeros to spaces so the grid is nicer.
      for(int j=0;j<maxdepth+1;j++)
         for(int k=0;k<gridLength;k++)
            if(grid[j][k]=='0') grid[j][k]=' ';
      
      for(int j=0;j<maxdepth+1;j++)
         fprintf(fptr,"%s\n",grid[j]);
      fprintf(fptr,"\n\n");
	}
   
	fclose(fptr);
	printf("Wrote %s\n",fname);
}
 
   
void TreeNode::PrintFileHelper(FILE* fptr, int depth)
{
   //indent
   for(int i=0;i<depth;i++)
      fprintf(fptr,"  ");
   fprintf(fptr,"<node midpoint=%f, interpolations=%i,layer=%i,children=%i,controlledLayers=%i,topLayer=%i>\n", mMidPoint,GetNumInterpolations(),mLayerNumber,mChildLayers.size()*2,mNumLayersToControl,mTopLayer);
   for(int h=0;h<GetNumInterpolations();h++)
   {
      for(int i=0;i<depth;i++)
         fprintf(fptr,"  ");
      fprintf(fptr,"<interpolation param=%i,coef=%f,interpType=%i,startTime=%f,duration=%f,concatenates=%i>\n",mInterpolations[h].GetParameter(),(float)mInterpolations[h].GetEndCoefficient(),mInterpolations[h].GetInterpolationType(),(float)mInterpolations[h].GetStartTime(),(float)mInterpolations[h].GetDuration(),(int)mInterpolations[h].DoesConcatenate());
   }
   for(int i=0;i<depth;i++)
      fprintf(fptr,"  ");
   fprintf(fptr,"</node>");
   for(int i=0;i<mChildLayers.size();i++)
   {
      mChildLayers[i].left->PrintFileHelper(fptr, depth+1);
      mChildLayers[i].right->PrintFileHelper(fptr, depth+1);
   }
}

int TreeNode::GetMaxLayerNum()
{
   int ret = 0;
   for(int i=0;i<mChildLayers.size();i++)
   {
      ret=mymax(ret,mChildLayers[i].left->GetMaxLayerNum());
      ret=mymax(ret,mChildLayers[i].right->GetMaxLayerNum()); 
   }
   return mymax(ret,mLayerNumber); 
}

int TreeNode::GetHeight(int depth)
{
   int ret = 0;
   for(int i=0;i<mChildLayers.size();i++)
   {
      ret=mymax(ret,mChildLayers[i].left->GetHeight(depth+1));
      ret=mymax(ret,mChildLayers[i].right->GetHeight(depth+1)); 
   }
   return mymax(depth,ret); 
}

void TreeNode::PrintLayerFileHelper(FILE* fptr, int layer, int depth)
{
   //indent
  // for(int i=0;i<depth;i++)
//      fprintf(fptr,"  ");
//   fprintf(fptr,"<node midpoint=%f,param=%i,coef=%d,interpType=%i,startTime=%d,duration=%d,layer=%i,children=%i>\n", mMidPoint,mInterp.GetParameter(),mInterp.GetEndCoefficient(),mInterp.GetInterpolationType(),mInterp.GetStartTime(),mInterp.GetDuration(),mLayerNumber,mChildLayers.size()*2);
   for(int i=0;i<depth;i++)
      fprintf(fptr,"  ");
   fprintf(fptr,"<node midpoint=%f, interpolations=%i,layer=%i,children=%i,controlledLayers=%i,topLayer=%i>\n", mMidPoint,GetNumInterpolations(),mLayerNumber,mChildLayers.size()*2,mNumLayersToControl,mTopLayer);
   for(int h=0;h<GetNumInterpolations();h++)
   {
      for(int i=0;i<depth;i++)
         fprintf(fptr,"  ");
      fprintf(fptr,"<interpolation param=%i,coef=%f,interpType=%i,startTime=%f,duration=%f,concatenates=%i>\n",mInterpolations[h].GetParameter(),(float)mInterpolations[h].GetEndCoefficient(),mInterpolations[h].GetInterpolationType(),(float)mInterpolations[h].GetStartTime(),(float)mInterpolations[h].GetDuration(),(int)mInterpolations[h].DoesConcatenate());
   }
   for(int i=0;i<depth;i++)
      fprintf(fptr,"  ");
   fprintf(fptr,"</node>");

   
   
   int closestLayer = -1;
   int closestLayerIndex = -1;
   for(int i=0;i<mChildLayers.size();i++)
   {
      //right now the layer tree has only one split point, because only zero layer can make layers.
      if(mChildLayers[i].left->mLayerNumber > closestLayer && (mChildLayers[i].left->mLayerNumber == 0||mChildLayers[i].left->mLayerNumber==layer))
      {
         closestLayer = mChildLayers[i].left->mLayerNumber;
         closestLayerIndex=i;
      }
   }
   if(closestLayer>=0)
   {
      mChildLayers[closestLayerIndex].left->PrintLayerFileHelper(fptr, layer, depth+1);
      mChildLayers[closestLayerIndex].right->PrintLayerFileHelper(fptr, layer, depth+1);      
   }
}

void TreeNode::PrintGridFileHelper(FILE* fptr, int layer, float start, float end, char** grid, int gridlen, int depth)
{
   grid[depth][(int)(gridlen*mymax(0.0,mymin(1.0,(end+start)/2.0)))]++;
   if(grid[depth][(int)(gridlen*mymax(0.0,mymin(1.0,(end+start)/2.0)))]==':')
       grid[depth][(int)(gridlen*mymax(0.0,mymin(1.0,(end+start)/2.0)))]='A';
	   
   else if(grid[depth][(int)(gridlen*mymax(0.0,mymin(1.0,(end+start)/2.0)))]>'Z'+1)
      grid[depth][(int)(gridlen*mymax(0.0,mymin(1.0,(end+start)/2.0)))]='Z';
   
   
   int closestLayer = -1;
   int closestLayerIndex = -1;
   for(int i=0;i<mChildLayers.size();i++)
   {
      //right now the layer tree has only one split point, because only zero layer can make layers.
      if((mChildLayers[i].left->mLayerNumber > closestLayer) && (mChildLayers[i].left->mLayerNumber==layer ||
           ( mChildLayers[i].left->mNumLayersToControl+mChildLayers[i].left->mTopLayer>layer
                  &&    mChildLayers[i].left->mTopLayer<=layer)))

      
      {
         closestLayer = mChildLayers[i].left->mLayerNumber;
         closestLayerIndex=i;
      }
   }
   if(closestLayer>=0)
   {
      mChildLayers[closestLayerIndex].left->PrintGridFileHelper(fptr, layer, start,start+(end-start)*mMidPoint,grid,gridlen,depth+1);
      mChildLayers[closestLayerIndex].right->PrintGridFileHelper(fptr, layer,start+(end-start)*mMidPoint,end,grid,gridlen,depth+1);
   }
}


void TreeNode::LoadFileHelper(FILE* fptr)
{
   
}
