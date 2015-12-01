/*
 *  VoiceHackSynthesizer.h
 *  treemusic
 *
 *  uses a hacked stk VoicForm instrument 
 *  (see http://stria.dartmouth.edu/kdrepos/chinentest/STKHack/)

 *  Created by apple on 9/13/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */
 
#ifndef __VOICEHACKSYNTHESIZER__
#define __VOICEHACKSYNTHESIZER__
#include "TreeSynthesizer.h"
#include "VoiceHack.h"
class AudioBuffer;
class MonoBuffer16;

class VoiceHackSynthesizer:public TreeSynthesizer
{
public:

   VoiceHackSynthesizer(float secondsLength, float sampleRate =44100.0);
   virtual ~VoiceHackSynthesizer();
   
   virtual void SynthesizeSample(std::vector<double> *instantParams);
	virtual void SynthesizeFinalizeSample(std::vector<double> *instantParams);


   //returns the buffer that has been synthesized.
   virtual AudioBuffer* GetBuffer();

private:
   bool firstRun; 
	VoiceHack voice;
	bool blowing;
	int  noteOrRestDur;
	int  noteOrRestLen;
   int finalizeSampleCount;
};

#endif //__VOICEHACKSYNTHESIZER__