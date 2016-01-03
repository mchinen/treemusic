/*
 *  MetaInterpolation.h
 *  treemusic
 *
    Interpolates some part of an interpolation,
    Note that this meta interpolation works on its ancestors, and can't cause boundry violations.
    Due to this we can only interpolate the mInterpVariable member freely for periodic vars- everything else
    we must check to see if our change violates any of the other nodes in the hierarchy, which may be a tricky thing to do.
 *  Created by Michael Chinen on 2/7/09.
 *  Copyright 2009 Michael Chinen. All rights reserved.
 *
 */

#ifndef __METAINTERPOLATION__
#define __METAINTERPOLATION__
#include "Interpolation.h"
class MetaInterpolation:public Interpolation
{
public:
   MetaInterpolation(Interpolation* target,int bottomLayer, int topLayer,int numTopLayers,TreeNode* owner);
   virtual ~MetaInterpolation(){}
   MetaInterpolation *Clone(Interpolation *target, TreeNode *owner);   
   //randomize looks at the active nodes and picks one to mess with.  
   virtual void Randomize(double startTime, double duration,std::vector<TreeNode*> *activeNodes, std::vector<ParameterList*> *parameters, int layerNum,  int topLayer, int numControlledLayers, int param=-1);

   virtual double ValueAtTime(double time);

   void Apply(double time, std::vector<double>* params);
   void Attach();
   void Detach();
   Interpolation *GetTarget(){return mTarget;}
protected:
   Interpolation* mTarget;
};

#endif
