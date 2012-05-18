/*
 *  BlurWorkUnit.cpp
 *  emptyExample
 *
 *  Created by Oriol Ferrer Mesià on 13/01/11.
 *  Copyright 2011 uri.cat. All rights reserved.
 *
 */

#include "BlurWorkUnit.h"

BlurWorkUnit::BlurWorkUnit( unsigned char * pixels, int w, int h ){
	
	image.setUseTexture(false);
	image.allocate(w, h);	
	image.setFromPixels(pixels, w, h);
}


BlurWorkUnit::~BlurWorkUnit(){
	
};


void BlurWorkUnit::process(){ //this will be exectued on a non-maon thread, keep this in mind if using openGL (ofImage, OpenCV, etc...)

	int iterations = 5;
	
	for (int i = 0; i < iterations; i++){
		image.blurGaussian(25);
		setPercentDone( (float)i / iterations);
		if (status == PENDING_CANCELLATION) return;	//really important to check this often during the process, to stop if early if required
	}
}

