/*
 *  BandedWGSynthesizer.h
 *  treemusic
 *
 *  Created by apple on 10/31/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */


#ifndef __BANDEDWGSYNTHESIZER__
#define __BANDEDWGSYNTHESIZER__
#include "TreeSynthesizer.h"
#include "BandedWG.h"
class AudioBuffer;
class MonoBuffer16;

class BandedWGSynthesizer:public TreeSynthesizer
{
public:

   BandedWGSynthesizer(float secondsLength, float sampleRate =44100.0);
   virtual ~BandedWGSynthesizer();
   
   virtual void SynthesizeSample(std::vector<double> *instantParams);
	virtual void SynthesizeFinalizeSample(std::vector<double> *instantParams);


   //returns the buffer that has been synthesized.
   virtual AudioBuffer* GetBuffer();

private:
   bool firstRun; 
	stk::BandedWG bandedWG;
	bool blowing;
	int  noteOrRestDur;
	int  noteOrRestLen;
};

#endif //BowedSynthesizer
