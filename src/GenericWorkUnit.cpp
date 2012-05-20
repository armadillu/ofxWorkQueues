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
	highPriority = false;
	//printf("new GenericWorkUnit ith ID %d\n", ID);
}				
	
GenericWorkUnit::~GenericWorkUnit(){
	//printf("~GenericWorkUnit();\n");
}

void GenericWorkUnit::cancel(){
	status = PENDING_CANCELLATION;
}


void GenericWorkUnit::draw(int x, int y, int tileW, bool drawIDs){

	setGLColorAccordingToStatus();
	float w = tileW - TILE_DRAW_GAP_H;
	float h = WORK_UNIT_DRAW_H - TILE_DRAW_GAP_V;
	ofRect( x, y, w, h );
		
	if (status == PROCESSING){
		float hh = 0.15;
		ofSetColor(0,0,0);
		ofRect( x, y + (h) * (1-hh) , w,  hh * (h)  );
		ofSetColor(0,200,0);		
		ofRect( x, y + (h) * (1-hh) , (w) * processPercent , (h) * hh  );
	}else
	if (status == FAILED){
		ofSetColor(255,0,0);
		ofLine(x, y, x + tileW- TILE_DRAW_GAP_H, y + h);
		ofLine(x + w, y, x , y + h);
	}
	if (highPriority){
		ofSetColor(200,100,0);
		ofTriangle(x + w - WORK_UNIT_DRAW_H * HIGH_PRIORITY_MARK_SIZE, y, 
				   x + w, y, 
				   x + w, y + WORK_UNIT_DRAW_H * HIGH_PRIORITY_MARK_SIZE );
	}
	if (drawIDs){
		glColor4ub(255,255,255, 255);
		ofDrawBitmapString( ofToString( ID ), x + tileW * 0.15f,  y +  WORK_UNIT_DRAW_H * BITMAP_MSG_HEIGHT);
	}
}

void GenericWorkUnit::processInThread(){	
	startThread(true, false);
};

void GenericWorkUnit::setGLColorAccordingToStatus(){
	
	switch (status) {
		case UNPROCESSED: ofSetColor(64,64,64); break;
		case PROCESSING: ofSetColor(128,128,128); break;
		case PROCESSED: ofSetColor(0,200,0); break;
		case PENDING_CANCELLATION: ofSetColor(255,255,0); break;
		case CANCELLED: ofSetColor(255,128,0); break;
		case FAILED: ofSetColor(50,50,50); break;
		default: ofSetColor(128,128,128); break;
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
