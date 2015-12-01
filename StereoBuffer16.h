/*
 *  StereoBuffer16.h
 *  Genesynth
 *
 *  Created by Michael  Chinen on 5/27/06.
 *  Copyright 2006 Michael Chinen. All rights reserved.
 *
 *  This class is not complicated at all, because it just uses two
 *  MonoBuffer16s.  They can be attatched and detatched through the
 *  accessors.
 */

#ifndef __STEREOBUFFER16__
#define __STEREOBUFFER16__

#include "AudioBuffer.h"
class MonoBuffer16;

class StereoBuffer16:public Genesynth::AudioBuffer
{
public:
	StereoBuffer16();
	~StereoBuffer16();
	
	virtual bool Create(unsigned int frames);
	//this function deep copies a buffer
	virtual Genesynth::AudioBuffer* Clone() ;
	//if mono in, copy mono to both sides.
	virtual bool LoadWave(const char* fname,unsigned int start,float level);
	
	//write a wave file 
	virtual bool WriteWave(const char* fname,unsigned int start, float level);
	
	//destructive mix.  Keeps original level.  the default ins will mix the entire buffer in.
	virtual void Mix( AudioBuffer* in, unsigned int start, float level,float targetChannel=0.5,int taperlength = 20,unsigned int inFrom = 0,int inLength=-1, bool undo=false);
	
	//normailze to a DB level  db should be less than 0.  does a channel linked normalize.
	virtual void Normalize(float db = 0);
	
	//destructive ifft overwrites starting at 0
	virtual void SynthesizeIFFT(int N,double complex[]);
	
	//this should be in a amp (-1 to 1) scale, regardless of the underlying representation
	virtual double GetChannelSample(unsigned int channel, unsigned int sample) const;

	virtual void* GetBuffer(unsigned int channel);
	
	virtual Genesynth::AudioBuffer* GetChannelBuffer(unsigned int channel);
	
	void SetBuffer(unsigned int channel, MonoBuffer16* buf);
	
protected:
	MonoBuffer16* m_left;
	MonoBuffer16* m_right;
};

#endif
