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
	for (unsigned long long i = 1; i < input; i++){	//let's iterate
		
		result *= i;								//iterative calculation of factorial
		ofSleepMillis(30);							//let's pretend this operation takes a looong time...
		
		setPercentDone( (float)(i+1) / input );		//upate this work unit progress in each loop
		if (status == PENDING_CANCELLATION) return;	//check if we are to stop, to stop if early if required
	}
	//our processing is done, we should now gather the result from the main thread...
}


unsigned long long MyWorkUnit::getResult(){
	return result;
}

int MyWorkUnit::getInput(){
	return input;
}