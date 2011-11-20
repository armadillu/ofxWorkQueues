/*
 *  WorkQueue.cpp
 *  emptyExample
 *
 *  Created by Oriol Ferrer Mesià on 05/03/11.
 *  Copyright 2011 uri.cat. All rights reserved.
 *
 */

#include "WorkQueue.h"

WorkQueue::WorkQueue(){
	ID = numWorkQueues;
	numWorkQueues ++;
	timeToStop = false;
	verbose = false;
	maxQueueLen = 50;
	currentWorkUnit = NULL;
	askedToJoin = false;
	measureTimes = false;
	avgTimePerUnit = 0;	
	queueName = "q";
}

WorkQueue::~WorkQueue(){
	
	timeToStop = true;
	if(verbose) printf("WorkQueue::~WorkQueue()\n");
	//ofxThread::setVerbose(true);	//thread printing
	if (isThreadRunning()){
		if(verbose) printf("WorkQueue::~WorkQueue() waiting for its Queue thread to end...\n");
		if (currentWorkUnit != NULL)
			currentWorkUnit->cancel();
		
		waitForThread(false) ;
	}

	if(verbose) printf("WorkQueue::~WorkQueue deleting pending (%d) and processed (%d) work units...\n", (int)pending.size(), (int)processed.size() );

	while ( pending.size() > 0 ){		
		GenericWorkUnit * w = pending[0];
		if(verbose) printf("WorkQueue::~WorkQueue delete pending work unit %d\n", w->getID());
		pending.erase( pending.begin() );
		delete w;		
	}
	
	while ( processed.size() > 0 ){		
		GenericWorkUnit * w = processed[0];
		if(verbose) printf("WorkQueue::~WorkQueue delete processed work unit %d\n", w->getID());
		processed.erase( processed.begin() );
		delete w;
	}
	if(verbose) printf("~WorkQueue() done!\n");
}


void WorkQueue::join(){
	if (threadRunning){
		askedToJoin = true;
		waitForThread(false);
		askedToJoin = false;
	}
}


bool WorkQueue::addWorkUnit( GenericWorkUnit * job){

	int cl = getPendingQueueLength();

	if ( cl < maxQueueLen ){
		if (verbose) printf("WorkQueue::addWorkUnit() ID = %d\n", job->getID() );			
		lock();
			pending.push_back(job);
		unlock();
		
		if ( !isThreadRunning() ){	//if the queue is not running, lets start it
			startThread(true, false);
		}
		return true;
	}else{	
		if (verbose) printf("WorkQueue::addWorkUnit() rejecting job, queue is too long!\n");	
		return false;
	}
}


GenericWorkUnit* WorkQueue::retrieveNextProcessedUnit(){

	lock();
		GenericWorkUnit * w = NULL;
		if ( processed.size() > 0){
			w = processed[0];
			processed.erase( processed.begin() );
		}
	unlock();
	return w;
}

void WorkQueue::updateAverageTimes(float lastTime){
	avgTimePerUnit = 0.4 * lastTime + 0.6 * avgTimePerUnit;
}

void WorkQueue::threadedFunction(){

	currentWorkUnit = NULL;	
	if(verbose) printf( "WorkQueue::threadedFunction() start processing\n" );
	setName( "WorkQueue " + ofToString(ID) );
	startTime = ofGetElapsedTimef();
	
	int pendingN = 1; //we lie here, but its ok, less locking. Should be getPendingQueueLength();

	float timeBefore, timeAfter;

	while ( pendingN > 0 && !timeToStop ) {
		
		lock();
			currentWorkUnit = pending[0];
		unlock();

		currentWorkUnit->preProcess();
		if(measureTimes) timeBefore = ofGetElapsedTimef();
		currentWorkUnit->process();	//run the job in the current thread
		if(measureTimes) timeAfter = ofGetElapsedTimef();		
		currentWorkUnit->postProcess();
		if(measureTimes) updateAverageTimes(timeAfter - timeBefore);

		lock();
			pending.erase( pending.begin() );
			processed.push_back(currentWorkUnit);
			currentWorkUnit = NULL;	
			pendingN = pending.size();
		unlock();
		if(verbose) printf("WorkQueue::threadedFunction() looping, %d left to process\n", pendingN);
	}
		
	if (verbose) printf("WorkQueue::threadedFunction() ended processing %f\n", ofGetElapsedTimef() - startTime );

	if (askedToJoin){
		askedToJoin = false;
		if (verbose) printf("WorkQueue::Asked to join WorkQueue, ending threadedMethod!\n");
		return;
	}
	if (!timeToStop){
		if (verbose) printf("WorkQueue::Detaching WorkQueue thread!\n");
		stopThread(true);		//why? cos this is a 1-off thread, once the task is finished, this thread is to be cleared. 
						//If not detached or joined with, it takes resources... neat, uh?
	}
};


vector<int> WorkQueue::getProcessedIDs(){
	vector<int> v;
	lock();
	int n = processed.size();
	for ( int i = 0; i < n; i++ ){
		v.push_back( processed[i]->getID() );
	}			
	unlock();
	return v;
}

vector<int> WorkQueue::getPendingIDs(){
	vector<int> v;
	lock();
	int n = pending.size();
	for ( int i = 0; i < n; i++ ){
		v.push_back( pending[i]->getID() );
	}			
	unlock();
	return v;
}

float WorkQueue::getCurrentWorkUnitPercentDone(){

	float d = 0.0f;
	if (currentWorkUnit != NULL){
		d = currentWorkUnit->getPercentDone();
	}
	return d;
}

int WorkQueue::getPendingQueueLength(){
	lock();
		int l = pending.size();
	unlock();
	return l;
}

int WorkQueue::getProcessedQueueLength(){
	lock();
		int l = processed.size();
	unlock();
	return l;
}

void WorkQueue::draw( int tileW, bool drawIDs, int queueID ){

	int xOff = 0;
	int w = tileW;
	int h = WORK_UNIT_DRAW_H;
	int gap = 8;
	int tOffx = 0;
	int nl = queueName.size();
	int unitGap = 1;
	
	glColor3ub(255,255,255);
	string time;

	if (measureTimes){
		time = ofToString(1000 * avgTimePerUnit,1) + "ms";
	}

	if (queueID != -1){
		char aux[10];
		int l = sprintf(aux, "%d", queueID);
		sprintf(aux, "%02d", queueID);	//force 2 digits for ID 
		xOff = ( TEXT_DRAW_WIDTH ) ;
		ofDrawBitmapString( queueName + " " +  aux , tOffx,  2 * h + h * 0.6);
	}else{
		xOff = ( TEXT_DRAW_WIDTH  ) ;
		ofDrawBitmapString( queueName, tOffx,  2 * h + h * 0.6);
	}


	//lock();
	int j;
	for (j = 0; j< processed.size(); j++){
		processed[j]->draw( gap + xOff + w * j, 2 * h, tileW, drawIDs);
	}


	for (j = 0; j< pending.size(); j++){
		pending[j]->draw(gap + xOff + w * j, 2 * h, tileW, drawIDs);		
	}
	//unlock();
	
	if (j > 0 && measureTimes){
		glColor3ub(128,128,128);
		ofDrawBitmapString( time , gap + xOff + 5 + w * j + w * 0.15, 2 * h + h - h * 0.33);
	}
}
