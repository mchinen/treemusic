/*
 *  ParameterList.h
 *  treemusic
 *
    Treemusic is about interpolation of parameters.  This structure keeps a list of the parameter values at a given traversal of the tree.
    intialValues refer to the values at time = 0, at the very left edge of the ENTIRE tree, not wherever we happen to be.
    
    The values at the beginning of a leaf can be computed by taking the initialValues and multiplying them against the coefficients.
    
    The min and max values for each parameter show where the parameters can be interpolated to.  
    
    TODO:Some min/max pairs have dependencies
    on eachother, because the paremeters themselves are mins or maxes, in which case the max and min of a minimum param can not be greater
    than the min/max of the maximum parameter.  ugly, but maybe beautiful?  This should be regulated as a different structure.  
 
 *  Created by Michael CHinen on 11/23/08.
 *  Copyright lAll rights reserved.
 *
 */

#ifndef __PARAMETER_LIST__H__
#define __PARAMETER_LIST__H__

#include <vector>

#include <map>

/* TODO: don't make this a hack that we need to uncomment to switch synths
enum Parameter
{
   eAmplitude=0,
   eCenterFrequency,
   eBandwidth,
   eRandomness,
   eLength,
   eMutation,
   eGrainStart,
   eShift,
   eSkip,
   eNumParams
};
*/
enum Parameter
{
	eBreathPressure=0,
	eBlowPosition,
	eReedStiffness,
	eReedAperature,
   eCenterFrequency,
   eVibratoFrequency,
   eVibratoGain,
   eNoiseGain,
	eMaxBlowLength,
	eMaxRestLength,
   eInstrumentNum,
	eAmplitude,
   eBandwidth,
   eMultiInstNum,
   eRandomness,
   eLength,
   eMutation,
   eGrainStart,
   eShift,
   eSkip,
	eNumParams
};

//we will have to typecast these.
enum InterpolationParameter
{
   eInterpTypeVariable=0,
   eNumInterpParams
};

class ParameterList
{
public:
//   ParameterList(int bottomLayer, int topLayer,int numTopLayers);
   ParameterList(){mBottomLayer=0;mTopLayer=1;mNumTopLayers=0;}
   ParameterList* Clone();
   
   int GetNumParameters(){return mInitialValues.size();}
   
   void AddParameter(int parameterNumber,double initialValue,double minimum, double maximum);
   void AddCoefficient(int parameterNumber,double coef, int bottomLayer,int topLayer,int numTopLayers);
   void SetParameter(int parameterNumber,double initialValue,double minimum, double maximum);
   
   void ResetCoefficients();

   double GetValue(int parameterNumber);
   double GetMaxValue(int parameterNumber);
   double GetMinValue(int parameterNumber);
   double GetInitialValue(int parameterNumber);

   //double GetLowestLayer(int layer, int paramIndex);
   
   //we should add a l
   //double AddLayer(int layer, int paramIndex);

   double GetLayerCoef(int layer, int paramIndex);

   void SetBottomLayer(int bottomLayer);
   void SetTopLayer(int topLayer);
   void SetNumTopLayers(int numTopLayers);   
private:
   std::vector<double> mInitialValues;
   std::vector<double> mCoefficients;  //permanent modifiers.  TODO:replace with layer coefs.
   std::vector<double> minValues;
   std::vector<double> maxValues;
   
   int mBottomLayer;
   int mTopLayer;
   int mNumTopLayers;
   
   //if our interpolation is meta-interpolated by another interpolation,
   //we need to keep track of what the smallest and largest interpolated values
   //are AT EACH LAYER, to keep us within boundaries.
   std::map<int, std::vector<double> > mLayerCoefs;
   
};

#endif
