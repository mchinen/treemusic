/*
 *  BandedWGSynthesizer.cpp
 *  treemusic
 *
 *  Created by apple on 10/31/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include "BandedWGSynthesizer.h"


#include "MonoBuffer16.h"
#include "handyfuncs.h"
#include "ParameterList.h"

BandedWGSynthesizer::BandedWGSynthesizer(float secondsLength, float sampleRate )
{
   buffer=new MonoBuffer16();
   buffer->Create(((long int)secondsLength)*((long int)sampleRate) +sampleRate*2);
      currentSample=0;
		
  bandedWG.setFrequency( 220.0 );
//  sax.startBlowing(1.0,1.0);
  
  blowing = false;
  noteOrRestDur = 0;
  noteOrRestLen = 1;

}

BandedWGSynthesizer::~BandedWGSynthesizer()
{
   delete buffer;

}
   

void BandedWGSynthesizer::SynthesizeSample(std::vector<double> *instantParams)
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
    * Bow Motion = 4
    * Strike Position = 8 (not implemented)
    * Vibrato Frequency = 11
    * Gain = 1
    * Bow Velocity = 128
    * Set Striking = 64
    * Instrument Presets = 16
          o Uniform Bar = 0
          o Tuned Bar = 1
          o Glass Harmonica = 2
          o Tibetan Bowl = 3

	*/
	
	//sax.controlChange(128,(*instantParams)[eLoudness]); 
	
   //these are the same control numbers as sax but mean different 
   //params as indicated by the enum names, but whatever.
	bandedWG.setFrequency((*instantParams)[eCenterFrequency]);
	bandedWG.controlChange(128,(*instantParams)[eBreathPressure]); 
	bandedWG.controlChange(11,(*instantParams)[eVibratoFrequency]); 
	bandedWG.controlChange(2,(*instantParams)[eReedStiffness]); 
	bandedWG.controlChange(64,(*instantParams)[eReedAperature]); //plucking
	bandedWG.controlChange(1,(*instantParams)[eVibratoGain]); 
	bandedWG.controlChange(4,(*instantParams)[eNoiseGain]); 

	//should we stop blowing?
	//eMaxBlowLength etc in seconds
	if(noteOrRestLen++ > noteOrRestDur)
	{
		blowing=!blowing;
		if(blowing)
			bandedWG.noteOn((*instantParams)[eCenterFrequency],1.0/(randfloat(44100.0*(*instantParams)[eMaxBlowLength])+0.01));
		//stop blowing at a rate proportional to the rest length
		else
			bandedWG.noteOff(1.0/(randfloat(44100.0*(*instantParams)[eMaxBlowLength])+0.01));
		//new duration?
		noteOrRestLen = 0;
		noteOrRestDur = randint((*instantParams)[blowing?eMaxBlowLength:eMaxRestLength] * 44100);
	}

   buffer->SetFrame(currentSample++,bandedWG.tick() *SHORTLIMITF);
}

void BandedWGSynthesizer::SynthesizeFinalizeSample(std::vector<double> *instantParams) 
{
	if(blowing)
	{
		bandedWG.noteOff(1.0/44100.0);
		blowing=false;
	}
	buffer->SetFrame(currentSample++,bandedWG.tick() *SHORTLIMITF);
}


//returns the buffer that has been synthesized.
AudioBuffer* BandedWGSynthesizer::GetBuffer()
{
   return (AudioBuffer*)buffer;
}