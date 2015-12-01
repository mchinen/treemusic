/*
 *  ParameterList.cpp
 *  treemusic
 *
 *  Created by Michael Chinen on 11/23/08.
 *  Copyright 2008 . All rights reserved.
 *
 */

#include "ParameterList.h"
#include <assert.h>
#include "handyfuncs.h"

//creates a copy
ParameterList* ParameterList::Clone()
{
   ParameterList* clone = new ParameterList;
   for(int i=0;i<mInitialValues.size();i++)
   {
      clone->mInitialValues.push_back(mInitialValues[i]);
      clone->mCoefficients.push_back(mCoefficients[i]);  //permanent modifiers.
      clone->minValues.push_back(minValues[i]);
      clone->maxValues.push_back(maxValues[i]);
      
      std::vector<double> dummy;
      //mLayerCoefs[mBottomLayer] = dummy;
      clone->mLayerCoefs[mBottomLayer].push_back(mLayerCoefs[mBottomLayer][i]);
   
      for(int j =mTopLayer;j<mTopLayer+mNumTopLayers;j++)
      {
		
      //  clone->mLayerCoefs[j]=dummy;
        clone->mLayerCoefs[j].push_back(mLayerCoefs[j][i]);   
      }    
   }
   
   
   clone->mBottomLayer=mBottomLayer;
   clone->mTopLayer=mTopLayer;
   clone->mNumTopLayers=mNumTopLayers;

   
   return clone;
}

/*
ParameterList::ParameterList(int bottomLayer, int topLayer,int numTopLayers)
      :mBottomLayer(bottomLayer),mTopLayer(topLayer),mNumTopLayers(numTopLayers)
{

}
*/
void ParameterList::SetBottomLayer(int bottomLayer)
{
   mBottomLayer=bottomLayer;
}
void ParameterList::SetTopLayer(int topLayer)
{
   mTopLayer=topLayer;
}
void ParameterList::SetNumTopLayers(int numTopLayers)
{
   mNumTopLayers=numTopLayers;
}

//parameters get added the base, so we don't need to specify which layers it affects.
void ParameterList::AddParameter(int parameterNumber,double initialValue,double minimum, double maximum)
{
   //make sure we insert in order.
   assert(parameterNumber==mInitialValues.size());

   mInitialValues.push_back(initialValue);
   mCoefficients.push_back(1.0);  //permanent modifiers.
      
   mLayerCoefs[mBottomLayer].push_back(1.0);
   for(int i =mTopLayer;i<mTopLayer+mNumTopLayers;i++)
   {
        mLayerCoefs[i].push_back(1.0);   
    }    
   minValues.push_back(minimum);
   maxValues.push_back(maximum);
}

void ParameterList::SetParameter(int parameterNumber,double initialValue,double minimum, double maximum)
{
   assert(parameterNumber<mInitialValues.size());

   mInitialValues[parameterNumber]=initialValue;
   minValues[parameterNumber]=minimum;
   maxValues[parameterNumber]=maximum;
 
}

//affects x number of layers, at least one (the bottom layer.)  if numTopLayers==0 nothing happens, otherwise,
//it also affects the layers starting at topLayer till topLyaer+numTopLayers-1
void ParameterList::AddCoefficient(int parameterNumber,double coef, int bottomLayer,int topLayer,int numTopLayers)
{
   assert(GetValue(parameterNumber) <= maxValues[parameterNumber]*1.02);
   assert(GetValue(parameterNumber) >= minValues[parameterNumber] *0.98 );
   mCoefficients[parameterNumber]= mCoefficients[parameterNumber]* coef;

//mchinen: old - shoudn't this use the local bottom and top layers
//(mixing members and locals seems wrong)   
//   mLayerCoefs[mBottomLayer][parameterNumber] *= coef;
//   
//   for(int i =mTopLayer;i<mTopLayer+numTopLayers;i++)
//        mLayerCoefs[i][parameterNumber]*= coef;

   mLayerCoefs[bottomLayer][parameterNumber] *= coef;

   for(int i =topLayer;i<topLayer+numTopLayers;i++)
   {
        mLayerCoefs[i][parameterNumber]*= coef;
   }
   assert(GetValue(parameterNumber) <= maxValues[parameterNumber]*1.02);
   assert(GetValue(parameterNumber) >= minValues[parameterNumber]*0.98 );
}


double ParameterList::GetLayerCoef(int layer, int paramIndex)
{
   return mLayerCoefs[layer][paramIndex];
}
   
double ParameterList::GetValue(int parameterNumber)
{
   return //mymin(maxValues[parameterNumber],mymax(minValues[parameterNumber],
         mInitialValues[parameterNumber]*mCoefficients[parameterNumber]
         ;
         //));
}

double ParameterList::GetInitialValue(int parameterNumber)
{
   return mInitialValues[parameterNumber];
}

double ParameterList::GetMaxValue(int parameterNumber)
{
   return maxValues[parameterNumber];
}

void ParameterList::ResetCoefficients()
{
   for(int i =0;i<mCoefficients.size();i++)
      mCoefficients[i]=1.0;
      
   
   for(int i =0;i<mInitialValues.size();i++)
      mLayerCoefs[mBottomLayer][i] = 1.0;
   
   for(int i =mTopLayer;i<mTopLayer+mNumTopLayers;i++)
      for(int j =0;j<mInitialValues.size();j++)
        mLayerCoefs[i][j] = 1.0;   
}

double ParameterList::GetMinValue(int parameterNumber)
{
   return minValues[parameterNumber];
}
