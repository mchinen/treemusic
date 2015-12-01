/*
 *  MultiSynthesizer.h
 *  treemusic
 *
 *  Created by apple on 12/31/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef __MULTISYNTHESIZER__
#define __MULTISYNTHESIZER__
#include "TreeSynthesizer.h"
#include <vector>

class AudioBuffer;
class MonoBuffer16;

class MultiSynthesizer:public TreeSynthesizer
{
public:

   MultiSynthesizer(float secondsLength, float sampleRate =44100.0);
   virtual ~MultiSynthesizer();
   
   virtual void SynthesizeSample(std::vector<double> *instantParams);
   void SynthesizeFinalizeSample(std::vector<double> *instantParams);

   //returns the buffer that has been synthesized.
   virtual AudioBuffer* GetBuffer();

private:
   bool firstRun;
   //synth stuff.
   std::vector<TreeSynthesizer*> m_subsynths;
};

#endif //__NOISEBANDSYNTHESIZER__

