/*
 *  TreeSynthesizer.h
 *  treemusic
 *
    abstract class that synthesizes based on a set of instantaneous parameters.
    subclasses should implement some memory for these parameters.
 
 *  Created by Michael Chinen on 11/25/08.
 *  Copyright 2008 MCDC. All rights reserved.
 *
 */
#ifndef __TREESYNTHESIZER__
#define __TREESYNTHESIZER__

#include <vector>
class AudioBuffer;
class MonoBuffer16;
class TreeSynthesizer
{
public:
   TreeSynthesizer(float sampleRate =44100.0):mSampleRate(sampleRate){}
   virtual ~TreeSynthesizer(){}
   
   float GetSampleRate(){return mSampleRate;}
   virtual void SynthesizeSample(std::vector<double> *instantParams) = 0;
   virtual void SynthesizeFinalizeSample(std::vector<double> *instantParams) {}
   
   virtual long GetLastLongSample();
   //returns the buffer that has been synthesized.
   virtual AudioBuffer* GetBuffer()=0;
protected:
float mSampleRate;
long int currentSample;
   MonoBuffer16* buffer;
};
#endif //#ifndef __TREESYNTHESIZER__