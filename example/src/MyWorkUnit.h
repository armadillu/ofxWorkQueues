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

		MyWorkUnit( int factorialInput );
		~MyWorkUnit(){};

		unsigned long long getResult();
		int getInput();
	
	private:
	
		int input;
		unsigned long long result;
	
		virtual void process();	//our subclass GenericWorkUnit subclassed method, that will run on a second thread
};
