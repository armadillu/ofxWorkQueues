/*
 *  DedicatedMultiQueue.h
 *  emptyExample
 *
 *  Created by Oriol Ferrer Mesià on 13/01/11.
 *  Copyright 2011 uri.cat. All rights reserved.
 *
 */

#pragma once

#include "ofMain.h"
#include "DedicatedMultiQueue.h"

class MyWorkUnit : public GenericWorkUnit{

	public:

		MyWorkUnit( float input_ );
		~MyWorkUnit(){};
	
		float input;
		float result;
	
		virtual void process();
};
