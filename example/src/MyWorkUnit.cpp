/*
 *  MyWorkUnit.cpp
 *  emptyExample
 *
 *  Created by Oriol Ferrer Mesià on 13/01/11.
 *  Copyright 2011 uri.cat. All rights reserved.
 *
 */

#include "MyWorkUnit.h"

MyWorkUnit::MyWorkUnit( int factorialInput ){
	input = factorialInput;	//store the input
}


void MyWorkUnit::process(){ //this will be exectued on a non-maon thread, keep this in mind if using openGL (ofImage, OpenCV, etc...)
							//do your timely operations here, and update "processPercent" with your progress if you want to visualize it on the queue draw method
							//in this example, we'll be calculating the factorial of the input value 

	result = 1;
	int iterations = input;
	for(int i = 0; i < iterations; i++){
		
		result *= i;								//iterative calculation of factorial
		ofSleepMillis(10);							//let's pretend this operation takes a looong time...

		setPercentDone( (float)(i+1) / iterations );		//upate this work unit progress in each loop
		if (isJobPendingCancelation()){				//check if we are to stop, to stop if early if required
			return;
		}
	}
	
	if ( ofRandom(1.0f) < 0.1 ) setStatusFailed();	//if required, you can mark a WorkUnit as failed.
	
	//our processing is done, we should now gather the result from the main thread...
}


unsigned long long MyWorkUnit::getResult(){
	return result;
}

int MyWorkUnit::getInput(){
	return input;
}