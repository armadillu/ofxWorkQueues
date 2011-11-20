/*
 *  MyWorkUnit.cpp
 *  emptyExample
 *
 *  Created by Oriol Ferrer Mesià on 13/01/11.
 *  Copyright 2011 uri.cat. All rights reserved.
 *
 */

#include "MyWorkUnit.h"

MyWorkUnit::MyWorkUnit( float input_ ){
	input = input_;	//store the input
}


void MyWorkUnit::process(){ //do your timely operations here, and update "processPercent" with your progress if you want to visualize it
							//in this example, we'll be calculating the factorial of the input value 

	result = 1;
	int iterations = input * ofRandom(0.9, 1.1);	//add some unevenness to the thread work length
	
	for (int i = 0; i < iterations; i++){
		if (status == PENDING_CANCELLATION) return;	//really important to check this often during the process, to stop if early if required
		result = iterations * result;
		ofSleepMillis(1);
		setPercentDone( (float)i / iterations );		//upate the progress in each loop
	}
	//or processing is over, we can gather the result from the main thread...
}
