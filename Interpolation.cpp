/*
 *  Interpolation.cpp
 *  treemusic
 *
 *  Created by Michael Chinen on 11/23/08.
 *  Copyright 2008 MC. All rights reserved.
 *
 */

#include "TreeNode.h"
#include "Interpolation.h"
#include "MetaInterpolation.h"
#include "handyfuncs.h"
#include <math.h>
#include <assert.h>

#define kSquareTransitSamples 10
#define kMaxPeriodsPerSecond 30
#define kMaxStepsPerSecond 12

Interpolation::Interpolation(int bottomLayer, int topLayer,int numTopLayers,TreeNode* owner)
{
   mOwner=owner;
   mParam =eAmplitude;
   mType=eLinear;
   mEndCoef = 1.0;
   mInterpParams.SetBottomLayer(bottomLayer);
   mInterpParams.SetTopLayer(topLayer);
   mInterpParams.SetNumTopLayers(numTopLayers);
   mInterpParams.AddParameter(eInterpTypeVariable,1.0,1.0,1.0);
   interpTypeVariable=1.0;
}

Interpolation *Interpolation::Clone(TreeNode *owner)
{
   Interpolation* clone = new Interpolation(0, 0, 0,owner);

   clone->mInterpParams = mInterpParams;

   clone->mParam =mParam;
   clone->mType=mType;
   clone->mEndCoef = mEndCoef;
   clone->mStartTime = mStartTime;
   clone->mDuration = mDuration;
   clone->mEndCoef = mEndCoef;
   clone->interpTypeVariable = interpTypeVariable;
   clone->mDoesConcatenate = mDoesConcatenate;

   return clone;
}

//create a similar clone.  Will have a different place in the tree, and therefore different layer status and parent.  
//Also, the clone must not have this one's meta interps, perhaps.  This is still questionable.
Interpolation* Interpolation::SimilarClone(int bottomLayer, int topLayer,int numTopLayers,TreeNode* owner)
{
   Interpolation* similarClone = new Interpolation(bottomLayer, topLayer, numTopLayers,owner);

   similarClone->mParam =mParam;
   similarClone->mType=mType;
   similarClone->mEndCoef = mEndCoef;
   similarClone->mStartTime = mStartTime;
   similarClone->mDuration = mDuration;
   similarClone->mEndCoef = mEndCoef;
   return similarClone;
}

typedef struct {
   std::vector<double> instantParams;
   bool instantParamsCleared = true;
} InterpolationMetaData;

//at start time it should retun 1.0 at end time mEndCoef
//the second parameter is for recursive calls since this uses incompatable static variables
double Interpolation::ValueAtTime(double time)
{
   double cursor =mymin(mymax(0.0,time-mStartTime)/mDuration,1.0);
   double phase;
   double ret;
    
   __thread static InterpolationMetaData *meta;
    if (!meta) {
        meta = new InterpolationMetaData; /*TODO:leak*/
    }
   //get the list of our local parameters and copy them.
    std::vector<double> &instantParams = meta->instantParams;
   bool &instantParamsCleared = meta->instantParamsCleared;
   
   if(instantParamsCleared)
   {
      for(int i =0;i<mInterpParams.GetNumParameters();i++)
         instantParams.push_back(mInterpParams.GetValue((Parameter)i));
      instantParamsCleared=false;
   }
   else
   {
      for(int i =0;i<mInterpParams.GetNumParameters();i++)
         instantParams[i]=mInterpParams.GetValue((Parameter)i);   
   }
   
   //check to see if we have metaInterps attached to us and apply them.
   for(int i=0;i<mAttachedMetaInterps.size();i++)
   {
	//caution - MetaInterpolation::Apply uses ValueAtTime 
      mAttachedMetaInterps[i]->Apply(time,&instantParams);
   }
   
//   assert(mStartTime <=time);
//   assert(mStartTime+mDuration>=time);
//   


   switch(mType)
   {
      
      case eLinear:
        ret= cursor*(mEndCoef-1.0) + 1.0;
        break;
      case eExp2:
         ret= exp2cursor(cursor)*mEndCoef+(1.0-exp2cursor(cursor));
         break;
	  case eStep:
          ret= (mEndCoef-1.0)*((float)(floor((cursor+0.001) * floor(instantParams[eInterpTypeVariable]))))/floor(instantParams[eInterpTypeVariable]) + 1.0;
          break;
	  case eExp2Step:
          ret= (mEndCoef-1.0)*exp2cursor(((float)(floor((cursor+0.001) * floor(instantParams[eInterpTypeVariable]))))/floor(instantParams[eInterpTypeVariable]))+1.0;
          break;
     case eSquare:
     //need to implement some fading to bandlimit the square.
//         while(phase>2*3.141592)
//            phase-=2*3.141592;
//         if(phase>(2*3.141592*interpTypeVariable) <kSquareTransitSamples/44100.0)
//            return (mEndCoef-1.0) + 1.0;
         phase=modf(     (2 * cursor * (floor(instantParams[eInterpTypeVariable])/**mDuration*/+0.5 ) - 0.5)/2.0,&phase);//we just clober the intpart with the return value
         phase = 3.141592*2* phase;
         ret= phase<3.141592?1.0:mEndCoef;
         break;
     
     case ePeriodic:
		//sinusoid that starts at -1 (-pi/2) and continues for (n+0.5)periods to end up at +1 (pi/2)
		//we also shrink it down soo that -pi/2 comes to 1.0 and pi/2 comes to mEndCoef. 
      phase=modf(     (2 * cursor * (floor(instantParams[eInterpTypeVariable])/**mDuration*/+0.5 ) - 0.5)/2.0,&phase);//we just clober the intpart with the return value
      phase = 3.141592*2* phase;
		ret=  (sin(phase)*(mEndCoef-1.0)/2.0)+(mEndCoef-1.0)/2.0 + 1.0;
		break;
      
     default:
       assert(1==0);//shouldn't reach here.
       break;
   }
   return ret;
}

double Interpolation::MaxValueInTimeRange(double startTime,double endTime)
{


   //double cursor =mymin(mymax(0.0,time-mStartTime)/mymax(1.0/44100.0,mDuration),1.0);
   switch(mType)
   {
      case eLinear:
       //  return mymax(mEndCoef,1.0);
        return mEndCoef<1.0?ValueAtTime(startTime):ValueAtTime(endTime);
        break;
      
      case eExp2:
        // return mymax(mEndCoef,1.0);
         return mEndCoef<1.0?ValueAtTime(startTime):ValueAtTime(endTime);
         break;
         
	  case eExp2Step:
      case eStep:
     //    return mymax(mEndCoef,1.0);
         return mEndCoef<1.0?ValueAtTime(startTime):ValueAtTime(endTime);
         break;
         
         
 	   case ePeriodic:
      case eSquare:
   		return mEndCoef<1.0?1.0:mEndCoef;
   }
}

double Interpolation::MinValueInTimeRange(double startTime,double endTime)
{

   //double cursor =mymin(mymax(0.0,time-mStartTime)/mymax(1.0/44100.0,mDuration),1.0);
   switch(mType)
   {
      case eLinear:
        return mEndCoef<1.0?ValueAtTime(endTime):ValueAtTime(startTime);
        break;
      
      case eExp2:
         return mEndCoef<1.0?ValueAtTime(endTime):ValueAtTime(startTime);
         break;
         
	  case eExp2Step:
      case eStep:
         return mEndCoef<1.0?ValueAtTime(endTime):ValueAtTime(startTime);
         break;
         
         
 	   case ePeriodic:
      case eSquare:
   		return mEndCoef<1.0?mEndCoef:1.0;
   }
}

void Interpolation::Invert()
{
   mEndCoef = 1.0/mEndCoef;
}

void Interpolation::Truncate(double startTime, double duration, std::vector<TreeNode*> *activeNodes,  std::vector<ParameterList*> *parameters, int layerNum,  int topLayer, int numControlledLayers)
{
   
   mStartTime = startTime;
	mDuration = duration;
   
   //make sure we fit within all parameter boundries for all associated paramter lists.
   double startValue = (*parameters)[layerNum]->GetValue(mParam);
   //- we don't know where it will be at the end of the leaf.  
   //now check to see where the parameter will be at upon the end of the leaf.  
   //we do this by asking each of the active nodes what their value will be at the end of THIS leaf
   //and multiply them against the start value
   
   double topValue = startValue;
   double bottomValue = startValue;
   
   for(int i =0;i<numControlledLayers;i++)
   {
      topValue = mymax(topValue, (*parameters)[topLayer+i]->GetValue(mParam));
      bottomValue = mymin(bottomValue, (*parameters)[topLayer+i]->GetValue(mParam));
   }
   
   for(int i=0;i<activeNodes->size();i++)
   {
      for(int j=0;j<((*activeNodes)[i])->GetNumInterpolations();j++)
      {
         
         if(((*activeNodes)[i])->GetInterpolation(j)->GetParameter() == mParam)
         {
            //we use this node's start time, and the parent node's end time (since we can't push them to overstep their boundries.)
            topValue = mymax(topValue,topValue*((*activeNodes)[i])->GetInterpolation(j)->MaxValueInTimeRange(mStartTime,((*activeNodes)[i])->GetInterpolation(j)->GetStartTime()+((*activeNodes)[i])->GetInterpolation(j)->GetDuration()));
            bottomValue = mymin(bottomValue,bottomValue*((*activeNodes)[i])->GetInterpolation(j)->MinValueInTimeRange(mStartTime,((*activeNodes)[i])->GetInterpolation(j)->GetStartTime()+((*activeNodes)[i])->GetInterpolation(j)->GetDuration()));
         }
      }
   }
   
   assert(topValue >= bottomValue);
   while(topValue*mEndCoef> (*parameters)[layerNum]->GetMaxValue(mParam))
      mEndCoef= randfloat(mEndCoef-1.0)+1.0;
   
   while(bottomValue*mEndCoef<(*parameters)[layerNum]->GetMinValue(mParam))
      mEndCoef = randfloat(1.0-mEndCoef)+mEndCoef;

   if((topValue*mEndCoef>(*parameters)[layerNum]->GetMaxValue(mParam)) ||
         (bottomValue*mEndCoef<(*parameters)[layerNum]->GetMinValue(mParam)) )
         mEndCoef=1.0;
      
}

//this randomize has to be clever enough to not overstep the min/max
//constraints WHILE the other nodes are interpolating the same parameter
void Interpolation::Randomize(double startTime, double duration, std::vector<TreeNode*> *activeNodes,  std::vector<ParameterList*> *parameters, int layerNum,  int topLayer, int numControlledLayers, int param)
{
	
   mType=(InterpolationType)randint(eNumInterpolationTypes);
   mDoesConcatenate = true;//randint(2);
   
	switch(mType)
	{
		case ePeriodic:
      case eSquare:
      //set the number of periods
			interpTypeVariable = (int)randfloatexp2(mymax(5,duration*kMaxPeriodsPerSecond)) + 0.5; //can't have more periods then we have samples.
         mInterpParams.SetParameter(eInterpTypeVariable, interpTypeVariable,0.001,mymax(5,duration*kMaxPeriodsPerSecond)+0.99);
			break;
         
	  case eExp2Step:
      case eStep:
         interpTypeVariable = randint(mymax(1.0,kMaxStepsPerSecond*duration))+1; //can't have more periods then we have samples.
         mInterpParams.SetParameter(eInterpTypeVariable,interpTypeVariable,interpTypeVariable,interpTypeVariable);
			break;
	}
	mStartTime = startTime;
	mDuration = duration;
   //first decide on a param if none was specified
   if(param==-1)
      mParam=(Parameter)randint((*parameters)[layerNum]->GetNumParameters());
   else
      mParam=(Parameter)param;
   
   //we know where the parameter will be at the start of the leaf 
   //here we get it from the parameter list
   
   
   double startValue = (*parameters)[layerNum]->GetValue(mParam);
   //- we don't know where it will be at the end of the leaf.  
   //now check to see where the parameter will be at upon the end of the leaf.  
   //we do this by asking each of the active nodes what their value will be at the end of THIS leaf
   //and multiply them against the start value.
   
   double topValue = startValue;
   double bottomValue = startValue;
   
   for(int i =0;i<numControlledLayers;i++)
   {
      topValue = mymax(topValue, (*parameters)[topLayer+i]->GetValue(mParam));
      bottomValue = mymin(bottomValue, (*parameters)[topLayer+i]->GetValue(mParam));
   }
   
   for(int i=0;i<activeNodes->size();i++)
   {
      for(int j=0;j<((*activeNodes)[i])->GetNumInterpolations();j++)
      {
         
         if(((*activeNodes)[i])->GetInterpolation(j)->GetParameter() == mParam)
         {
            //we use this node's start time, and the parent node's end time (since we can't push them to overstep their boundries.)
            topValue = mymax(topValue,topValue*((*activeNodes)[i])->GetInterpolation(j)->MaxValueInTimeRange(mStartTime,((*activeNodes)[i])->GetInterpolation(j)->GetStartTime()+((*activeNodes)[i])->GetInterpolation(j)->GetDuration()));
            bottomValue = mymin(bottomValue,bottomValue*((*activeNodes)[i])->GetInterpolation(j)->MinValueInTimeRange(mStartTime,((*activeNodes)[i])->GetInterpolation(j)->GetStartTime()+((*activeNodes)[i])->GetInterpolation(j)->GetDuration()));

            //this makes new nodes never overstep the boundries of what old nodes have reliquished.
            //the above two lines need to be commented out in order for it to work.
            //topValue = mymax(topValue,topValue*((*activeNodes)[i])->GetInterpolation(j)->MaxValueInTimeRange(((*activeNodes)[i])->GetInterpolation(j)->GetStartTime(),((*activeNodes)[i])->GetInterpolation(j)->GetStartTime()+((*activeNodes)[i])->GetInterpolation(j)->GetDuration()));
            //bottomValue = mymin(bottomValue,bottomValue*((*activeNodes)[i])->GetInterpolation(j)->MinValueInTimeRange(((*activeNodes)[i])->GetInterpolation(j)->GetStartTime(),((*activeNodes)[i])->GetInterpolation(j)->GetStartTime()+((*activeNodes)[i])->GetInterpolation(j)->GetDuration()));

         }
      }
   }
   
   assert(topValue >= bottomValue);
   //now decide if we are going to interpolate UP or down.
   bool up = coinflip();
    
   if(up)
   {
      if(mParam == eBandwidth || mParam==eBreathPressure || mParam == eBlowPosition || mParam == eReedStiffness ||mParam == eMultiInstNum ||
              mParam== eReedAperature || mParam ==eVibratoGain || mParam ==eNoiseGain || mParam ==eMaxBlowLength || mParam ==eInstrumentNum)
//               mEndCoef = 1.0+randfloat( mymax(0.0,parameters->GetMaxValue(mParam)/mymax(0.000001,mymax(startValue,endValue)) -1.0));
         mEndCoef = 1.0+randfloat( mymax(0.0,(*parameters)[layerNum]->GetMaxValue(mParam)/mymax(0.000001,topValue) -1.0));
      else
//         mEndCoef = 1.0+randfloatexp2( mymax(0.0,parameters->GetMaxValue(mParam)/mymax(0.000001,mymax(startValue,endValue)) -1.0));
         mEndCoef = 1.0+randfloatexp2( mymax(0.0,(*parameters)[layerNum]->GetMaxValue(mParam)/mymax(0.000001,topValue) -1.0));

   assert(topValue*mEndCoef<=(*parameters)[layerNum]->GetMaxValue(mParam)*1.02);
   }
   else
   {
	  //use the minimum to find out where the coefficient should be.  
	  //mEnd Coef can be from the ratio of min to the end, up to 1.0 
     
      if(mParam == eBandwidth || mParam==eBreathPressure || mParam == eBlowPosition || mParam == eReedStiffness || mParam == eMultiInstNum ||
         mParam== eReedAperature || mParam ==eVibratoGain || mParam ==eNoiseGain || mParam ==eMaxBlowLength || mParam ==eInstrumentNum)
         mEndCoef = 1.0- randfloat( 1.0-mymin(1.0,mymax(0.0,(*parameters)[layerNum]->GetMinValue(mParam)/mymax(0.000001,bottomValue))));
//         mEndCoef = mymax(0.0,parameters->GetMinValue(mParam)/mymax(0.000001,bottomValue))+ randfloat( 1.0-mymax(0.0,parameters->GetMinValue(mParam)/mymax(0.000001,bottomValue)));   
      else
         mEndCoef = 1.0- randfloatexp2( 1.0-mymin(1.0,mymax(0.0,(*parameters)[layerNum]->GetMinValue(mParam)/mymax(0.000001,bottomValue))));
//         mEndCoef = mymax(0.0,parameters->GetMinValue(mParam)/mymax(0.000001,bottomValue))+ randfloatexp2( 1.0-mymax(0.0,parameters->GetMinValue(mParam)/mymax(0.000001,bottomValue)));   

   assert(bottomValue*mEndCoef>=(*parameters)[layerNum]->GetMinValue(mParam)*0.98);
   }
   //fix boundries.
   //if(topValue*mEndCoef>=parameters->GetMaxValue(mParam))
   //   mEndCoef = parameters->GetMaxValue(mParam)/topValue;
  // else if(bottomValue*mEndCoef<=parameters->GetMinValue(mParam))
    //  mEndCoef=parameters->GetMinValue(mParam)/bottomValue;
   
   //we might have caused a conflict on the other side, if we did, this guy can't interpolate, and must stay at 1.0
   //it can have local non concatenating fluctuations, however.
   if((topValue*mEndCoef>=(*parameters)[layerNum]->GetMaxValue(mParam)) ||
         (bottomValue*mEndCoef<=(*parameters)[layerNum]->GetMinValue(mParam)) )
         mEndCoef=1.0;
         
//   assert(topValue*mEndCoef<=(*parameters)[layerNum]->GetMaxValue(mParam)*1.02);
 //  assert(bottomValue*mEndCoef>=(*parameters)[layerNum]->GetMinValue(mParam)*0.98);
   
   
}

void Interpolation::AddMetaInterpolation(MetaInterpolation* metaInterp)
{
   mAttachedMetaInterps.push_back(metaInterp);
}

void Interpolation::ConcatAndRemoveMetaInterpolation(MetaInterpolation* metaInterp)
{
   //first remove from the array
   for(int i=0;i<mAttachedMetaInterps.size();i++)
   {
      if(mAttachedMetaInterps[i]==metaInterp)
      {
         mAttachedMetaInterps.erase(mAttachedMetaInterps.begin()+i);
         break;
      }
   }
   mInterpParams.AddCoefficient(metaInterp->GetParameter(), metaInterp->GetEndCoefficient(), 
            metaInterp->GetOwner()->GetLayerNumber(),metaInterp->GetOwner()->GetTopLayer(),metaInterp->GetOwner()->GetNumTopLayers());
}
