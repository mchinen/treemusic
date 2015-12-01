/*
 *  handyfuncs.c
 *  Genesynth
 *
 *  Created by Michael  Chinen on 4/13/06.
 *  Copyright 2006 Michael Chinen. All rights reserved.
 *
 */

#include "math.h"
#include "stdlib.h"


float amptodb(float);
float dbtoamp(float);
int randint(int);
float randfloat(float);
float randfloatexp2(float, int);
float exp2cursor(float);

void cfft (double x [],int NC,int forward );
void bitreverse (double x [],int N);

//this next number essentially should be the number of octaves you want.
#define kRandFloatExp2Limit 9 

float amptodb(float amp)
{
  	return 20 * log10((double)amp);
}

float dbtoamp(float db)
{
	return powf(10,  db/20.0);
}

int randint(int under)
{
	double randpart = rand()/(RAND_MAX+1.0);
	int r =    randpart*under;
	return r;
}

float randfloat(float under)
{
	double randpart = rand()/(RAND_MAX+1.0);
	return  randpart*under;
}

//expontential in div by 2 - the last 50% has the same chance as the 25-50%,12.5-25%bracket, and so on.  this will make
//lower values more likely, but it can't go on for infinity.  we cut it once we reach a 1% bracket.
float randfloatexp2(float under, int divs)
{
	int part = randint(kRandFloatExp2Limit);
	
	if(part>0) 
		return (1.0/(1 << part) + randfloat(1.0/(1<<part)))*under;
	else
	{
		//special case for the remainder.
		return randfloat(1.0/(1<<((divs<1?kRandFloatExp2Limit:divs)-1)));
	}
}

//cursor from 0 to 1, return a logarithmic cursor s.t. the 50%-100% bracket has the same cursor width as the 25-50%etc.  
//we use randfloatexp2limit to define the smallest unit.
float exp2cursor(float cursor)
{
   if(cursor<2.0/((float)kRandFloatExp2Limit))
      return cursor*kRandFloatExp2Limit/2.0 * powf(2.0,(2.0/((float)kRandFloatExp2Limit) - 1.0)* kRandFloatExp2Limit);
   else
      return powf(2.0,(cursor - 1.0)* kRandFloatExp2Limit);
}

int coinflip()
{
	return randint(2);
}

double mymin(double a, double b)
{
	return a<b?a:b;	
}

double mymax(double a, double b)
{
	return a>b?a:b;
}

//borrowed from the Moore text.
void rfft (
		   double x [],
		   int    N,
		   int    forward)
{
	double c1;
	double c2;
	double h1r;
	double h1i;
	double h2r;
	double h2i;
	double wr;
	double wi;
	double wpr;
	double wpi;
	double temp;
	double theta;
	double xr;
	double xi;
	int    i;
	int    i1;
	int    i2;
	int    i3;
	int    i4;
	int    N2p1;
	
	double PII   = 4.0 * atan (1.0);
	
	theta = PII / N;
	wr = 1.0;
	wi = 0.0;
	c1 = 0.5;
	
	if (forward)
	{
		c2 = -0.5;
		cfft (x,N,forward);
		xr = x [0];
		xi = x [1];
	}
	else
	{
		c2    = 0.5;
		theta = -theta;
		xr    = x [1];
		xi    = 0.0;
		x [1] = 0.0;
	}
	
	temp = sin (0.5 * theta);
	wpr  = -2.0 * (temp * temp);
	wpi  = sin (theta);
	N2p1 = (N << 1) + 1;
	
	for (i=0; i<=N>>1; i++)
	{
		i1 = i << 1;
		i2 = i1 + 1;
		i3 = N2p1 - i2;
		i4 = i3 + 1;
		
		if (i == 0)
		{
			h1r =  c1 * (x [i1] + xr);
			h1i =  c1 * (x [i2] - xi);
			h2r = -c2 * (x [i2] + xi);
			h2i =  c2 * (x [i1] - xr);
			
			x [i1] = h1r + wr * h2r - wi * h2i;
			x [i2] = h1i + wr * h2i + wi * h2r;
			
			xr =  h1r - wr * h2r + wi * h2i;
			xi = -h1i + wr * h2i + wi * h2r;
		}
		else
		{
			h1r =  c1 * (x [i1] + x [i3]);
			h1i =  c1 * (x [i2] - x [i4]);
			h2r = -c2 * (x [i2] + x [i4]);
			h2i =  c2 * (x [i1] - x [i3]);
			
			x [i1] =  h1r + wr * h2r - wi * h2i;
			x [i2] =  h1i + wr * h2i + wi * h2r;
			x [i3] =  h1r - wr * h2r + wi * h2i;
			x [i4] = -h1i + wr * h2i + wi * h2r;
		}
		
		wr = (temp = wr) * wpr - wi * wpi + wr;
		wi = wi * wpr + temp * wpi + wi;
	}
	
	if (forward)
		x [1] = xr;
	else
		cfft (x,N,forward);
}

void cfft (
		   double x [],
		   int    NC,
		   int    forward )
{
	double wr;
	double wi;
	double wpr;
	double wpi;
	double theta;
	double scale;
	double temp;
	int    mmax;
	int    ND;
	int    m;
	int    i;
	int    j;
	int    delta;
	
	double TWOPI =  8.0 * atan (1.0);
	ND = NC << 1;
	bitreverse (x,ND);
	
	delta = 0;
	for (mmax=2; mmax<ND; mmax=delta)
	{
		delta = mmax << 1;
		theta = TWOPI / (forward ? mmax : -mmax);
		temp = sin (0.5 * theta);
		wpr = -2.0 * (temp * temp);
		wpi = sin (theta);
		wr = 1.0;
		wi = 0.0;
		
		for (m=0; m<mmax; m+=2)
		{
#ifdef USEREGISTERS
			register double rtemp;
			register double itemp;
#else
			double rtemp;
			double itemp;
#endif
			
			for (i=m; i<ND; i+=delta)
			{
				j        = i + mmax;
				rtemp    = wr * x [j]   - wi * x [j+1];
				itemp    = wr * x [j+1] + wi * x [j];
				x [j]    = x [i] - rtemp;
				x [j+1]  = x [i+1] - itemp;
				x [i]   += rtemp;
				x [i+1] += itemp;
			}
			
			wr = (rtemp = wr) * wpr - wi * wpi + wr;
			wi = wi * wpr + rtemp * wpi + wi;
		}
	}
	
	scale = forward ? 1.0 / ND : 2.0;
	for (i=0; i<ND; i++)
	{
		x [i] *= scale;
	}
}

void bitreverse (
				 double x [],
				 int    N)
{
	double rtemp;
	double itemp;
	int    i;
	int    j;
	int    m;
	
	m = 0;
	for (i=j=0; i<N; i+=2,j+=m)
	{
		if (j>i)
		{
			rtemp   = x [j];
			itemp   = x [j+1];
			x [j]   = x [i];
			x [j+1] = x [i+1];
			x [i]   = rtemp;
			x [i+1] = itemp;
		}
		
		for (m=N>>1; m>=2 && j>=m; m>>=1)
		{
			j -= m;
		}
	}
}



