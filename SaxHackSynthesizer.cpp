/*
 *  SaxHackSynthesizer.cpp
 *  treemusic
 *
 *  Created by apple on 6/2/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include "SaxHackSynthesizer.h"

#include "MonoBuffer16.h"
#include "handyfuncs.h"
#include "ParameterList.h"

SaxHackSynthesizer::SaxHackSynthesizer(float secondsLength, float sampleRate )
{
   buffer=new MonoBuffer16();
   buffer->Create(((long int)secondsLength)*((long int)sampleRate) +sampleRate*2);
   currentSample=0;
		
  sax.setFrequency( 220.0 );
//  sax.startBlowing(1.0,1.0);
  
  blowing = false;
  noteOrRestDur = 0;
  noteOrRestLen = 1;

}

SaxHackSynthesizer::~SaxHackSynthesizer()
{
   delete buffer;

}
   

void SaxHackSynthesizer::SynthesizeSample(std::vector<double> *instantParams)
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
		
	 input.controlChange(11,64); //blow position
	 input.controlChange(2,64); //reed stiffness
	 input.controlChange(26,64); //reed ap
	 input.controlChange(128,128); //breath pressure
	*/
	
	//sax.controlChange(128,(*instantParams)[eLoudness]); 
	
	sax.setFrequency((*instantParams)[eCenterFrequency]);
	sax.controlChange(128,(*instantParams)[eBreathPressure]); 
	sax.controlChange(11,(*instantParams)[eBlowPosition]); 
	sax.controlChange(2,(*instantParams)[eReedStiffness]); 
	sax.controlChange(26,(*instantParams)[eReedAperature]); 
	sax.controlChange(29,(*instantParams)[eVibratoFrequency]); 
	sax.controlChange(1,(*instantParams)[eVibratoGain]); 
	sax.controlChange(4,(*instantParams)[eNoiseGain]); 

	//should we stop blowing?
	//eMaxBlowLength etc in seconds
	if(noteOrRestLen++ > noteOrRestDur)
	{
		blowing=!blowing;
		if(blowing)
			sax.startBlowing(1.0,1.0/(randfloat(44100.0*(*instantParams)[eMaxBlowLength])+0.01));
		//stop blowing at a rate proportional to the rest length
		else
			sax.stopBlowing(1.0/(44100.0*randfloat((*instantParams)[eMaxRestLength])+0.01));
		//new duration?
		noteOrRestLen = 0;
		noteOrRestDur = randint((*instantParams)[blowing?eMaxBlowLength:eMaxRestLength] * 44100);
	}


	sax.controlChange(26,(*instantParams)[eReedAperature]); 
      buffer->SetFrame(currentSample++,sax.tick() *SHORTLIMITF * (*instantParams)[eAmplitude]);
}

 void SaxHackSynthesizer::SynthesizeFinalizeSample(std::vector<double> *instantParams) 
{
	if(blowing)
	{
		sax.stopBlowing(1.0/44100.0);
		blowing=false;
	}
	buffer->SetFrame(currentSample++,sax.tick() *SHORTLIMITF);
}


//returns the buffer that has been synthesized.
AudioBuffer* SaxHackSynthesizer::GetBuffer()
{
   return (AudioBuffer*)buffer;
}
