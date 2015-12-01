
#include "AudioBuffer.h"
#include "handyfuncs.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <math.h>

#define kTruncateSilenceThreshold -90.0
#define kMinNoiseFloorDb -90.0

namespace Genesynth
{
//after calling this function, the file ptr is ready to take in buffer data.
FILE* AudioBuffer::WriteWaveHeader(const char* fname,unsigned int frames)
{
	FILE* fptr;
	fptr = fopen(fname,"wb");
	if(fptr == NULL)
	{
		printf("Error creating file:");
		printf(fname);
		printf("/n");
		return NULL;
	}

	assert( sizeof(int) == 4);
	
	char* riffHeader = "RIFF";
	char* wavHeader	 = "WAVE";
	unsigned int bitDepth = mBitDepth;
	//rawdata+ 12 for the wavHeader,24 for the fmtchunk, 8 for the data chunk, minus 8 for the header.
	unsigned int fileSizeMinus8 = frames*mNumChannels*bitDepth/8 + 12 + 24+ 8 - 8; 
	
	assert(sizeof(int) == 4);
	assert(sizeof(short int) == 2);
	
	fwrite(riffHeader,1,4,fptr); //the RIFF chars
	
//#ifndef LITTLEENDIAN
#ifndef __LITTLE_ENDIAN__
	fileSizeMinus8 = Swap_32(fileSizeMinus8);
#endif
	fwrite(&fileSizeMinus8,4,1,fptr); //the datasize. (MUST BE FILESIZE - 8)
	fwrite(wavHeader,1,4,fptr);
	
	//WAVE Format Chunk:
	//Offset Size Description Value 
	//0x00 4 Chunk ID "fmt " (0x666D7420) 
	char* fmtHeader = "fmt ";
	//0x04 4 Chunk Data Size 16 + extra format bytes 
	unsigned int formatChunkSize = 16;
	//0x08 2 Compression code 1 - 65,535  - 1 is uncompressed
	unsigned short int compressionCode = 0x0001;
	//0x0a 2 Number of channels 1 - 65,535 
	unsigned short int numberChannels = mNumChannels;
	//0x0c 4 Sample rate 1 - 0xFFFFFFFF 
	//0x10 4 Average bytes per second 1 - 0xFFFFFFFF 
	//0x14 2 Block align 1 - 65,535 
	//0x16 2 Significant bits per sample 2 - 65,535 
	unsigned short int significantBits = bitDepth;
	unsigned short int blockAlign = significantBits* numberChannels/8;
	unsigned int  averageBytesPerSec = blockAlign * mSR;
	unsigned int sampleRate = mSR;
	unsigned int dataSize = frames*mNumChannels*significantBits/8;
	
	
	fwrite(fmtHeader,1,4,fptr);
//#ifndef LITTLEENDIAN
#ifndef __LITTLE_ENDIAN__
	formatChunkSize = Swap_32(formatChunkSize);
	compressionCode = Swap_16(compressionCode);
	numberChannels = Swap_16(numberChannels);
	
	sampleRate = Swap_32(sampleRate);
	averageBytesPerSec = Swap_32(averageBytesPerSec);
	blockAlign = Swap_16(blockAlign);  
	significantBits = Swap_16(significantBits);
	
	dataSize = Swap_32(dataSize);
#endif
	fwrite(&formatChunkSize,4,1,fptr);
	fwrite(&compressionCode,2,1,fptr);
	fwrite(&numberChannels,2,1,fptr);
	fwrite(&sampleRate,4,1,fptr);
	fwrite(&averageBytesPerSec,4,1,fptr);
	fwrite(&blockAlign,2,1,fptr);
	fwrite(&significantBits,2,1,fptr);
	
	
	//DATA chunk.
	//0x00 4 char[4] chunk ID "data" (0x64617461) 
	//0x04 4 dword chunk size depends on sample length and compression 
	//0x08 sample data 
	char* dataHeader = "data";
	
	
	fwrite(dataHeader,1,4,fptr);
	fwrite(&dataSize,4,1,fptr);
	
	return fptr;
}

bool  AudioBuffer::ReadWaveHeader(FILE* fptr, unsigned int &numChannelsOut,
								  unsigned int &numFramesOut,
								  unsigned int &bitDepthOut,
								  unsigned int &sampleRateOut)
{
	
	
	//	unsigned int bitDepth = mBitDepth;
	//rawdata+ 12 for the wavHeader,24 for the fmtchunk, 8 for the data chunk, minus 8 for the header.
	//	unsigned int fileSizeMinus8;// = frames*mNumChannels*bitDepth/8 + 36; 
	
	assert(sizeof(int) == 4);
	assert(sizeof(short int) == 2);
	
	char readByte;
	char readStr[5];
	unsigned short readShort;
	unsigned int   readInt;
	
	fread(readStr,1,4,fptr); //the RIFF chars
	readStr[4] = 0; //null term. for strcmp
	if(0 != strcmp("RIFF",readStr))
	{
		printf("Not a Wave File - did not start with RIFF header.\n");
		fclose(fptr);
		return false;
	}
	fread(&readInt,4,1,fptr); //the datasize. (MUST BE FILESIZE - 8)
	
	
	fread(readStr,1,4,fptr); //the WAVE chars
	readStr[4] = 0; //null term. for strcmp
	if(0 != strcmp("WAVE",readStr))
	{
		printf("Not a Wave File - did not start with WAVE header.\n");
		fclose(fptr);
		return false;
	}
	//WAVE Format Chunk:
	//Offset Size Description Value 
	//0x00 4 Chunk ID "fmt " (0x666D7420) 
	//	char* fmtHeader = "fmt ";
	//0x04 4 Chunk Data Size 16 + extra format bytes 
	//	unsigned int formatChunkSize = 16;
	//0x08 2 Compression code 1 - 65,535  - 1 is uncompressed
	//	unsigned short int compressionCode = 0x0001;
	//0x0a 2 Number of channels 1 - 65,535 
	//	unsigned short int numberChannels = mNumChannels;
	//0x0c 4 Sample rate 1 - 0xFFFFFFFF 
	//0x10 4 Average bytes per second 1 - 0xFFFFFFFF 
	//0x14 2 Block align 1 - 65,535 
	//0x16 2 Significant bits per sample 2 - 65,535 
	//	unsigned short int significantBits = bitDepth;
	//	unsigned short int blockAlign = significantBits* numberChannels/8;
	//	unsigned int  averageBytesPerSec = blockAlign * mSR;
	
	fread(readStr,1,4,fptr);
	fread(&readInt,4,1,fptr);//format chunk size
		
		fread(&readShort,2,1,fptr);//compression code
//#ifndef LITTLEENDIAN
#ifndef __LITTLE_ENDIAN__
			readShort = Swap_16(readShort);
#endif	
			if(readShort != 1)//readshort ==3 means float, i think
			{
				printf("File is compressed and I don't know how to deal.\n");
				fclose(fptr);
				return false;
			}
			fread(&readShort,2,1,fptr);//numChannels
				numChannelsOut = readShort;
				
//#ifndef LITTLEENDIAN
#ifndef __LITTLE_ENDIAN__
				numChannelsOut = Swap_16(numChannelsOut);
#endif	
				
				fread(&sampleRateOut,4,1,fptr);
//#ifndef LITTLEENDIAN
#ifndef __LITTLE_ENDIAN__
				sampleRateOut = Swap_32(sampleRateOut);
#endif		
				
				fread(&readInt,4,1,fptr); //bytes per second
				fread(&readShort,2,1,fptr);//block align
					fread(&readShort,2,1,fptr);//bitdepth
						bitDepthOut = readShort;
						
//#ifndef LITTLEENDIAN

#ifndef __LITTLE_ENDIAN__
						bitDepthOut = Swap_16(bitDepthOut);
#endif		
						//DATA chunk.
						//0x00 4 char[4] chunk ID "data" (0x64617461) 
						//0x04 4 dword chunk size depends on sample length and compression 
						//0x08 sample data 
						//	char* dataHeader = "data";
						//	unsigned int dataSize = frames*mNumChannels*significantBits/8;
						
						fread(readStr,1,4,fptr);
						if(0 != strcmp("data",readStr) && 0 != strcmp("DATA",readStr) )
						{
							printf("Parsed thru the whole file but there was extra formating thing that I don't understand.  Passing.\n");
							fclose(fptr);
							return false;
						}
						fread(&readInt,4,1,fptr);//datasize
//#ifndef LITTLEENDIAN

#ifndef __LITTLE_ENDIAN__
							readInt = Swap_32(readInt);
#endif			
							
							numFramesOut = readInt*8 / (bitDepthOut* numChannelsOut);
							
							//ready to read the raw data.
							return true;
}


// a range from 0 to 1.0
double AudioBuffer::GetRMS(unsigned int start, unsigned int rmsLength) const
{
	double sample, rms;
	
	if(start >= mNumFrames)
		return 0.0;
	
	if(start+rmsLength > mNumFrames)
		rmsLength = mNumFrames - start;
	
	sample = rms = 0.0;
	
	for(int i = start; i < rmsLength+start; i++)
	{
		for(int j = 0; j< mNumChannels; j ++)
			sample +=  GetChannelSample(j,i) * (1.0/mNumChannels);
		
		rms += sample*sample;
		sample = 0.0;
	}
	
	
	rms = sqrt(rms / rmsLength);
	
	return rms;
	
	
}

//non deletrous truncate.
void AudioBuffer::Truncate(int newsize)
{
	mNumFrames=mymin(mNumFrames,newsize);
}

void AudioBuffer::TruncateSilence()
{
	bool silence = true;
	for(int i=mNumFrames;i>0&&silence;i--)
	{
		for(int j=0;j<this->GetNumChannels();j++)
		{
			if(amptodb(GetChannelSample(j,i))>kTruncateSilenceThreshold) 
				silence=false;
		}
		mNumFrames=i;
	}
}

//slow dft, for non 2^n cases, should they arise
//all arrays must exist.
void AudioBuffer::DFT(int start, int N,double real[],double imag[])
{
	//for now, this uses the algorithm in Moore's Elements of C.M. 
	double pi2oN = 8.*atan(1.)/N;
	
	for(int k=0;k<N;k++)
	{
		real[k]=imag[k]=0.0;
		for(int j=0; j<N;j++)
		{
			//get a sample (mixed if multi channel)
			double sample;
			sample=0.0;
			for(int l =0;l<mNumChannels;l++)
				sample += GetChannelSample(l,j+start)/mNumChannels;
			
			real[k]+= sample*cos(pi2oN*k*j);
			imag[k] -= sample*sin(pi2oN*k*j);
		}
		//normalize.
		real[k] = real[k]/ N;
		imag[k] = imag[k]/N;
	}
}

//faster in place FFT.   out already exists with a size of N doubles.
void AudioBuffer::FFT(int start, int N,double outvalues[]) const
{
	//fill the array with audio
	for(int i=0;i<N;i++)
	{
		outvalues[i] = GetChannelSample(0,i+start)/mNumChannels;
		
		for(int l =1;l<mNumChannels;l++)
			outvalues[i] += GetChannelSample(l,i+start)/mNumChannels;
	}
	
	//use an fft.  rfft takes its n as the number of complex values returned.
	rfft(outvalues,N/2,true);
	
}
double AudioBuffer::SNR(AudioBuffer* other)
{
	double num,dem,temp,total;
	
	int segment = 1000;
	int numsegments = mNumFrames/segment +1;
	
	total=0.0;
	for(int i =0;i<numsegments;i++)
	{
		num=dem=0.0;
		for(int j =0;j<segment;j++)
		{
			temp = GetChannelSample(0,i*segment +j);
			num+= temp*temp;
			temp -= other->GetChannelSample(0,i*segment+j);
			dem += temp*temp;
		}
		total += 10.0*log10(num/dem);
	}
	
	return total/numsegments;
}

//TODO: hamming window is currently size of hopsize - should be independent windowsize.
double AudioBuffer::SD(AudioBuffer* other, int N)
{
	
	int hopsize;
	int numWindows;
	int scoredBins;
	int totalScoredBins;
	int scoredWindows;
	int start;
	bool audible;
	double* outvaluesa;
	double* outvaluesb;
	double* ham;
	
	double cutoffdb = -35.0;
	double bincutoffdb = -80.0;
	
	outvaluesa = new double[N];
	outvaluesb = new double[N];
	
	for(int i =0;i<N;i++)
	{
		outvaluesa[i]=outvaluesb[i]=0.0;
	}
	
	hopsize = N/8; 
	ham = new double[hopsize];
	
	for(int n=0;n<hopsize;n++)
		ham[n] = 0.54 - 0.46*cos(n*3.141592*2.0/(hopsize-1));
	
	numWindows = mNumFrames / hopsize -1;
	
	double cumscore,temp,a,b,framescore;
	
	cumscore =0.0;
	scoredWindows = 0;
	
	for(int i=1;i<numWindows;i++)
	{
		audible = false;
		framescore = 0.0;
		
		for(int j=0;j<hopsize;j++)
		{
			
			outvaluesa[j] = GetChannelSample(0,j+i*hopsize)*ham[j];
			outvaluesb[j] = other->GetChannelSample(0,j+i*hopsize)*ham[j];
			if(amptodb(outvaluesa[j]) > cutoffdb)
				audible = true;
		}
		if(amptodb(GetRMS(i*hopsize,hopsize) ) < cutoffdb)
			audible = false;
		if(!audible)
			printf("not audible frame %i\n",i);
		if(audible)
		{
			scoredWindows++;
		
			rfft(outvaluesa,N/2,true);
			rfft(outvaluesb,N/2,true);
		
		//nyquist, 0hz
		//temp= mymax(-60.0,amptodb(outvaluesa[0]))-mymax(-60.0,amptodb(outvaluesb[0]));
		//framescore+=temp*temp;
		//temp= mymax(-60.0,amptodb(outvaluesa[1]))-mymax(-60.0,amptodb(outvaluesb[1]));
		
		//framescore+=temp*temp;
		    scoredBins = 0;
			for(int j =1;j < N/2;j++)
			{
				a=mymax(bincutoffdb,amptodb(sqrt(outvaluesa[j*2]*outvaluesa[j*2] + outvaluesa[j*2+1]*outvaluesa[j*2+1])));
				b=mymax(bincutoffdb,amptodb(sqrt(outvaluesb[j*2]*outvaluesb[j*2] + outvaluesb[j*2+1]*outvaluesb[j*2+1])));

				temp = a-b;
	//			if(!(a== bincutoffdb && b==bincutoffdb ))
	//			{	
					scoredBins++;
					framescore += temp*temp;
	//			}
			}
			if(scoredBins)
				cumscore += sqrt( framescore/scoredBins);
			else
			{
				printf("not scoring %i\n",i);
				scoredWindows--;
			}
		}
	}
	
	delete [] outvaluesa;
	delete [] outvaluesb;
	delete [] ham;
	
	return cumscore/scoredWindows;
	
}

//if left at default -1, do entire buffer.
double AudioBuffer::GetPeak(int start,int end) const
{
	int till;
	if(-1 == end)
		till =mNumFrames;
	else
		till = end;
	
	double peak = 0.0;
	double sample;
	for(int i =start;i<mymin(mNumFrames,till);i++)
	{
		for(int j =0;j<mNumChannels;j++)
		{
			sample = fabs(GetChannelSample(j,i));
			if(peak<sample)
				peak = sample;
		}
	}
	return peak;
}


}
