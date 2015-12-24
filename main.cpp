#include <iostream>
#include "TreeNode.h"
#include <time.h>
#include <stdio.h>
#include <thread>

#include "ParameterList.h"
#include "StereoBuffer16.h"
#include "MonoBuffer16.h"
#include "NoiseBandSynthesizer.h"

#include "SaxHackSynthesizer.h"
#include "VoiceHackSynthesizer.h"
#include "BowedSynthesizer.h"
#include "BandedWGSynthesizer.h"
#include "ShakersSynthesizer.h"

#include "MultiSynthesizer.h"
//#include "ParamOutputSynthesizer.h"
#include "handyfuncs.h"

int main (int argc, char * const argv[]) {
    // insert code here...
//    std::cout << "Hello, World!\n";
   int  stime;
  long ltime;

  /* get the current calendar time */
  ltime = time(NULL);
  stime = (unsigned) ltime/2;
  srand(stime);


   TreeNode* tree;
   ParameterList params;
   
   tree=new TreeNode(0);
   
   /*
   
enum Parameter
{
   eAmplitude=0,
   eCenterFrequency,
   eBandwidth,
   eLength,
   eSkip,
   eRandomness,
   eMutation,
   eGrainStart,
   eShift,
   eNumParams
};

   for(int i=0;i<100;i++)
   {
      printf("value of randfloatexp2(%f)=%f\n",(float)i/99.0,randfloatexp2((float)i/99.0));
      printf("value of exp2cursor(%f)=%f\n",(float)i/99.0, exp2cursor((float)i/99.0));

   }
*/
   //we need to add each parameter manually, to set it's mins and maxes.
   //nodte that because this program was hacked together in a week that these parameters need to be
   //added in the order of their enum (enumeration).
   
   float startFreq = 1000.0;
   float spacing = 20.0;

   params.SetNumTopLayers(kMaxLayers-1);


/*for sax*/	
	params.AddParameter(eBreathPressure,127.0,0.005,128.0);
	params.AddParameter(eBlowPosition,64.0,0.0005,128.0);
	params.AddParameter(eReedStiffness,64.0,0.0005,128.0);
	params.AddParameter(eReedAperature,64.0,0.0005,128.0);
	params.AddParameter(eCenterFrequency,200,35.0,3600.0);
	params.AddParameter(eVibratoFrequency,5.735*128.0/12.0,0.005,128.0); //these are set wrt the way Saxofony.cpp handles controlChange
	params.AddParameter(eVibratoGain,0.1*2*128,0.005,128.0);
	params.AddParameter(eNoiseGain,0.2*128/0.4,0.005,128.0);
	params.AddParameter(eMaxBlowLength,5.0,0.01,45.0); //in seconds
	params.AddParameter(eMaxRestLength,0.3,0.001,3.0);
	params.AddParameter(eInstrumentNum,1.0,0.001,22.99); //shakers has 23 instruments (0-22)
 /* for noise band*/

   params.AddParameter(eAmplitude,0.3,0.005,1.0);
   params.AddParameter(eBandwidth,0.3,0.01,0.9);
   params.AddParameter(eMultiInstNum,0.001,0.0001,15.0);//for multisynthesizer


	
//   params.AddParameter(eRandomness,0.0,0.000001,1.0);
  // params.AddParameter(eLength,0.5,0.000001,20.0);//in seconds.
   
   float duration = 60*3.14159265 * 2;
   std::vector<ParameterList*> paramLists;
   paramLists.push_back(&params);
   tree->Generate(/*startTime*/0.0,duration, &paramLists);
   
   tree->PrintFile("firsttest.txt",stime);
   tree->PrintLayerFile("layertest.txt",stime);
   tree->PrintGridFile("grridtest.txt",stime);
   
   //synth
   StereoBuffer16* mix = new StereoBuffer16();
   mix->Create((duration+4)*44100);
   
   int numLayers = tree->GetMaxLayerNum();

    
    std::vector<std::thread> threads;
    std::vector<TreeSynthesizer*> synths;
    
   for(int i=0;i<numLayers+1;i++)
   {
       
      threads.push_back(std::thread([&, i](){
          char layerfilename[20];
          char paramfilename[20];
          strcpy(layerfilename,"layerXX.wav");
          strcpy(paramfilename,"layerXX.wav");
          ParameterList *copyParams = params.Clone();
          copyParams->ResetCoefficients();
          
          /*for noise band*/
          /*
           copyParams.SetParameter(eCenterFrequency,startFreq,startFreq/spacing,startFreq*spacing);
           startFreq=startFreq*(coinflip()?1.25:(1.25/1.0592));
           */
          MonoBuffer16* buf;
          TreeSynthesizer* synth;
          synth = new MultiSynthesizer(duration);
          synths.push_back(synth);
          /*
           switch(i)
           {
           case 0:
           case 13:
           case 5:
           synth = new NoiseBandSynthesizer(duration);
           break;
           case 10:
           case 4:
           case 2:
           synth = new ShakersSynthesizer(duration);
           break;
           case 6:
           case 3:
           case 8:
           synth = new BowedSynthesizer(duration);
           break;
           case 9:
           case 7:
           case 11:
           synth = new VoiceHackSynthesizer(duration);
           break;
           case 12:
           case 1:
           case 14:
           synth = new SaxHackSynthesizer(duration);
           break;
           default:
           synth = new NoiseBandSynthesizer(duration);
           break;
           }
           */
          
          //TreeSynthesizer* synth = new NoiseBandSynthesizer(duration);
          //TreeSynthesizer* synth = new SaxHackSynthesizer(duration);
          //TreeSynthesizer* synth = new VoiceHackSynthesizer(duration);
          //TreeSynthesizer* synth = new BowedSynthesizer(duration);
          //TreeSynthesizer* synth = new BandedWGSynthesizer(duration);
          //TreeSynthesizer* synth = new ShakersSynthesizer(duration);
          
          //    for(int j=1;j<2/*copyParams.GetNumParameters()*/;j++)
          //      {
          //         ParamOutputSynthesizer* paramOutput = new ParamOutputSynthesizer(duration);
          //         paramOutput->SetParameter((Parameter)j,copyParams.GetMinValue((Parameter)j),copyParams.GetMaxValue((Parameter)j));
          //         copyParams.ResetCoefficients();
          //
          //         tree->Synthesize(paramOutput,&copyParams, i);
          //         buf=(MonoBuffer16*)paramOutput->GetBuffer();
          //         buf->Normalize(-3.0);
          //
          //         strcpy(paramfilename,"layerX, paramX.wav");
          //         paramfilename[5]=i+'1';
          //
          //         paramfilename[13]=j+'1';
          //         buf->WriteWave(paramfilename ,0,1.0);
          //
          //         delete paramOutput;
          //      }
          
error          /* Need copy of tree for thread since it writes things!! */
          tree->Synthesize(synth,copyParams, i);
          buf=(MonoBuffer16*)synth->GetBuffer();
          //layer zero and layer numLayer-1 are the most similar so make sure to put them around the back.
          printf("buf %i peak = %f",i,((MonoBuffer16*)synth->GetBuffer())->GetPeak());
          // buf->Normalize(-3.0);
          buf->Normalize(0.0);
          
          layerfilename[5]=(i/10)+'0';
          layerfilename[6]=(i%10)+'0';
          buf->WriteWave(layerfilename,0,1.0);
          delete copyParams;
      }));
       //threads[i].join();
   }
    int i = 0;
    std::for_each(threads.begin(), threads.end(), [&](std::thread &t) {
        t.join();
        mix->Mix((MonoBuffer16*)synths[i]->GetBuffer(),1.0*44100,1.0,((i==0?numLayers-1:(i-1))+0.5)/((float)numLayers));
        delete synths[i];
        i++;
    });
    
   mix->Normalize(0.0);
   mix->WriteWave("TreeMusic.wav",0,1.0);
   
   delete tree;
    
        return 0;
}
