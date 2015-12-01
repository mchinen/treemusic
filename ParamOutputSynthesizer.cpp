/*
 *  ParamOutputSynthesizer.cpp
 *  treemusic
 *
 *  Created by mchinen on 12/1/08.
 *  Copyright 2008 Michael Chinen. All rights reserved.
 *
 */

#include "ParamOutputSynthesizer.h"
#include "MonoBuffer16.h"
#include "handyfuncs.h"

ParamOutputSynthesizer::ParamOutputSynthesizer(float secondsLength, float sampleRate )
{
   buffer=new MonoBuffer16();
   buffer->Create(((long int)secondsLength)*((long int)sampleRate) +sampleRate);
      currentSample=0;
}

ParamOutputSynthesizer::~ParamOutputSynthesizer()
{
   delete buffer;

}
   
void ParamOutputSynthesizer::SetParameter(Parameter param, double min, double max)
{
   mParam=param;
   mMin = mymin(min, max);
   mMax= mymax(min,max);
}   
void ParamOutputSynthesizer::SynthesizeSample(std::vector<double> *instantParams)
{
      buffer->SetFrame(currentSample++,(((*instantParams)[mParam]-mMin)/mymax(0.00001,mMax-mMin) - 0.5)  *SHORTLIMITF);
}


//returns the buffer that has been synthesized.
AudioBuffer* ParamOutputSynthesizer::GetBuffer()
{
   return (AudioBuffer*)buffer;
}
