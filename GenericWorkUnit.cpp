/*
 *  GenericWorkUnit.h
 *  emptyExample
 *
 *  Created by Oriol Ferrer Mesià on 05/03/11.
 *  Copyright 2011 uri.cat. All rights reserved.
 *
 */


#include "ofMain.h"
#include "GenericWorkUnit.h"

	
GenericWorkUnit::GenericWorkUnit(){
	status = UNPROCESSED;
	ID = numWorkUnits;
	numWorkUnits ++;
	processPercent = 0.0f;
	debug = false;
}				
	

void GenericWorkUnit::cancel(){
	status = PENDING_CANCELLATION;
}


void GenericWorkUnit::draw(int x, int y, int tileW, bool drawIDs){

	glColor3ub(0,200,0);
	setGLColorAccordingToStatus();
	ofRect( x, y, tileW - TILE_DRAW_GAP_H, WORK_UNIT_DRAW_H - TILE_DRAW_GAP_V );
		
	if (status == PROCESSING){
		float hh = 0.2;
		glColor3ub(0,0,0);
		ofRect( x, y + (WORK_UNIT_DRAW_H - TILE_DRAW_GAP_V) * (1-hh) , tileW - TILE_DRAW_GAP_H,  hh * (WORK_UNIT_DRAW_H - TILE_DRAW_GAP_V)  );
		glColor3ub(0,200,0);		
		ofRect( x, y + (WORK_UNIT_DRAW_H - TILE_DRAW_GAP_V) * (1-hh) , (tileW - TILE_DRAW_GAP_H) * processPercent , (WORK_UNIT_DRAW_H - TILE_DRAW_GAP_V) * hh  );
	}
	if (status == FAILED){
		glColor3ub(255,0,0);
		ofLine(x, y, x + tileW- TILE_DRAW_GAP_H, y + WORK_UNIT_DRAW_H - TILE_DRAW_GAP_V);
		ofLine(x + tileW - TILE_DRAW_GAP_H, y, x , y + WORK_UNIT_DRAW_H - TILE_DRAW_GAP_V);
	}
	if (drawIDs){
		glColor3ub(255,255,255);
		ofDrawBitmapString( ofToString( ID ), x + tileW * 0.15,  y +  WORK_UNIT_DRAW_H * 0.66);
	}
}

void GenericWorkUnit::processInThread(){	
	startThread(true, false);
};

void GenericWorkUnit::setGLColorAccordingToStatus(){
	
	switch (status) {
		case UNPROCESSED: glColor3ub(75,75,75); break;
		case PROCESSING: glColor3ub(128,128,128); break;
		case PROCESSED: glColor3ub(0,200,0); break;
		case PENDING_CANCELLATION: glColor3ub(255,255,0); break;
		case CANCELLED: glColor3ub(255,128,0); break;
		case FAILED: glColor3ub(50,50,50); break;
		default: glColor3ub(128,128,128); break;
	}
}

void GenericWorkUnit::preProcess(){
	status = GenericWorkUnit::PROCESSING;
}


void GenericWorkUnit::postProcess(){
	if (status == PENDING_CANCELLATION){
		status = GenericWorkUnit::CANCELLED;
		if(debug) printf("GenericWorkUnit::Aknowledged cancellation for workUnit %d\n", ID);
	}else{
		if (status != FAILED)
			status = GenericWorkUnit::PROCESSED;
	}
}


void GenericWorkUnit::process(){	//subclass this method to do your work

	int numIterations = ofRandom(20, 25);
	printf("GenericWorkUnit::Subclass GenericWorkUnit and implement your own process() method!\n");
	
	for ( int i = 0; i < numIterations; i++) {
		if (status == PENDING_CANCELLATION) return;
		processPercent = (float)i / numIterations;
		ofSleepMillis(10);
	}
}
	
void GenericWorkUnit::threadedFunction(){

	setName("WorkUnit " + ofToString(ID) );
	preProcess();
	process();
	postProcess();

	if (status == PROCESSED || status == FAILED){
		if(debug) printf("GenericWorkUnit::Detaching Generic Work Unit (%d) thread!\n", ID);
		stopThread(true);		//why? cos this is a 1-off thread, once the task is finished, this thread is to be cleared. 
						//If not detached or joined with, it takes resources... neat, uh?
	}else
		if(debug) printf("GenericWorkUnit::This WorkUnit (%d) was cancelled, thus it's not detached (we will join it in destructor)\n", ID);
};
