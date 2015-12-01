/*
This class/program creates musical structures using something similar to a binary tree.  

I will explain how what the tree means and how it is turned into music.  

The following describes how a tree node is created.  (the process is recursive, so describing one node describes how the entire tree is made.

-Let P be a node in a binary tree at some depth.  It corresponds to the interpolation of the coefficient of a single parameter over a duration of time. 
 The start and end coefficient values are determined randomly within a range.  There are many types of parameters, including some that affect this process,
 such as the parameter range.  Other parameters could be things such as amplitude, grain length, grain size, center frequency, bandwidth, etc. 
-Let R be the root of the tree.
-P is assigned a layer number by it's creator.  The root's layer number is 0.  
-P can have 0, or 2 child nodes that have the same layer number.  If the depth is less than a predetermined minumum depth, it must have 2 child nodes. 
   -The children, Pchild1 and Pchild2, are assigned a random parameter and interpolation coefficients.  Pchild1 occupies the space from P's start point, 
    until some randomly determined time BEFORE P's end time.  Pchild2 start at Pchild1's end time until P's end time.  Thus the children do not overstep the 
    time boundries of the parents.
   -If a random dice check is passed one of these children can be constructed to be the clone of a subtree of R that does not include P.
   -P should have 0 children if it is beyond a minimum depth and a random check OR sufficient termination contstraints have been met, OR if it is beyond a maximum depth.
-P can also has a flag called canCreateLayers.  If this flag is true, then P can create 2 children with a layer number higher than it, but this will only happen 
 if a random check is passed.  It can happen more than once, so that P creates for example, 8 children (2 of its own and 6 at 3 new layers.) 
 These children will be assigned consecutive layer numbers that are higher than the original.  The number of layers should also be a interpolatable parameter.
 -If P creates one or more layers, it should set canCreateLayers of all children to false.
 -The new layers can be thought of as parallel trees that have the same structure of tree up until this node. 

      //new as of 1/9/2009
      //we also have to decide how many extra layers to give away to this layer.  We subtract this number plus 1 for the current
      //layer from our mNumLayersToCreate, and we increment the top layer number as well so we don't have our layers' layers 
      //generating the same layer number as the current layer.  Confusing?  
      
      //this allows it so that layers can have relationships with other layers than just the first.  There still are constraints,
      //since a higher layer can never generate a lower layer, but this is alright.


Here is an example of a tree with only one layer: (it is hard to draw multi layers with text)

|---------------------------------------------------------------------------------------------------------------------------------------------------------|
                                                                     (Root) [CF]
|-----------------------------------------------------------------------------------------------|---------------------------------------------------------|
                        [Amp]                                                                                           [len]
|-------------|---------------------------------------------------------------------------------|-------------------------------------|-------------------|
      [CF]                                   [stretch]                                                      [amp]                            [BW]
|----|--------|-----------------|---------------------------------------------------------------|                                     |---------|---------|
 [N]   [BW]        [Mutation]                      [periodicity]                                                                       [stretch]  [CF]
     |---|----|-------|---------|                                                                                                     |-----|---|
      [N] [BW]  [Amp]   [len]                                                                                                          [amp][CF]


In practice a real tree would go down many more levels, in the range of 20 to 100.  The layers of tree provide structural relationships.  It is interesting
to note that although a tree layer has it's low height areas identical to it's parent's tree's, the actual sounds and structures produced will be markedly different
because of the cumulative nature of tree.  

Note that many parameters appear twice. in the hierarchy - this is not problematic, and it is why coefficients were chosen instead of concrete numbers - if there are
two interpolations going on simultaneously, you just multiply them together to find the final coefficient.

Each layer of the tree is then processed depth first.  At the leaves you will have a interpolated value for each of the synthesis parameters.  We pass these parameters
through a synthesizer at each of the leaves.  

The synthesizer function uses granular synthesis and my genesynth output to extract pure tones and noise, and to truncate frequencies, etc.  I will place some restrictions
on min and max frquencies, so that it is always truncating frequency components outside of a small range.  This range will change throughout the piece, from one end to another.

I intend to generate a tree randomly, and then inspect and alter it manually (at the higher levels).

Q:
Won't the thing just end up sounding like noise if the tree has a large height?
A:
It may not.  The structure of the binary tree will be heard - for example, at the first division under the root, EVERY parameter will change.  in other less
important places, only one or two things are changed.  At a high enough height, the synthesizer may not be able to respond fast enough.
*/

#ifndef __TREENODE_H__
#define __TREENODE_H__

#include <stdio.h>
#include <ctime>
#include <vector>

#include "Interpolation.h"
class TreeNode;
class TreeSynthesizer;


#define kMaxLayers 12


typedef struct {
   TreeNode*   left;
   TreeNode*   right;

} ChildLayer;

class TreeNode
{
public:
   //initialize a tree at a layer number.
   TreeNode(TreeNode* parent, int layerNumber=0);   
   virtual ~TreeNode();
   
   //finds a peer (same depth) that is probably close but possibly in a diff. layer.
   TreeNode* GetNearbyPeer();
   int RandomWalkUp(TreeNode** resultHandle, int distanceTraveled=0);
   TreeNode* RandomWalkDown(int distance);
   
   void Generate(double startTime=0.0, double duration=100.0, std::vector<ParameterList*> *parameters=NULL,int depth=0,  std::vector<TreeNode*> *activeNodes=NULL);
   
   void Synthesize(TreeSynthesizer* synth, ParameterList* parameters, int layerNumber=0, std::vector<TreeNode*> *activeNodes=NULL);
   
   //to be called from root:
   void PrintFile(const char *fname, int stime);
   //for convenience only - no loading for this format.
   void PrintLayerFile(const char *fname, int stime);
   //spatial grid - for viewing pleasure only
   void PrintGridFile(const char *fname, int stime);

   void Randomize(double startTime, double duration, std::vector<TreeNode*> *activeNodes, std::vector<ParameterList*> *parameters,int depth);
   
   void BecomeDeepCopy(TreeNode* target,double startTime, double duration,std::vector<ParameterList*> *parameters,std::vector<TreeNode*> *activeNodes, int depth,int copyDepth=-1);


   //returns the highest number layer in the tree
   int GetMaxLayerNum();
   //returns the highest (largest,deepest) depth in the tree 
   int GetHeight(int depth =0);
   
   Interpolation* GetInterpolation(int index){return &mInterpolations[index];}
   int   GetNumInterpolations();
   
   void ResetParameters();
   
   int GetLayerNumber(){return mLayerNumber;}
   int GetNumTopLayers(){return mNumLayersToControl;}
   int GetTopLayer(){return mTopLayer;}
   
private:
   //used on children from PrintFile.   
	void PrintFileHelper(FILE* fptr, int depth=0);
   void PrintLayerFileHelper(FILE* fptr, int layer, int depth=0);
   void PrintGridFileHelper(FILE* fptr, int layer, float start, float end, char** grid, int gridlen, int depth=0);


	void LoadFileHelper(FILE* fptr);

   TreeNode*   mParent;
   std::vector<ChildLayer> mChildLayers;
   std::vector<Interpolation> mInterpolations;
   std::vector<MetaInterpolation*> mMetaInterps;
   float       mMidPoint;
   int         mLayerNumber;
   int         mNumLayersToCreate;
   int         mNumLayersToControl; //the total amount of layers this tree node can or has created
   int         mTopLayer;//the next layer this node would create
   
   bool        mIsRefNode;
   TreeNode*    mReferant;
};




#endif