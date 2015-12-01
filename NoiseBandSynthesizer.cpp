/*
 *  NoiseBandSynthesizer.cpp
 *  treemusic
 *
 *  Created by Michael Chinen on 11/25/08.
 *  Copyright 2008 Michael Chinen. All rights reserved.
 *
 */

#include "NoiseBandSynthesizer.h"
#include "ParameterList.h"
#include <math.h>
#include  "handyfuncs.h"
#include "MonoBuffer16.h"

#define kFinalizeSamplesNoiseBand 1000

NoiseBandSynthesizer::NoiseBandSynthesizer(float secondsLength, float sampleRate):TreeSynthesizer(sampleRate)
{
   firstRun = true;
   buffer=new MonoBuffer16();
   buffer->Create(((long int)secondsLength)*((long int)sampleRate) +sampleRate*2);
   currentSample=0;
   finalizeSampleCount=0;
}

NoiseBandSynthesizer::~NoiseBandSynthesizer()
{
   delete buffer;
}
/*
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
   */   
   
void NoiseBandSynthesizer::SynthesizeSample(std::vector<double> *instantParams)
{
   if(firstRun)
   {
      phase=0.0;
      phaseAdd=0.0;
      lastfreq=(*instantParams)[eCenterFrequency];
      lastamp=(*instantParams)[eAmplitude];
      lastbw=(*instantParams)[eBandwidth];
      targetfreq=lastfreq;
      instfreq=lastfreq;
      samplesTillNextTarget =-1;
      firstRun=false;
   }
   float lowFreq,highFreq,centerFreq,maxFreq;
   float freq,amp,bw;
   
   maxFreq = 44100.0/2.0;
   
   freq=(*instantParams)[eCenterFrequency];
   amp=(*instantParams)[eAmplitude];
   bw=mymin(1.0,mymax(0.0,(*instantParams)[eBandwidth]));
   
	lowFreq = freq*(1.0-bw);
   highFreq = mymin(maxFreq,freq*(1.0+bw) );
   centerFreq = (lowFreq+highFreq)/2.0;

   
   //see if we need a new target
   if(samplesTillNextTarget <0)
   {
      targetfreq = randfloat(highFreq-lowFreq)+lowFreq;  //assumes bw <1.0
      samplesTillNextTarget = randint(44100.0/mymax(2.0,centerFreq))+1.0; 
   }else
   {
      samplesTillNextTarget = mymin(44100.0/mymax(2.0,centerFreq),samplesTillNextTarget);
      samplesTillNextTarget--;
   }
   
   instfreq = instfreq + (targetfreq-instfreq)/mymax(1.0,samplesTillNextTarget);
   
   phaseAdd = instfreq*2.0*3.141592/44100.0;
   //phaseAdd += phaseAdd* (randfloat(bw*2)-bw);
   
   phase += phaseAdd;
   while(phase>2.0*3.141592)
      phase-=2.0*3.141592;
   
   buffer->SetFrame(currentSample++,amp*sin(phase)*SHORTLIMITF);
   
   lastfreq=freq;
   lastamp=amp;
   lastbw=bw;
}

 void NoiseBandSynthesizer::SynthesizeFinalizeSample(std::vector<double> *instantParams) 
{
   //TODO: this assumes number of finalize samples is 44100
   long thisSample = currentSample;
   
   SynthesizeSample(instantParams);

	buffer->SetFrame(thisSample,buffer->GetFrame(thisSample)*
         mymax(0.0f,(float)(kFinalizeSamplesNoiseBand-finalizeSampleCount++)/kFinalizeSamplesNoiseBand)
         );
}

AudioBuffer* NoiseBandSynthesizer::GetBuffer()
{
   return (AudioBuffer*)buffer;
}
