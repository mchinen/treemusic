//A mono buffer with a int of 16 bitdepth.

#ifndef __MONOBUFFER16__
#define __MONOBUFFER16__

#define SHORTLIMIT 32767
#define SHORTLIMITF 32767.0

#include "AudioBuffer.h"
class MonoBuffer16 : public Genesynth::AudioBuffer
{
public:
	MonoBuffer16(){mBuf = NULL;mNumChannels = 1;smManaged = false;}
	virtual ~MonoBuffer16(){if (mBuf!=NULL) delete [] mBuf;}

	virtual Genesynth::AudioBuffer* Clone() ;
	bool Create(unsigned int frames);

	bool LoadWave(const char* fname,unsigned int start,float level);

	bool WriteWave(const char* fname,unsigned int start, float level);

	void SetFrame(unsigned int frame, long value);

	short GetFrame(unsigned int frame);

	//destructive mix.  Keeps original level.
	virtual void Mix( Genesynth::AudioBuffer* in, unsigned int start, float level,float targetChannel=0,int taperlength = 20,unsigned int inFrom = 0,int inLength=-1, bool undo=false);

	
	//normailze to a DB level  db should be less than 0.
	virtual void Normalize(float db = 0) ;
	
	virtual void SynthesizeIFFT(int N,double complex[]);
	
	//returns a sample with the (-1.0 to 1.0) range, but can be over and under
	//because of the long representation.
	virtual double GetChannelSample(unsigned int channel, unsigned int sample) const;
	
	//expend or shrink with a pitch shift
	virtual void Shift(float rate);
	
	//channel is not important for this virtual call.
	void* GetBuffer(unsigned int channel){return mBuf;}
	
	virtual Genesynth::AudioBuffer* GetChannelBuffer(unsigned int channel){return this;}

	void MakeSin(float freq,float phase = 0.0);
	void AddSin(float freq, unsigned int start, unsigned int length, float level);
protected:
	//secretly represented by a long to avoid clipping for normalization
	//uses SHORTLIMIT AND SHORTLIMITF to act like a short, though.	
	long *mBuf; 
	
	//these next member variables are for the sound manager to change for the purpose of behaving as if
	//the buffer is smaller than the number of longs allocated, and for reserving it or freeing it.
	int  smRealSize,smAmountUsed;
	bool smUsed,smManaged;
};

#endif