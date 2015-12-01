/*
 *  TreeSynthesizer.cpp
 *  treemusic
 *
 *  Created by Michael Chinen on 11/25/08.
 *  Copyright 2008 Michael Chinen. All rights reserved.
 *
 */

#include "MonoBuffer16.h"
#include "TreeSynthesizer.h"

long TreeSynthesizer::GetLastLongSample()
{
   return buffer->GetFrame(currentSample-1);
}