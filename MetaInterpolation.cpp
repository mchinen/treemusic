/*
 *  MetaInterpolation.cpp
 *  treemusic
 *
 *  Created by Michael Chinen on 2/7/09.
 *  Copyright 2009 Michael Chinen. All rights reserved.
 *
 */

#include "MetaInterpolation.h"
#include "ParameterList.h"
#include "handyfuncs.h"
#include "TreeNode.h"
#include <math.h>
#include <assert.h>

#define kPeriodicMaxFreq 30

MetaInterpolation::MetaInterpolation(Interpolation* target,int bottomLayer, int topLayer,int numTopLayers,TreeNode* owner)
      :Interpolation(bottomLayer, topLayer, numTopLayers, owner)
{
   mTarget=target;
}

MetaInterpolation *MetaInterpolation::Clone(Interpolation *target, TreeNode *owner) {
   MetaInterpolation* clone = new MetaInterpolation(target, 0, 0, 0,owner);

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

void MetaInterpolation::Randomize(double startTime, double duration,std::vector<TreeNode*> *activeNodes, std::vector<ParameterList*> *parameters, int layerNum,  int topLayer, int numControlledLayers, int param)
{

   mType=(InterpolationType)randint(eNumInterpolationTypes);
   mDoesConcatenate = true;//randint(2);

   switch(mType)
	{
		case ePeriodic:
      case eSquare:
			interpTypeVariable = (int)randfloatexp2(mymax(5,duration*kPeriodicMaxFreq)) + 0.5; //can't have more periods then we have samples.
         mInterpParams.SetParameter(eInterpTypeVariable, interpTypeVariable,0.001,mymax(5,duration*kPeriodicMaxFreq)+0.99);
			break;
         
	  case eExp2Step:
      case eStep:
         interpTypeVariable = randint(mymax(1.0,10*duration))+1; //can't have more periods then we have samples.
         mInterpParams.SetParameter(eInterpTypeVariable,interpTypeVariable,interpTypeVariable,interpTypeVariable);
			break;
	}
   mStartTime = startTime;
	mDuration = duration;
   mParam = (Parameter) 0;//only one for now (interpTypeParam)
   
    double startValue = mTarget->GetInterpParameters()->GetValue(mParam);
   //- we don't know where it will be at the end of the leaf.  
   //now check to see where the parameter will be at upon the end of the leaf.  
   //we do this by asking each of the active nodes what their value will be at the end of THIS leaf
   //and multiply them against the start value.
   
   double topValue = startValue;
   double bottomValue = startValue;
   double initialValue = mTarget->GetInterpParameters()->GetInitialValue(mParam);
   for(int i =0;i<numControlledLayers+1;i++)
   {
     //double GetLayerCoef(int layer, int paramIndex);
      topValue = mymax(topValue,initialValue*mTarget->GetInterpParameters()->GetLayerCoef(i<numControlledLayers?i+topLayer:layerNum,mParam));
      bottomValue = mymin(bottomValue, initialValue*mTarget->GetInterpParameters()->GetLayerCoef(i<numControlledLayers?i+topLayer:layerNum,mParam));
   }
   


   for(int j=0;j<mTarget->GetNumAttachedMetaInterpolations();j++)
   {
         
      if(mTarget->GetAttachedMetaInterpolation(j)->GetParameter() == mParam)
      {
         //we use this node's start time, and the parent node's end time (since we can't push them to overstep their boundries.)
         topValue = mymax(topValue,topValue*mTarget->GetAttachedMetaInterpolation(j)->MaxValueInTimeRange(mStartTime,mTarget->GetAttachedMetaInterpolation(j)->GetStartTime()+mTarget->GetAttachedMetaInterpolation(j)->GetDuration()));
         bottomValue = mymin(bottomValue,bottomValue*mTarget->GetAttachedMetaInterpolation(j)->MinValueInTimeRange(mStartTime,mTarget->GetAttachedMetaInterpolation(j)->GetStartTime()+mTarget->GetAttachedMetaInterpolation(j)->GetDuration()));
   assert(topValue*mEndCoef<=mTarget->GetInterpParameters()->GetMaxValue(mParam)*1.02);

      }
   }
   
   
   
   
   assert(topValue >= bottomValue);
   //now decide if we are going to interpolate UP or down.
   bool up = coinflip();
    
   if(up)
   {

//      if(param == eBandwidth || param==eBreathPressure || param == eBlowPosition || param == eReedStiffness || param == eMultiInstNum ||
//              param== eReedAperature || param ==eVibratoGain || param ==eNoiseGain || param ==eMaxBlowLength || param ==eInstrumentNum)
//         mEndCoef = 1.0+randfloat( mymax(0.0,mTarget->GetInterpParameters()->GetMaxValue(mParam)/mymax(0.000001,topValue) -1.0));
//      else
         mEndCoef = 1.0+randfloatexp2( mymax(0.0,mTarget->GetInterpParameters()->GetMaxValue(mParam)/mymax(0.000001,topValue) -1.0));

   assert(topValue*mEndCoef<=mTarget->GetInterpParameters()->GetMaxValue(mParam)*1.02);
   }
   else
   {
	  //use the minimum to find out where the coefficient should be.  
	  //mEnd Coef can be from the ratio of min to the end, up to 1.0 

//      if(param == eBandwidth || param==eBreathPressure || param == eBlowPosition || param == eReedStiffness || param == eMultiInstNum ||
//              param== eReedAperature || param ==eVibratoGain || param ==eNoiseGain || param ==eMaxBlowLength || param ==eInstrumentNum)
//         mEndCoef = 1.0- randfloat( 1.0-mymin(1.0,mymax(0.0,mTarget->GetInterpParameters()->GetMinValue(mParam)/mymax(0.000001,bottomValue))));
//      else
         mEndCoef = 1.0- randfloatexp2( 1.0-mymin(1.0,mymax(0.0,mTarget->GetInterpParameters()->GetMinValue(mParam)/mymax(0.000001,bottomValue))));

   assert(bottomValue*mEndCoef>=mTarget->GetInterpParameters()->GetMinValue(mParam)*0.98);
   }
   //fix boundries.
   //if(topValue*mEndCoef>=parameters->GetMaxValue(mParam))
   //   mEndCoef = parameters->GetMaxValue(mParam)/topValue;
  // else if(bottomValue*mEndCoef<=parameters->GetMinValue(mParam))
    //  mEndCoef=parameters->GetMinValue(mParam)/bottomValue;
   
   //we might have caused a conflict on the other side, if we did, this guy can't interpolate, and must stay at 1.0
   //it can have local non concatenating fluctuations, however.
   if((topValue*mEndCoef>=mTarget->GetInterpParameters()->GetMaxValue(mParam)) ||
         (bottomValue*mEndCoef<=mTarget->GetInterpParameters()->GetMinValue(mParam)) )
         mEndCoef=1.0;
         

}

//This function needs to be copied from Interpolation.  
//the only difference is that this uses no static variables because the Apply call is recursive.
//double Interpolation::ValueAtTime(double time)
double MetaInterpolation::ValueAtTime(double time)
{
   double cursor =mymin(mymax(0.0,time-mStartTime)/mDuration,1.0);
   double phase;
   double ret;
   
   //get the list of our local parameters and copy them.
   std::vector<double> instantParams;
   bool instantParamsCleared = true;
   
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
   
   //check to see if we have metaInterps and apply them.
   for(int i=0;i<mAttachedMetaInterps.size();i++)
   {
	//caution - MetaInterpolation::Apply uses ValueAtTime 
      mAttachedMetaInterps[i]->Apply(time,&instantParams);
   }
   
//   assert(mStartTime <=time);
//   assert(mStartTime+mDuration>=time);
//   
   phase=modf(     (2 * cursor * (floor(instantParams[eInterpTypeVariable])/**mDuration*/+0.5 ) - 0.5)/2.0,&phase);//we just clober the intpart with the return value
   phase = 3.141592*2* phase;

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
         ret= phase<3.141592?1.0:mEndCoef;
         break;
     
     case ePeriodic:
		//sinusoid that starts at -1 (-pi/2) and continues for (n+0.5)periods to end up at +1 (pi/2)
		//we also shrink it down soo that -pi/2 comes to 1.0 and pi/2 comes to mEndCoef. 
		ret=  (sin(phase)*(mEndCoef-1.0)/2.0)+(mEndCoef-1.0)/2.0 + 1.0;
		break;
      
     default:
       assert(1==0);//shouldn't reach here.
       break;
   }
   return ret;
}

void MetaInterpolation::Apply(double time, std::vector<double>* params)
{
   //we simply concat our value with the param list, since Randomize has already checked boundries.
   (*params)[mParam]*=ValueAtTime(time);
}

void MetaInterpolation::Attach()
{
   mTarget->AddMetaInterpolation(this);
}

void MetaInterpolation::Detach()
{
   mTarget->ConcatAndRemoveMetaInterpolation(this);
}
