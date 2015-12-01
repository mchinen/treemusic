/*
 *  BowedSynthesizer.h
 *  treemusic
 *
 *  Created by apple on 10/31/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */


#ifndef __BOWEDSYNTHESIZER__
#define __BOWEDSYNTHESIZER__
#include "TreeSynthesizer.h"
#include "Bowed.h"
class AudioBuffer;
class MonoBuffer16;

class BowedSynthesizer:public TreeSynthesizer
{
public:

   BowedSynthesizer(float secondsLength, float sampleRate =44100.0);
   virtual ~BowedSynthesizer();
   
   virtual void SynthesizeSample(std::vector<double> *instantParams);
	virtual void SynthesizeFinalizeSample(std::vector<double> *instantParams);


   //returns the buffer that has been synthesized.
   virtual AudioBuffer* GetBuffer();

private:
   bool firstRun; 
	stk::Bowed bowed;
	bool blowing;
	int  noteOrRestDur;
	int  noteOrRestLen;
};

#endif //BowedSynthesizer
