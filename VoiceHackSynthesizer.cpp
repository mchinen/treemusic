/*
 *  VoiceHackSynthesizer.cpp
 *  treemusic
 *
 *  Created by apple on 9/13/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include "VoiceHackSynthesizer.h"


#include "MonoBuffer16.h"
#include "handyfuncs.h"
#include "ParameterList.h"

#define kFinalizeSamplesVoiceHack 100

VoiceHackSynthesizer::VoiceHackSynthesizer(float secondsLength, float sampleRate )
{
   buffer=new MonoBuffer16();
   buffer->Create(((long int)secondsLength)*((long int)sampleRate) +sampleRate*2);
      currentSample=0;
		
   voice.setFrequency( 220.0 );
//  sax.startBlowing(1.0,1.0);
  
   blowing = false;
   noteOrRestDur = 0;
   noteOrRestLen = 1;
   finalizeSampleCount=0;
}

VoiceHackSynthesizer::~VoiceHackSynthesizer()
{
   delete buffer;

}
   

void VoiceHackSynthesizer::SynthesizeSample(std::vector<double> *instantParams)
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
	
   //these are the same control numbers as sax but mean different 
   //params as indicated by the enum names, but whatever.
	voice.setFrequency((*instantParams)[eCenterFrequency]);
	voice.controlChange(128,(*instantParams)[eBreathPressure]); 
	voice.controlChange(11,(*instantParams)[eVibratoFrequency]); 
	voice.controlChange(2,(*instantParams)[eReedStiffness]); 
//	voice.controlChange(26,(*instantParams)[eReedAperature]); 
//	voice.controlChange(29,(*instantParams)[eVibratoFrequency]); 
	voice.controlChange(1,(*instantParams)[eVibratoGain]); 
	voice.controlChange(4,(*instantParams)[eNoiseGain]); 

	//should we stop blowing?
	//eMaxBlowLength etc in seconds
	if(noteOrRestLen++ > noteOrRestDur)
	{
		blowing=!blowing;
		if(blowing)
			voice.speak();
		//stop blowing at a rate proportional to the rest length
		else
			voice.quiet();
		//new duration?
		noteOrRestLen = 0;
		noteOrRestDur = randint((*instantParams)[blowing?eMaxBlowLength:eMaxRestLength] * 44100);
	}

   buffer->SetFrame(currentSample++,voice.tick() *SHORTLIMITF * (*instantParams)[eAmplitude]);
}

void VoiceHackSynthesizer::SynthesizeFinalizeSample(std::vector<double> *instantParams) 
{
	if(blowing)
	{
		voice.quiet();
		blowing=false;
	}
	buffer->SetFrame(currentSample++,voice.tick() *SHORTLIMITF * mymax(0.0f,(float)(kFinalizeSamplesVoiceHack-finalizeSampleCount++)/kFinalizeSamplesVoiceHack));
}


//returns the buffer that has been synthesized.
AudioBuffer* VoiceHackSynthesizer::GetBuffer()
{
   return (AudioBuffer*)buffer;
}