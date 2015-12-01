/*
 *  NoiseBandSynthesizer.h
 *  treemusic
 *
 *  Created by Michael Chinen on 11/25/08.
 *  Copyright 2008 mcdc. All rights reserved.
 *
 */

#ifndef __NOISEBANDSYNTHESIZER__
#define __NOISEBANDSYNTHESIZER__
#include "TreeSynthesizer.h"

class AudioBuffer;
class MonoBuffer16;

class NoiseBandSynthesizer:public TreeSynthesizer
{
public:

   NoiseBandSynthesizer(float secondsLength, float sampleRate =44100.0);
   virtual ~NoiseBandSynthesizer();
   
   virtual void SynthesizeSample(std::vector<double> *instantParams);
   virtual void SynthesizeFinalizeSample(std::vector<double> *instantParams) ;
   
   
   //returns the buffer that has been synthesized.
   virtual AudioBuffer* GetBuffer();

private:
   bool firstRun;
   //synth stuff.
   double phase;
   double phaseAdd;
	float lastfreq;
	float instfreq;
	float lastamp;
	float lastbw;
   float targetfreq;
	int samplesTillNextTarget;
   int finalizeSampleCount;

   

};

#endif //__NOISEBANDSYNTHESIZER__