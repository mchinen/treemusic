#ifndef __AUDBUFFER_H__
#define __AUDBUFFER_H__

#include <stdio.h>


//for mac little(pc)<-->big(mac) conversions.


#define Swap_32(value)         \
    (((((unsigned int)value)<<24) & 0xFF000000) | \
     ((((unsigned int)value)<< 8) & 0x00FF0000) | \
     ((((unsigned int)value)>> 8) & 0x0000FF00) | \
     ((((unsigned int)value)>>24) & 0x000000FF))

#define Swap_16(value)         \
    (((((unsigned short)value)>> 8) & 0x000000FF) | \
     ((((unsigned short)value)<< 8) & 0x0000FF00))
	 

namespace Genesynth{


class AudioBuffer
{
public:
	AudioBuffer(){mNumChannels = mNumFrames = 0; mSR = 44100; mBitDepth = 16;}
	virtual ~AudioBuffer(){}
	//This function allocates the buffer.
	virtual bool Create(unsigned int frames)=0;
	//this function deep copies a buffer
	virtual AudioBuffer* Clone()  =0;
	//implementations of LoadWave should try to fit the incoming 
	//format into the Dynamic Type of the buffer - if it is a stereo wav 
	//coming in, and LoadWave is called by a MonoBuffer, a mono mix should
	//be the result stored in monobuffer.
	virtual bool LoadWave(const char* fname,unsigned int start,float level) = 0;
	
	//destructive length pitchshift
	virtual void Shift(float rate){}
	
	//naive, non deletrous truncate.
	virtual void Truncate(int newsize);
	
	//write a wave file 
	virtual bool WriteWave(const char* fname,unsigned int start, float level) = 0;

	//destructive mix.  Keeps original level.  the default ins will mix the entire buffer in.
	virtual void Mix( AudioBuffer* in, unsigned int start, float level, float targetChannel=0,int taperlength = 20,unsigned int inFrom = 0,int inLength=-1, bool undo=false)=0;
	
	//normailze to a DB level  db should be less than 0.
	virtual void Normalize(float db = 0) = 0;
	
	//destructive ifft overwrites starting at 0
	virtual void SynthesizeIFFT(int N,double complex[]) = 0;
	
	//this should be in a amp (-1 to 1) scale, regardless of the underlying representation
	virtual double GetChannelSample(unsigned int channel, unsigned int sample) const =0;
	//the rms of the signal, mixed down to mono.
	virtual double GetRMS(unsigned int start, unsigned int rmsLength = 256) const;
	//the frequency response of this buffer .  Arrays MUST exist and be of size N before called.
	virtual void DFT(int start, int N,double real[],double imag[]);
	
	//an in place fft that process N samples, and returns N/2 in place complex values.
	//the first returned complex value has the amplitude of the zero frequency in the real part
	//and the amplitude of the nyquist in the imaginary part.
	//the out[] array should exist before calling.
	virtual void FFT(int start, int N,double outvalues[]) const;
	
	//Signal to noise ratio.  uses *this as a ref.
	double SNR(AudioBuffer* other);
	
	//Spectral Distance measure.
	double SD(AudioBuffer* other, int N = 8192);
		
	//returns the peak in the entire buffer.  the default end of -1 will be taken as the end of the buffer.
	virtual double GetPeak(int start = 0, int end = -1)const;

	virtual void* GetBuffer(unsigned int channel) = 0;

	virtual AudioBuffer* GetChannelBuffer(unsigned int channel) = 0;
	
	void TruncateSilence();

	unsigned int GetNumChannels() const {return mNumChannels;}
	unsigned int GetNumFrames() const {return  mNumFrames;}
	unsigned int GetSampleRate() const {return mSR;}
	void		 SetSampleRate(unsigned int sr){mSR=sr;}
	
	
protected:

		//Writes the header of a wave file - creates it, and File Handle is returned.
	//Meant to be called from Writewave.
	FILE* WriteWaveHeader(const char* fName,unsigned int frames);
	bool   ReadWaveHeader(FILE* fptr, unsigned int &numChannelsOut,
									 unsigned int &numFramesOut,
									 unsigned int &bitDepthOut,
									 unsigned int &sampleRateOut);
	unsigned int mNumChannels;
	unsigned int mNumFrames;
	unsigned int mSR;
	unsigned int mBitDepth;
	
};
}

#endif