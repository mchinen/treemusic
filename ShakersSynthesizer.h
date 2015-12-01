/*
 *  ShakersSynthesizer.h
 *  treemusic
 *
 *  Created by apple on 10/31/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */
 
#ifndef __SHAKERSSYNTHESIZER__
#define __SHAKERSSYNTHESIZER__
#include "TreeSynthesizer.h"
#include "Shakers.h"
class AudioBuffer;
class MonoBuffer16;

class ShakersSynthesizer:public TreeSynthesizer
{
public:

   ShakersSynthesizer(float secondsLength, float sampleRate =44100.0);
   virtual ~ShakersSynthesizer();
   
   virtual void SynthesizeSample(std::vector<double> *instantParams);
	virtual void SynthesizeFinalizeSample(std::vector<double> *instantParams);


   //returns the buffer that has been synthesized.
   virtual AudioBuffer* GetBuffer();

private:
   bool firstRun; 
	stk::Shakers shaker;
	bool blowing;
	int  noteOrRestDur;
	int  noteOrRestLen;
};

#endif //BowedSynthesizer
