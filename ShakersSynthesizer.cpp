/*
 *  ShakersSynthesizer.cpp
 *  treemusic
 *
 *  Created by apple on 10/31/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include "ShakersSynthesizer.h"


#include "MonoBuffer16.h"
#include "handyfuncs.h"
#include "ParameterList.h"

ShakersSynthesizer::ShakersSynthesizer(float secondsLength, float sampleRate )
{
   buffer=new MonoBuffer16();
   buffer->Create(((long int)secondsLength)*((long int)sampleRate) +sampleRate*2);
      currentSample=0;
		
//  shaker.setFrequency( 220.0 );
//  sax.startBlowing(1.0,1.0);
  
  blowing = false;
  noteOrRestDur = 0;
  noteOrRestLen = 1;

}

ShakersSynthesizer::~ShakersSynthesizer()
{
   delete buffer;

}
   

void ShakersSynthesizer::SynthesizeSample(std::vector<double> *instantParams)
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
    * Shake Energy = 2
    * System Decay = 4
    * Number Of Objects = 11
    * Resonance Frequency = 1
    * Shake Energy = 128
    * Instrument Selection = 1071
          o Maraca = 0
          o Cabasa = 1
          o Sekere = 2
          o Guiro = 3
          o Water Drops = 4
          o Bamboo Chimes = 5
          o Tambourine = 6
          o Sleigh Bells = 7
          o Sticks = 8
          o Crunch = 9
          o Wrench = 10
          o Sand Paper = 11
          o Coke Can = 12
          o Next Mug = 13
          o Penny + Mug = 14
          o Nickle + Mug = 15
          o Dime + Mug = 16
          o Quarter + Mug = 17
          o Franc + Mug = 18
          o Peso + Mug = 19
          o Big Rocks = 20
          o Little Rocks = 21
          o Tuned Bamboo Chimes = 22


	*/
	
	//sax.controlChange(128,(*instantParams)[eLoudness]); 
	
   //these are the same control numbers as sax but mean different 
   //params as indicated by the enum names, but whatever.
//	shaker.setFrequency((*instantParams)[eCenterFrequency]);
	shaker.controlChange(128,(*instantParams)[eBreathPressure]); 
	shaker.controlChange(11,(*instantParams)[eVibratoFrequency]); 
	shaker.controlChange(2,(*instantParams)[eReedStiffness]); 
	shaker.controlChange(1071,(*instantParams)[eInstrumentNum]); //plucking
	shaker.controlChange(1,(*instantParams)[eVibratoGain]); 
	shaker.controlChange(4,(*instantParams)[eNoiseGain]); 

	//should we stop blowing?
	//eMaxBlowLength etc in seconds
	if(noteOrRestLen++ > noteOrRestDur)
	{
		blowing=!blowing;
		if(blowing) //noteOn takes frequency as first param.  This is not documented (special case for Shakers)
			shaker.noteOn((*instantParams)[eCenterFrequency],1.0/(randfloat(44100.0*(*instantParams)[eMaxBlowLength])+0.01));
		//stop blowing at a rate proportional to the rest length
		else
         shaker.noteOff(1.0/(randfloat(44100.0*(*instantParams)[eMaxBlowLength])+0.01));
		//new duration?
		noteOrRestLen = 0;
		noteOrRestDur = randint((*instantParams)[blowing?eMaxBlowLength:eMaxRestLength] * 44100);
	}

   buffer->SetFrame(currentSample++,shaker.tick() *SHORTLIMITF* (*instantParams)[eAmplitude]);
}

void ShakersSynthesizer::SynthesizeFinalizeSample(std::vector<double> *instantParams) 
{
	if(blowing)
	{
      shaker.noteOff(1.0/44100.0);
		blowing=false;
	}
	buffer->SetFrame(currentSample++,shaker.tick() *SHORTLIMITF);
}


//returns the buffer that has been synthesized.
AudioBuffer* ShakersSynthesizer::GetBuffer()
{
   return (AudioBuffer*)buffer;
}