/*
 *  handyfuncs.h
 *  Genesynth
 *
 *  Created by Michael  Chinen on 4/13/06.
 *  Copyright 2006 Michael Chinen. All rights reserved.
 *
 *  a small collection of c functions that are good to have.
 */


#ifndef __HANDYFUNCS__
#define __HANDYFUNCS__

#ifdef __cplusplus

extern "C"{

#endif

 float amptodb(float amp);

 float dbtoamp(float db);

 int randint(int under);
 
 float randfloat(float);
 float randfloatexp2(float under, int divs = -1);
float exp2cursor(float);
 int coinflip();
 
 double mymin(double a, double b);
	 
 double mymax(double a, double b);
 void rfft (double x [],int    N,int    forward);
 void cfft (double x [],int    NC,int    forward );
 void bitreverse (double x [],int    N);

#ifdef __cplusplus
}
#endif

#endif