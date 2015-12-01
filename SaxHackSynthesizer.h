/*
 *  SaxHackSynthesizer.h
 *  treemusic
 *
 *  uses a hacked stk sax instrument 
 *  (see http://stria.dartmouth.edu/kdrepos/chinentest/STKHack/)
 *
 *  Created by michael chinen on 6/2/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef __SAXHACKSYNTHESIZER__
#define __SAXHACKSYNTHESIZER__
#include "TreeSynthesizer.h"
#include "SaxHack.h"
class AudioBuffer;
class MonoBuffer16;

class SaxHackSynthesizer:public TreeSynthesizer
{
public:

   SaxHackSynthesizer(float secondsLength, float sampleRate =44100.0);
   virtual ~SaxHackSynthesizer();
   
   virtual void SynthesizeSample(std::vector<double> *instantParams);
	virtual void SynthesizeFinalizeSample(std::vector<double> *instantParams);


   //returns the buffer that has been synthesized.
   virtual AudioBuffer* GetBuffer();

private:
   bool firstRun;
	SaxHack sax;
	bool blowing;
	int  noteOrRestDur;
	int  noteOrRestLen;
};

#endif //SaxHackSynthesizer