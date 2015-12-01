/*
 *  Interpolation.h
 *  treemusic
 *
    A class that represents the interpolation from one point in time until another.  
    The same parameter might be interpolated on several layers, which brings a few other things
    into consideration, unless we don't want to worry about overshooting the min/max.
    
    Interpolation objects have only one value that represents the range of the coefficient. 
    It is a single coefficient.  Note that it does not specify an absolute value to interpolate to.
    Coefficients can be added up.
    
    The type of interpolation is variable - it could be linear, exponential, or modulate back and forth.
    However, it should always stay between zero and the end value.  
    
    For now the coefficeints are from zero to the maximum allowed interpolation.  This does not support negative values.
   
    Start time and duration are also maintained by this object, because it needs to know h
    
    When the interpolation is over, the parameter remains affected by the interpolation - it does not roll back to the original value.
    It should be thought of as a turning of the knob, in one direction.  
    
    I am introducing a new type of interpolation as of 1/9/2009.  It does not have a cumulative effect on the parameter because
    it ends where it starts.  But it does something else in the middle.
 *
 *  Created by Michael Chinen on 11/23/08.
 *  Copyright 2008 Arg. All rights reserved.
 *
 */
 
#ifndef __INTERPOLATION_H__
#define __INTERPOLATION_H__


#include <vector>
#include "ParameterList.h"



enum InterpolationType
{

   eSquare,
   ePeriodic,
	eStep,
   eExp2Step,
   eExp2,
   eLinear,
   
   eNumInterpolationTypes,
   eEndDummy //for copy and pasting the above guys around (we can keep the comma)
};

class TreeNode;
class MetaInterpolation;

class Interpolation
{
public:
   Interpolation(int bottomLayer, int topLayer,int numTopLayers,TreeNode* owner);

   Interpolation* SimilarClone(int bottomLayer, int topLayer,int numTopLayers,TreeNode* owner);


   void SetStartTime(double time){mStartTime = time;}
   double GetStartTime(){return mStartTime;}
   
   void SetDuration(double time){mDuration = time;}
   double GetDuration(){return mDuration;}
   
   double GetEndCoefficient(){return mDoesConcatenate?mEndCoef:1.0;}
   
   Parameter GetParameter(){return mParam;}
   void   SetParameter(Parameter p){mParam=p;}
   
   bool DoesConcatenate(){return mDoesConcatenate;}
   
   InterpolationType GetInterpolationType(){return mType;}
   
   void AddMetaInterpolation(MetaInterpolation* metaInterp);
   void ConcatAndRemoveMetaInterpolation(MetaInterpolation* metaInterp);
   
   virtual double ValueAtTime(double time);   
	//for estimating value
   double MaxValueInTimeRange(double startTime,double endTime);
   double MinValueInTimeRange(double startTime,double endTime);

   //sets the end coef to be the reciprocal.
   void Invert();
   void Truncate(double startTime, double duration, std::vector<TreeNode*> *activeNodes,  std::vector<ParameterList*> *parameters, int layerNum,  int topLayer, int numControlledLayers);
   virtual void Randomize(double startTime, double duration,std::vector<TreeNode*> *activeNodes, std::vector<ParameterList*> *parameters, int layerNum,  int topLayer, int numControlledLayers, int param=-1);
   
   ParameterList* GetInterpParameters(){return &mInterpParams;}
   int GetNumAttachedMetaInterpolations(){return mAttachedMetaInterps.size();}
   MetaInterpolation* GetAttachedMetaInterpolation(int index){return mAttachedMetaInterps[index];}
   
   TreeNode* GetOwner(){return mOwner;}
protected:
   //meta interps that belong to child nodes that attach to US
   std::vector<MetaInterpolation*> mAttachedMetaInterps;
         
   TreeNode* mOwner;
   InterpolationType mType;
   Parameter   mParam;
   ParameterList mInterpParams;
   
   float       mEndCoef;
   double      mStartTime;
   double      mDuration;
   double       interpTypeVariable;
   bool        mDoesConcatenate;
};

#endif