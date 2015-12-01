/*
 *  ParamOutputSynthesizer.h
 *  treemusic
 *
 *  Created by mchinen on 12/1/08.
 *  Copyright 2008 Michael Chinen. All rights reserved.
 *
 */

#ifndef __PARAMOUTPUTSYNTHESIZER__
#define __PARAMOUTPUTSYNTHESIZER__

#include "ParameterList.h"
#include "TreeSynthesizer.h"

class AudioBuffer;
class MonoBuffer16;

class ParamOutputSynthesizer: public TreeSynthesizer
{
public:
   ParamOutputSynthesizer(float secondsLength, float sampleRate =44100.0);
   virtual ~ParamOutputSynthesizer();
   
   void SetParameter(Parameter param, double min, double max);
   
   virtual void SynthesizeSample(std::vector<double> *instantParams);

   //returns the buffer that has been synthesized.
   virtual AudioBuffer* GetBuffer();
private:
   Parameter mParam;
   double mMin;
   double mMax;
   
};


#endif __PARAMOUTPUTSYNTHESIZER__