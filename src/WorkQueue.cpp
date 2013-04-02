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
	verbose = false;
	maxQueueLen = 10;
	currentWorkUnit = NULL;
	measureTimes = false;
	avgTimePerUnit = 0;
	queueName = "q";
	pending.clear();
	processed.clear();
	priority = 0.5;
	priorityNeedsUpdating = false;
	numJobsExecuted = 0;
}


WorkQueue::~WorkQueue(){
	
	if(verbose) printf("WorkQueue::~WorkQueue()\n");

	if (isThreadRunning()){
		if(verbose) printf("WorkQueue::~WorkQueue() waiting for its Queue thread to end...\n");
		lock();
			if (currentWorkUnit != NULL){
				currentWorkUnit->cancel();
			}
		unlock();
		waitForThread();
	}

	if(verbose) printf("WorkQueue::~WorkQueue deleting pending (%d) and processed (%d) work units...\n", (int)pending.size(), (int)processed.size() );

	lock(); //not reall necessary
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
	unlock(); //not reall necessary
	if(verbose) printf("~WorkQueue() done!\n");
}


void WorkQueue::join(){
	if (isThreadRunning()){
		waitForThread();
	}
}

GenericWorkUnit* WorkQueue::retrieveNextProcessedUnit(){
	
	GenericWorkUnit * w = NULL;
	lock();
	int procS = processed.size();
	if ( procS > 0){
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
			cl ++;
			ret = true;
		}else{	
			if (verbose) printf("WorkQueue::addWorkUnit() rejecting job %d, queue is too long!\n", job->getID());	
			ret = false;
		}
	unlock();
	return ret;
}


void WorkQueue::update(){

	lock();
	int numPending = pending.size();
	unlock();

	if (numPending > 0){ //start thread if we have work to do
		if ( !isThreadRunning() ){
			startThread(false /*verbose*/);
		}
	}
}


void WorkQueue::updateAverageTimes(float lastTime){
	avgTimePerUnit = 0.95 * lastTime + 0.05 * avgTimePerUnit;
}

void WorkQueue::setVerbose(bool v){ 
	verbose = v;
}

void WorkQueue::setThreadPriority( float p ){
	//printf("%d %d\n", sched_get_priority_min(SCHED_OTHER), sched_get_priority_max(SCHED_OTHER) );
	priority = ofClamp(p, 0, 1 );
	priorityNeedsUpdating = true;
	//printf("%d\n", priority);
}

void WorkQueue::applyThreadPriority(){
	setPriority(priority);
	priorityNeedsUpdating = false;
}


void WorkQueue::threadedFunction(){

	currentWorkUnit = NULL;	
	
	if(verbose) printf( "WorkQueue::threadedFunction() WQ_%d start processing\n", ID );
	setName( "WorkQueue " + ofToString(ID) ); //thread name

	float timeBefore, timeAfter;

	if (priorityNeedsUpdating){
		applyThreadPriority();
	}

	while ( !isThreadExpectedToStop() ) {
		lock();
			pendingN = pending.size();
			if ( pendingN <= 0 ){ 
				unlock();
				requestThreadToStop();
				if (verbose) printf("WorkQueue::threadedFunction() WQ_%d exiting loop, no pending jobs...\n", ID);
				break;
			}
			if (verbose) printf("WorkQueue::threadedFunction() WQ_%d loop, %d left to process\n", ID,  pendingN);
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
		numJobsExecuted++;

		lock();			
			processed.push_back(currentWorkUnit);
			currentWorkUnit = NULL;
		unlock();
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
		int l = pending.size() + ((currentWorkUnit != NULL) ? 1 : 0);
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


void WorkQueue::draw( int x, int y, int tileW, bool drawIDs, int queueID ){

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
	ofPushMatrix();
	ofTranslate(x, y);

	if (queueID != -1){
		char aux[10];
		sprintf(aux, "%02d", queueID);	//force 2 digits for ID 
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
		int k = j;
		for (j = 0; j< pend ; j++){
			GenericWorkUnit* wu = pending[j];
			wu->draw( gap + TEXT_DRAW_WIDTH + w * (off + j + k), /*2 * h*/0, tileW, drawIDs );
		}
	unlock();
	
	if ( pend == MAX_PENDING_ON_SCREEN ){	//indicate there's more coming...
		glColor4ub(64, 64, 64, 128);
		ofRect( gap + TEXT_DRAW_WIDTH + w * (off + j + k) , 0.0f , tileW - TILE_DRAW_GAP_H , WORK_UNIT_DRAW_H - TILE_DRAW_GAP_V);
		j++;
		glColor4ub(64, 64, 64, 100);
		ofRect( gap + TEXT_DRAW_WIDTH + w * (off + j + k) , 0.0f , tileW - TILE_DRAW_GAP_H , WORK_UNIT_DRAW_H - TILE_DRAW_GAP_V);
		j++;
		glColor4ub(64, 64, 64, 70);
		ofRect( gap + TEXT_DRAW_WIDTH + w * (off + j + k) , 0.0f , tileW - TILE_DRAW_GAP_H , WORK_UNIT_DRAW_H - TILE_DRAW_GAP_V);
		j++;
	}

	if ( k + j + off > 0 && measureTimes ){
		ofSetColor(128,128,128);
		ofDrawBitmapString( time , gap + TEXT_DRAW_WIDTH + 5.0f + w * (off + k + j) + w * 0.15f, /*2 * h + */ h - h * (1.0f- BITMAP_MSG_HEIGHT));
	}
	ofPopMatrix();
}
