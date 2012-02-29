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
#include "ofxCvHaarFinder.h"
class BlurWorkUnit : public GenericWorkUnit{

	public:

		BlurWorkUnit( unsigned char * pixels, int w, int h );
		~BlurWorkUnit();

		ofxCvColorImage getResult(){ return image; }
	
	private:
	
		ofxCvColorImage image;
	
		virtual void process();	//our subclass GenericWorkUnit subclassed method, that will run on a second thread
};
