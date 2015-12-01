/*
 *  BowedSynthesizer.cpp
 *  treemusic
 *
 *  Created by apple on 10/31/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include "BowedSynthesizer.h"

#include "MonoBuffer16.h"
#include "handyfuncs.h"
#include "ParameterList.h"

BowedSynthesizer::BowedSynthesizer(float secondsLength, float sampleRate ):
bowed(30.0)
{
   buffer=new MonoBuffer16();
   buffer->Create(((long int)secondsLength)*((long int)sampleRate) +sampleRate*2);
      currentSample=0;
		
  bowed.setFrequency( 220.0 );
//  sax.startBlowing(1.0,1.0);
  
  blowing = false;
  noteOrRestDur = 0;
  noteOrRestLen = 1;

}

BowedSynthesizer::~BowedSynthesizer()
{
   delete buffer;

}
   

void BowedSynthesizer::SynthesizeSample(std::vector<double> *instantParams)
{
	/* these should be defined.
	eBreathPressure,
	eBlowPosition,
	eReedStiffness,
	eReedAperature,
	  
	   eVibratoFrequency,
   eVibratoGain,
   eNoiseGain,
	eMaxBlowLength,
	eMaxRestLength,
	control numbers:
    * Bow Pressure = 2
    * Bow Position = 4
    * Vibrato Frequency = 11
    * Vibrato Gain = 1
    * Volume = 128
	*/
	
	//sax.controlChange(128,(*instantParams)[eLoudness]); 
	
   //these are the same control numbers as sax but mean different 
   //params as indicated by the enum names, but whatever.
	bowed.setFrequency((*instantParams)[eCenterFrequency]);
	bowed.controlChange(128,(*instantParams)[eBreathPressure]); 
	bowed.controlChange(11,(*instantParams)[eVibratoFrequency]); 
	bowed.controlChange(2,(*instantParams)[eReedStiffness]); 
//	bowed.controlChange(26,(*instantParams)[eReedAperature]); 
//	bowed.controlChange(29,(*instantParams)[eVibratoFrequency]); 
	bowed.controlChange(1,(*instantParams)[eVibratoGain]); 
	bowed.controlChange(4,(*instantParams)[eNoiseGain]); 

	//should we stop blowing?
	//eMaxBlowLength etc in seconds
	if(noteOrRestLen++ > noteOrRestDur)
	{
		blowing=!blowing;
		if(blowing)
			bowed.startBowing(1.0,1.0/(randfloat(44100.0*(*instantParams)[eMaxBlowLength])+0.01));
		//stop blowing at a rate proportional to the rest length
		else
			bowed.stopBowing(1.0/(44100.0*randfloat((*instantParams)[eMaxRestLength])+0.01));
		//new duration?
		noteOrRestLen = 0;
		noteOrRestDur = randint((*instantParams)[blowing?eMaxBlowLength:eMaxRestLength] * 44100);
	}

   buffer->SetFrame(currentSample++,bowed.tick() *SHORTLIMITF* (*instantParams)[eAmplitude]);
}

void BowedSynthesizer::SynthesizeFinalizeSample(std::vector<double> *instantParams) 
{
	if(blowing)
	{
		bowed.stopBowing(1.0/44100.0);
		blowing=false;
	}
	buffer->SetFrame(currentSample++,bowed.tick() *SHORTLIMITF);
}


//returns the buffer that has been synthesized.
AudioBuffer* BowedSynthesizer::GetBuffer()
{
   return (AudioBuffer*)buffer;
}