/*
 *  StereoBuffer16.cpp
 *  Genesynth
 *
 *  Created by Michael  Chinen on 5/27/06.
 *  Copyright 2006 Michael Chinen. All rights reserved.
 *
 */

#include "StereoBuffer16.h"
#include "MonoBuffer16.h"
#include "handyfuncs.h"
#include <memory>

const float PI =3.141592;
const int   SR = 44100;
const int   READINCHUNKSIZE = SR*30;

StereoBuffer16::StereoBuffer16()
{
	m_left = m_right = NULL;
	mNumChannels=2;
}
StereoBuffer16::~StereoBuffer16()
{
	if(m_left)
		delete m_left;
	if(m_right)
		delete m_right;
}

//TODO: make this smart-resizable
bool StereoBuffer16::Create(unsigned int frames)
{
	m_left = new MonoBuffer16();
	m_left->Create(frames);
	m_right = new MonoBuffer16();
	m_right->Create(frames);
	mNumFrames = frames;

	return true;
}


//this function deep copies a buffer
Genesynth::AudioBuffer* StereoBuffer16::Clone() 
{
	StereoBuffer16* harry = new StereoBuffer16();
	if(m_left)
		harry->m_left = (MonoBuffer16*)m_left->Clone();
	if(m_right)
		harry->m_right = (MonoBuffer16*)m_right->Clone();
	harry->mNumFrames=mNumFrames;
	
	return harry;
}

//if mono in, copy mono to both sides.
bool StereoBuffer16::LoadWave(const char* fname,unsigned int start,float level)
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
		
		long *bufl = (long*)m_left->GetBuffer(0);
		long *bufr = (long*)m_right->GetBuffer(0);

		while(frames>0)
		{
			fread(temp,2, (READINCHUNKSIZE<frames?READINCHUNKSIZE:frames)*channels ,fptr);
			
			for(int i=0;i< (READINCHUNKSIZE<frames?READINCHUNKSIZE:frames) ;i++)
			{
				
				if(2==channels)
				{
					short l = temp[i*2];
					short r = temp[i*2 +1];
#ifndef __LITTLE_ENDIAN__
					l = Swap_16(l);
					r = Swap_16(r);
#endif
					bufl[i+cursor] = l*level;
					bufr[i+cursor] = r * level;
				}
				else if (1 == channels)
				{
					short t = temp[i];
#ifndef __LITTLE_ENDIAN__
					t = Swap_16(t);
#endif
					bufl[i+cursor] = t*level;
					bufr[i+cursor] = t*level;
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

//write a wave file 
bool StereoBuffer16::WriteWave(const char* fname,unsigned int start, float level)
{
	//check to see if we have anything.
	if(NULL == m_left || NULL == m_right || 0 == mNumFrames)
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
	short valuel,valuer;
	long *bufl = (long*)m_left->GetBuffer(0);
	long *bufr = (long*)m_right->GetBuffer(0);
	for(int i = start; i < mNumFrames;i++)
	{
		//have to pretend a short 
		valuel = mymax(mymin(bufl[i],SHORTLIMIT),-SHORTLIMIT) * level;
		valuer = mymax(mymin(bufr[i],SHORTLIMIT),-SHORTLIMIT) * level;
//#ifndef LITTLEENDIAN
#ifndef __LITTLE_ENDIAN__
		valuel = Swap_16(valuel);
		valuer = Swap_16(valuer);
#endif
		fwrite(&valuel,2,1,fptr);
		fwrite(&valuer,2,1,fptr);
	}
	fclose(fptr);
	printf("wrote the file %s\n", fname);
	return true;
	

}

//destructive mix.  Keeps original level.  the default ins will mix the entire buffer in.
void StereoBuffer16::Mix( Genesynth::AudioBuffer* in, unsigned int start, float level,float targetChannel,int taperlength,unsigned int inFrom,int inLength, bool undo)
{
	if(NULL==in)
		return;
	if(1==in->GetNumChannels())
	{
		m_left->Mix(in,start,level*(1.0-targetChannel),0,taperlength,inFrom,inLength,undo);
		m_right->Mix(in,start,level*targetChannel,0,taperlength,inFrom,inLength,undo);
	}
	//otherwise mix over all the channels even into left, right into odd.
	else
	{
		for(int i =0;i<in->GetNumChannels();i++)
		{
			if(i%2 == 0)
				m_left->Mix(in->GetChannelBuffer(i),start,level,0,taperlength,inFrom,inLength,undo);
			else
				m_right->Mix(in->GetChannelBuffer(i),start,level,0,taperlength,inFrom,inLength,undo);
		}
	}
	mNumFrames = mymax(mNumFrames,mymax(m_left->GetNumFrames(),m_right->GetNumFrames()));
}

//normailze to a DB level  db should be less than 0.  does a channel linked normalize.
void StereoBuffer16::Normalize(float db)
{
	float lpeak,rpeak;
	lpeak = m_left->GetPeak();
	rpeak = m_right->GetPeak();
	
	if(lpeak>rpeak)
	{
		m_left->Normalize(db);
		m_right->Normalize(db + amptodb(rpeak/lpeak));//done with lpeak on bottom to assure non zerodiv.
	}else
	{
		m_right->Normalize(db);
		m_left->Normalize(db + amptodb(lpeak/rpeak));
		
	}
}

//destructive ifft overwrites starting at 0
void StereoBuffer16::SynthesizeIFFT(int N,double complex[]){}

//this should be in a amp (-1 to 1) scale, regardless of the underlying representation
double StereoBuffer16::GetChannelSample(unsigned int channel, unsigned int sample) const
{
	return channel?m_right->GetChannelSample(0,sample):m_left->GetChannelSample(0,sample);
}

void* StereoBuffer16::GetBuffer(unsigned int channel)
{
	return channel?m_right:m_left;
}

Genesynth::AudioBuffer* StereoBuffer16::GetChannelBuffer(unsigned int channel) 
{
	return channel?m_right:m_left;
}

void StereoBuffer16::SetBuffer(unsigned int channel, MonoBuffer16* buf)
{}