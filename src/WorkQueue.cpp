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
	pending.clear();
	processed.clear();
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


GenericWorkUnit* WorkQueue::retrieveNextProcessedUnit(){
	
	GenericWorkUnit * w = NULL;
	lock();
	int procS = processed.size();
	if ( processed.size() > 0){
		w = processed[0];
		if(verbose) printf("WorkQueue::retrieveNextProcessedUnit() %d\n", w->getID());
		processed.erase( processed.begin() );
	}
	unlock();
	return w;
}

bool WorkQueue::addWorkUnit( GenericWorkUnit * job, bool highPriority){

	bool ret = false;
	if (verbose) printf("WorkQueue::addWorkUnit() trying to add ID = %d\n", job->getID() );			
	lock();
		int cl = pending.size();
		
		if ( cl < maxQueueLen || ( cl < maxQueueLen * 1.5  && highPriority ) ){ // if job is high priority, give some margin to the max pending queue len restriction
			if (verbose) printf("WorkQueue::addWorkUnit() added ID = %d\n", job->getID() );
			if (!highPriority){
				pending.push_back(job);
			}else{
				job->highPriority = true;
				pending.insert( pending.begin(), job);
			}
			ret = true;
		}else{	
			if (verbose) printf("WorkQueue::addWorkUnit() rejecting job %d, queue is too long!\n", job->getID());	
			ret = false;
		}
	unlock();
	return ret;
}

void WorkQueue::updateAverageTimes(float lastTime){
	avgTimePerUnit = 0.2 * lastTime + 0.8 * avgTimePerUnit;
}

void WorkQueue::setVerbose(bool v){ 
	verbose = v;
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
			if (pending.size() <= 0){ 
				unlock();
				printf("mmmmmmmmmm.......\n");
				continue;
			}
			currentWorkUnit = pending[0];
			pending.erase( pending.begin() );
		unlock();
		
		//int currID = currentWorkUnit->getID();
		//printf("currentWorkUnit is %d\n", currID);

		currentWorkUnit->preProcess();
		if(measureTimes) timeBefore = ofGetElapsedTimef();
		currentWorkUnit->process();	//run the job in the current thread
		if(measureTimes) timeAfter = ofGetElapsedTimef();		
		currentWorkUnit->postProcess();
		if(measureTimes) updateAverageTimes(timeAfter - timeBefore);

		lock();			
			processed.push_back(currentWorkUnit);
			pendingN = pending.size();
			currentWorkUnit = NULL;	
		unlock();
		
		if(verbose) printf("WorkQueue::threadedFunction() looping, %d left to process\n", pendingN);
	}
	
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
	lock();
	if (currentWorkUnit != NULL){
		d = currentWorkUnit->getPercentDone();
	}
	unlock();
	return d;
}

int WorkQueue::getPendingQueueLength(){
	lock();
		int l = pending.size();
	unlock();
	if (verbose) printf("getPendingQueueLength() >> %d\n",l);
	return l;
}

int WorkQueue::getProcessedQueueLength(){
	lock();
		int l = processed.size();		
	unlock();
	if (verbose) printf("getProcessedQueueLength() >> %d\n",l);
	return l;
}

void WorkQueue::update(){

	if ( !isThreadRunning() ){
		int l = pending.size();
		if ( l > 0 ){ // make sure the thread is started if there's work to do
			startThread(true, false);
		}
	}
}

void WorkQueue::draw( int tileW, bool drawIDs, int queueID ){

	int w = tileW;
	int h = WORK_UNIT_DRAW_H;
	int gap = 8;
	int off = 0;
	int j;		
	string time;

	if (measureTimes){
		time = ofToString(1000.0f * avgTimePerUnit, 1) + "ms";
	}
	
	ofSetColor(255,255,255);
	if (queueID != -1){
		char aux[10];
		int l = sprintf(aux, "%02d", queueID);	//force 2 digits for ID 
		ofDrawBitmapString( queueName + " " +  aux , 0.0f,  /*2 * h  + */ h * BITMAP_MSG_HEIGHT);
	}else{
		ofDrawBitmapString( queueName + " (" + ofToString(pending.size()) + ")", 0.0f,  /* 2 * h + */ h * BITMAP_MSG_HEIGHT);
	}

	lock();	
		if (currentWorkUnit != NULL) off = 1;
		int proc = processed.size();
		
		for (j = 0; j< proc ; j++){
			GenericWorkUnit* wu = processed[j];
			wu->draw( gap + TEXT_DRAW_WIDTH + w * (j), /*2 * h*/0.0f, tileW, drawIDs);
		}

		if (currentWorkUnit != NULL){
			currentWorkUnit->draw(gap + TEXT_DRAW_WIDTH + w * j , /*2 * h*/0.0f, tileW, drawIDs);
		}

		int pend = pending.size();
		if ( pend > MAX_PENDING_ON_SCREEN ) {
			pend = MAX_PENDING_ON_SCREEN;
		}
		
		for (j = 0; j< pend ; j++){
			GenericWorkUnit* wu = pending[j];
			wu->draw( gap + TEXT_DRAW_WIDTH + w * (off + j), /*2 * h*/0, tileW, drawIDs );
		}
	unlock();
	
	if ( pend == MAX_PENDING_ON_SCREEN ){	//indicate there's more coming...
		j++;
		glColor4ub(64, 64, 64, 128);
		ofRect( gap + TEXT_DRAW_WIDTH + w * j , 0.0f , tileW - TILE_DRAW_GAP_H , WORK_UNIT_DRAW_H - TILE_DRAW_GAP_V);
		j++;
		glColor4ub(64, 64, 64, 100);
		ofRect( gap + TEXT_DRAW_WIDTH + w * j , 0.0f , tileW - TILE_DRAW_GAP_H , WORK_UNIT_DRAW_H - TILE_DRAW_GAP_V);
		j++;
		glColor4ub(64, 64, 64, 70);
		ofRect( gap + TEXT_DRAW_WIDTH + w * j , 0.0f , tileW - TILE_DRAW_GAP_H , WORK_UNIT_DRAW_H - TILE_DRAW_GAP_V);
	}

	if ( j > 0 && measureTimes ){
		ofSetColor(128,128,128);
		ofDrawBitmapString( time , gap + TEXT_DRAW_WIDTH + 5.0f + w * (off + j) + w * 0.15f, /*2 * h + */ h - h * (1.0f- BITMAP_MSG_HEIGHT));
	}
}
