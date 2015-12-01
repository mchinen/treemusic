/*
 *  MultiSynthesizer.cpp
 *  treemusic
 *
 *  Created by apple on 12/31/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include "MultiSynthesizer.h"
#include "ParameterList.h"
#include <math.h>
#include  "handyfuncs.h"
#include "MonoBuffer16.h"

#include "NoiseBandSynthesizer.h"
#include "SaxHackSynthesizer.h"
#include "ShakersSynthesizer.h"
#include "BowedSynthesizer.h"
#include "VoiceHackSynthesizer.h"



#define kRadiusExponent 1

MultiSynthesizer::MultiSynthesizer(float secondsLength, float sampleRate):TreeSynthesizer(sampleRate)
{
   firstRun = true;
   buffer=new MonoBuffer16();
   buffer->Create(((long int)secondsLength)*((long int)sampleRate) +sampleRate*2);
   
   //add all the synthesizers we want.
   m_subsynths.push_back(new NoiseBandSynthesizer(secondsLength,sampleRate));
   m_subsynths.push_back(new ShakersSynthesizer(secondsLength,sampleRate));
   m_subsynths.push_back(new SaxHackSynthesizer(secondsLength,sampleRate));
   m_subsynths.push_back(new BowedSynthesizer(secondsLength,sampleRate));
   m_subsynths.push_back(new VoiceHackSynthesizer(secondsLength,sampleRate));
   currentSample=0;
}

MultiSynthesizer::~MultiSynthesizer()
{
   for(int i=0;i<m_subsynths.size();i++)
      delete m_subsynths[i];
   delete buffer;
}


AudioBuffer* MultiSynthesizer::GetBuffer()
{
   return (AudioBuffer*)buffer;
}

//we use the  param eMultiInstNum to determine the instrument.  if it is from 0 to m_subsynths.size()
//then we use the two instrument it is closest to, with each inst being proportionately louder as 
//the parameter is closer to the inst number.  
//above m_subsynths.size() the same thing happens, but the 'radius' is larger, and the 'center' inst
//is the parameter modulo the number of insts.
void MultiSynthesizer::SynthesizeSample(std::vector<double> *instantParams)
{
   //diamter is number of synths to use.  we always have at least 2.
   int diameter = (*instantParams)[eMultiInstNum]/m_subsynths.size() +2;
   int instNumFloor = ((int)(*instantParams)[eMultiInstNum]);
   int closestInst = instNumFloor%m_subsynths.size();
   double paramModded = (*instantParams)[eMultiInstNum];
   while(paramModded >= m_subsynths.size())
      paramModded-= m_subsynths.size();
      
   if(diameter>m_subsynths.size()) diameter=m_subsynths.size();
   
   //if the decimal part is greater than .5 then we round up the number.
   if( (*instantParams)[eMultiInstNum] -instNumFloor >0.5)
      closestInst=++closestInst % m_subsynths.size();
   
   bool closestAbove =closestInst >= paramModded || (closestInst ==0 && paramModded>1.0);

   //distance to closest synth
   float distance = closestAbove ?closestInst==0?m_subsynths.size()-paramModded:(closestInst - paramModded):
                                (paramModded- closestInst);
   assert(distance<=1.0);
   assert(distance>=0.0);
   //we play the closest instrument at a rate like:
   
   long sample = 0;
   int nextInst = closestAbove?(closestInst-1<0?m_subsynths.size()-1:(closestInst-1)):
                     (closestInst+1==m_subsynths.size()?0:(closestInst+1));
   
   
   int rightCursor=closestAbove?closestInst:nextInst;
   int leftCursor= closestAbove?nextInst:closestInst;
   bool rightCursorIsNext = closestAbove;
   
   //extend the 'circle' out from the closestInst index.  we use a right and left boundry to do this.
   //the last two instruments use partial amplitude based on distance from center.
   int i = diameter;
   while(i-- > 0)
   {
      //on the last iteration i will be zero.
      m_subsynths[rightCursorIsNext?rightCursor:leftCursor]->SynthesizeSample(instantParams);
      sample += m_subsynths[rightCursorIsNext?rightCursor:leftCursor]->GetLastLongSample() * 
         (i<2?powf(i==1?1.0-distance:distance,kRadiusExponent) : 1); 
         
      if(rightCursorIsNext) 
         rightCursor= (rightCursor+1)%m_subsynths.size();
      else
         leftCursor= leftCursor==0?m_subsynths.size()-1:(leftCursor-1);
      rightCursorIsNext=!rightCursorIsNext;
      
   }
//
//   m_subsynths[nextInst]->SynthesizeSample(instantParams);
//   sample += m_subsynths[nextInst]->GetLastLongSample() * powf(distance,kRadiusExponent);
   
   //now go backwards and add at 
   
   buffer->SetFrame(currentSample,sample );
   
   currentSample++;
}
void MultiSynthesizer::SynthesizeFinalizeSample(std::vector<double> *instantParams) 
{
  //diamter is number of synths to use.  we always have at least 2.
   int diameter = (*instantParams)[eMultiInstNum]/m_subsynths.size() +2;
   int instNumFloor = ((int)(*instantParams)[eMultiInstNum]);
   int closestInst = instNumFloor%m_subsynths.size();
   double paramModded = (*instantParams)[eMultiInstNum];
   while(paramModded >= m_subsynths.size())
      paramModded-= m_subsynths.size();
      
   if(diameter>m_subsynths.size()) diameter=m_subsynths.size();
   
   //if the decimal part is greater than .5 then we round up the number.
   if( (*instantParams)[eMultiInstNum] -instNumFloor >0.5)
      closestInst=(1+closestInst) % m_subsynths.size();
   
   bool closestAbove =closestInst >= paramModded || (closestInst ==0 && paramModded>1.0);

   float distance = closestAbove ?closestInst==0?m_subsynths.size()-paramModded:(closestInst - paramModded):
                                (paramModded- closestInst);
   assert(distance<=1.0);
   assert(distance>=0.0);
   //we play the closest instrument at a rate like:
   
   long sample = 0;
   int nextInst = closestAbove?(closestInst-1<0?m_subsynths.size()-1:(closestInst-1)):
                     (closestInst+1==m_subsynths.size()?0:(closestInst+1));
   
   
   int rightCursor=closestAbove?closestInst:nextInst;
   int leftCursor= closestAbove?nextInst:closestInst;
   bool rightCursorIsNext = closestAbove;
   
   //extend the 'circle' out from the closestInst index.  we use a right and left boundry to do this.
   //the last two instruments use partial amplitude based on distance from center.
   while(diameter-- > 0)
   {
      
      m_subsynths[rightCursorIsNext?rightCursor:leftCursor]->SynthesizeFinalizeSample(instantParams);
      sample = m_subsynths[rightCursorIsNext?rightCursor:leftCursor]->GetLastLongSample() * 
         (diameter<2?powf(diameter==1?1.0-distance:distance,kRadiusExponent) : 1); 
         
      if(rightCursorIsNext) 
         rightCursor= (rightCursor+1)%m_subsynths.size();
      else
         leftCursor= leftCursor==0?m_subsynths.size()-1:(leftCursor-1);
      rightCursorIsNext=!rightCursorIsNext;
   }
//
//   m_subsynths[nextInst]->SynthesizeSample(instantParams);
//   sample += m_subsynths[nextInst]->GetLastLongSample() * powf(distance,kRadiusExponent);
   
   //now go backwards and add at 
   
   buffer->SetFrame(currentSample,sample );
   
   currentSample++;}