
#include "AudioBuffer.h"
#include "MonoBuffer16.h"
#include "handyfuncs.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

const float PI =3.141592;
const int   SR = 44100;
const int   READINCHUNKSIZE = SR*30;

//clone
Genesynth::AudioBuffer* MonoBuffer16::Clone() 
{
	MonoBuffer16* mb= new MonoBuffer16;
   
   mb->Create(mNumFrames);
	mb->Mix(this, 0,1.0,0,0);
	return mb;
}

//This function allocates the memory for a buffer size samples, 
//and destroys any old data that was on it previously
bool MonoBuffer16::Create(unsigned int samples)
{
	//we want to recycle allocated memory, so only delete mBuf if it isn't big enough
    if(mBuf != NULL && samples > mNumFrames)
	{
		delete [] mBuf;
		mBuf = NULL;
	}
	mNumFrames = samples;
	if(mBuf == NULL)
	{
		//only if we actually adjust the size.
		smRealSize = samples;
		//if it is one of the monitored buffers, we need to readjust the sorted array.
		//SoundManager::GetInstance()->ReinsertBuffer16(this);
		mBuf = new long[samples];
	}
	
	if(mBuf!= NULL)
	{
		for(int i = 0 ; i < samples;i++)
			mBuf[i] = 0;
		return true;
	}
	mNumFrames = smRealSize = 0;
	return false;
}
bool MonoBuffer16::LoadWave(const char* fname,unsigned int start,float level)
{
	FILE* fptr;
	unsigned int channels,frames,bitDepth,sr,cursor;
 
	cursor = start;
	fptr = fopen(fname,"rb");
	if(fptr == NULL)
	{
		printf("Error opening file: %s",fname);
		return false;
	}

	if(!ReadWaveHeader(fptr,channels,frames,bitDepth,sr))
	{
		printf("Error opening file: %s",fname);
		return false;
	}

	//read data.
	
	if(16 == bitDepth)
	{
		short *temp =(short *) malloc(sizeof(short)*(READINCHUNKSIZE<frames?READINCHUNKSIZE:frames)*channels);//READINCHUNKSIZE instead of frames
		if(NULL == temp || !Create(frames-start))
			return false;
		
		while(frames>0)
		{
			fread(temp,2, (READINCHUNKSIZE<frames?READINCHUNKSIZE:frames)*channels ,fptr);

			for(int i=0;i< (READINCHUNKSIZE<frames?READINCHUNKSIZE:frames) ;i++)
			{
			
				if(2==channels)
				{
					short l = temp[i*2];
					short r = temp[i*2 +1];
//#ifndef LITTLEENDIAN
#ifndef __LITTLE_ENDIAN__
					l = Swap_16(l);
					r = Swap_16(r);
#endif
					mBuf[i+cursor] = (l+r) * 0.5 * level;
				}
				else if (1 == channels)
				{
					short t = temp[i];
//#ifndef LITTLEENDIAN

#ifndef __LITTLE_ENDIAN__
					t = Swap_16(t);
#endif
					mBuf[i+cursor] = t*level;
				}
			}
			frames = frames - (READINCHUNKSIZE<frames?READINCHUNKSIZE:frames);
			cursor+= READINCHUNKSIZE;
		}
		fclose(fptr);

		free(temp);
		return true;
	}
	return false;
}
bool MonoBuffer16::WriteWave(const char* fname,unsigned int start, float level)
{
	//check to see if we have anything.
	if(NULL == mBuf && 0 == mNumFrames)
		return false;

	FILE* fptr;
	fptr = WriteWaveHeader(fname, mNumFrames-start);

	if(NULL == fptr)
	{   //couldn't come through.
		return false;
	}
	
	//writing 1 short at a time might not be that fast- if this is 
	//an issue, we'll have to create a temp buff at the level instead
	//and do the entire buffer in one fwrite.
	short value;
	for(int i = start; i < mNumFrames;i++)
	{
		//have to pretend a short 
		value = mymax(mymin(mBuf[i],SHORTLIMIT),-SHORTLIMIT) * level;
//#ifndef LITTLEENDIAN
#ifndef __LITTLE_ENDIAN__
		value = Swap_16(value);
#endif
		fwrite(&value,2,1,fptr);
	}
	fclose(fptr);
	printf("wrote the file %s\n", fname);
	return true;
}

short MonoBuffer16::GetFrame(unsigned int frame)
{
	if(mNumFrames <= frame)
		return 0;

	return mBuf[frame] ;
}
void MonoBuffer16::SetFrame(unsigned int frame, long value)
{
	if(mNumFrames <= frame)
		return;

	mBuf[frame] = value;
}

void MonoBuffer16::Shift(float rate)
{
	
	int newsize = mNumFrames/rate;
	if(newsize<2)
		return;
	
	int toframes=mNumFrames;
	double sizeratio = (double)(toframes-1)/(newsize-1);
	long* to=mBuf;
	//if the rate is less than one we need to expand.
	if(rate<1.0)
	{
		if(newsize>smRealSize)
		{
			long *newBuf  = new long[newsize];
			mNumFrames = smRealSize = newsize;
			to=mBuf;
			mBuf = newBuf;
			//SoundManager::GetInstance()->ReinsertBuffer16(this);
		}
		
		//go backwards because we write more than we seek.
		int baseindex;
		float loc,val;
		for(int i =newsize-1;i>=0;i--)
		{
			baseindex = i*sizeratio;
			loc = ((float)i*sizeratio)- ((int)i*sizeratio);
			if(loc ==0.0)
				val=to[baseindex];
			else
				val=to[baseindex]+to[baseindex+1]*loc;
			
			mBuf[i]=val;
		}
		if(to&&to!=mBuf) 
			delete [] to;		
	}
	else if(rate>1.0)
	{
		//write but don't get written
		//(go forward since the seek cursor moves faster than the write)
		int baseindex;
		float loc,val;
		for(int i =0;i<newsize;i++)
		{
			baseindex = i*sizeratio;
			loc = ((float)i*sizeratio) - ((int)i*sizeratio);
			if(loc ==0.0)
				val=to[baseindex];
			else
				val=to[baseindex]+to[baseindex+1]*loc;
			
			mBuf[i]=val;
		}
		//you are the proud owner of a smaller buffer
		mNumFrames=newsize;
		
	}
	
	
}

//MonoBuffer16* MonoBuffer16::MakeChordlet(int start, int end


//destructive mix.  Keeps original level.  appends to end.
//note that the things won't clip with the longs until things get really heavy
void MonoBuffer16::Mix( Genesynth::AudioBuffer* in, unsigned int start,float level,float targetChannel,int taperlength,unsigned int inFrom,int inLength, bool undo)
{
	
	if(in == NULL)
	{
		printf("Warning: attempting to mix NULL buffer.\n");
		return;
	}
	
	
	int inSamples;
	if(-1 == inLength)
		inSamples = in->GetNumFrames();
	else
		inSamples = mymin(in->GetNumFrames()-inFrom,inLength);
	//first see if our buffer is long enough. if it is not,
	//Use realSize and not numFrames, because the audio buffer might already exist.  In this case,
	//we need to zero the tail data as it might be dirty.
	//otherwise just create a new one
	if(inSamples+start > mNumFrames && inSamples+start<=smRealSize)
	{
		for(int i = mNumFrames;i<smRealSize;i++)
			mBuf[i]=0;
		mNumFrames = inSamples+start;
	}
	else if(inSamples+ start > smRealSize)
	{
		unsigned int newSize = inSamples + start;
		long *newBuf  = new long[newSize];
		
		//SoundManager::GetInstance()->mixResizes++;
		//SoundManager::GetInstance()->mixRatio += (double)newSize/(double)smRealSize;
		//copy over.
		for(int i=0; i < mNumFrames;i++)
			newBuf[i] = mBuf[i];
		for(int i = mNumFrames; i< newSize;i++)
			newBuf[i] = 0;
		mNumFrames = smRealSize = newSize;
		if(mBuf) 
			delete [] mBuf;
		mBuf = newBuf;
		
		//SoundManager::GetInstance()->ReinsertBuffer16(this);
	}
    //make sure taperlength is valid.  taperlength applies to taperlength amount of samples on both sides
	if(taperlength*2>inSamples)
		taperlength = mymax(1, inSamples/2 -1);
	
	
	//mix here with envelope
	for(int i = 0; i < inSamples;i++)
	{
		
		float envelope =1.0;
		
		if(taperlength>0)
		{
			envelope= fabs((float)i - (inSamples/2)) + taperlength - inSamples/2;
			if(envelope<0.0) 
				envelope = 0.0;
			envelope = 1.0 - (float)(envelope/taperlength);  //this will taper off the edges.
		}
		if(undo)
			mBuf[i+start] = mBuf[i+start] - (in->GetChannelSample(0,i+inFrom)*SHORTLIMIT) * (level *envelope);	
		else
			mBuf[i+start] += (in->GetChannelSample(0,i+inFrom)*SHORTLIMIT) * (level *envelope);
	}
}

//normailze to a DB level  db should be less than 0.
void MonoBuffer16::Normalize(float db ) 
{
	//find out the max amp in our file.
	long maxVolume= 0;
	for(int i =0; i<mNumFrames;i++)
		maxVolume = mymax(maxVolume, abs(mBuf[i]));
	
	if(maxVolume==0)
		return;
	
	//compute the ratio between the two
	double maxAmp = maxVolume / SHORTLIMITF ;
	double targetAmp = dbtoamp(db);
	
	if(targetAmp == 0.0) 
		return;
	double scalar = targetAmp / maxAmp;
						
	
	for(int i =0; i<mNumFrames;i++)
		mBuf[i]= (long) mBuf[i]*scalar;
}


void MonoBuffer16::SynthesizeIFFT(int N,double complex[])
{	
	//use an fft.  rfft takes its n as the number of samples returned.
	rfft(complex,N/2,false);
	
	//fill the array with audio
	for(int i=0;i<mNumFrames && i <N;i++)
	{
		mBuf[i]= complex[i] * SHORTLIMITF;
	}
}

//if the sound clips, we return the real value
//CHANGED FROM OLD WHICH WAS: + or -1.0.  if it isn't in bounds, we return 0.
double MonoBuffer16::GetChannelSample(unsigned int channel,unsigned int sample) const
{
	if(sample >= mNumFrames || sample< 0)
		return 0.0;
	return (double) mBuf[sample]/SHORTLIMITF; //channel is not important in a mono file.
}
//makes the whole buffer into a sin, destroying whatever was in it.
void MonoBuffer16::MakeSin(float freq,float phase)
{
	for(int i = 0 ; i<mNumFrames;i++)
	{	
		short int value = sin( ((double)i*freq*2.0*PI /SR)+((double) phase))*SHORTLIMITF;

		mBuf[i] =  value;
	}
}
void MonoBuffer16::AddSin(float freq, unsigned int start, unsigned int length, float level)
{
	MonoBuffer16 temp;

	temp.Create(length);
	temp.MakeSin(freq);

	Mix(&temp,start,level);
}