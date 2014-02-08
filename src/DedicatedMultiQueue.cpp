/*
 *  DedicatedMultiQueue.cpp
 *  emptyExample
 *
 *  Created by Oriol Ferrer Mesià on 13/01/11.
 *  Copyright 2011 uri.cat. All rights reserved.
 *
 */

#include "DedicatedMultiQueue.h"
#ifdef TARGET_OSX
	#include <pthread.h>
	#include <sched.h>
#endif

DedicatedMultiQueue::DedicatedMultiQueue( int numWorkers_ ){
	if (numWorkers_ < 1) numWorkers_ = 1;
	numWorkers = numWorkers_;
	restTime = 1;
	measureTime = false;
	maxWorkerQueueLen = 10;
	maxPendingQueueLength = 100;
	verbose = false;
	for (int i = 0; i < numWorkers; i++){
		WorkQueue * wq = new WorkQueue();
		wq->setQueueName(WORKER_NAMES);
		//wq->setVerbose(true);
		workers.push_back( wq );
	}
	weAreBeingDeleted = false;
}


DedicatedMultiQueue::~DedicatedMultiQueue(){

	weAreBeingDeleted = true;
	if(verbose) printf("DedicatedMultiQueue::~DedicatedMultiQueue() waiting for queue thread...\n");
	if (isThreadRunning()){
		waitForThread();
	}
	
	for (int i = 0; i < numWorkers; i++){
		delete workers[i];
	}
	
	if(verbose) printf("DedicatedMultiQueue::~DedicatedMultiQueue() deleting pending and processed work units...\n");
	lock(); //not really necessary
	while ( pending.size() > 0 ){		
		GenericWorkUnit * w = pending[0];
		pending.erase( pending.begin() );
		delete w;		
	}
	while ( processed.size() > 0 ){		
		GenericWorkUnit * w = processed[0];
		processed.erase( processed.begin() );
		delete w;
	}
	unlock(); //not really necessary
	if(verbose) printf("~DedicatedMultiQueue done!\n");
}


bool DedicatedMultiQueue::addWorkUnit(GenericWorkUnit* job, bool highPriority){

	lock();
	bool ret = false;
	int ql = pending.size();

	if ( ql < maxPendingQueueLength || ( ql < maxPendingQueueLength * 1.5  && highPriority ) ){
		if (!highPriority){
			pending.push_back(job);
		}else{
			job->highPriority = true;
			pending.insert( pending.begin(), job);
		}
		if (verbose) printf("DedicatedMultiQueue::addWorkUnit() ID = %d\n", job->getID());
		ret = true;

	}else{
		if (verbose) printf("DedicatedMultiQueue::addWorkUnit() rejecting job %d, pending queue is already too long!\n", job->getID());
	}
	unlock();

	return ret;
}


void DedicatedMultiQueue::setNumWorkers(int num){
	
	if (num > 0){
		if (num > numWorkers){
			for (int i = numWorkers; i < num; i++){
				WorkQueue * wq = new WorkQueue();
				wq->setQueueName(WORKER_NAMES);
				workers.push_back( wq );
			}
		}
		numWorkers = num;
		setMeasureTimes(measureTime);	//extend our time measure setting to our new monkeys					
	}
}


void DedicatedMultiQueue::setMeasureTimes(bool m){
	measureTime = m;
	for (int i = 0; i < numWorkers; i++){
		workers[i]->setMeasureTimes(measureTime);
	}
}


void DedicatedMultiQueue::update(){

	lock();
	int numPending = pending.size();

	for (int i = 0 ; i < numWorkers; i++) {
		workers[i]->update();
	}

	if (numPending > 0){ //start thread if we have work to do
		if ( !isThreadRunning() ){
			startThread(false /*verbose*/);
		}
	}
	unlock();
}


void DedicatedMultiQueue::threadedFunction(){


	if(verbose) printf("DedicatedMultiQueue::threadedFunction() start processing\n");
	#ifdef TARGET_OSX
	pthread_setname_np("DedicatedMultiQueue Manager");
	#endif

	int stillWorking = 0;
	
	while( isThreadRunning() ){

		lock();
		int nPending = pending.size();	// how many pending jobs
		stillWorking = 0;				//how many workers are working
		for (int i = 0 ; i < numWorkers; i++) {
			if ( workers[i]->getPendingQueueLength() > 0){
				stillWorking++;
			}
		}

		if( nPending == 0 && stillWorking == 0){ //if no jobs left, and no workers currently working, stop the loop
			unlock();
			stopThread();
			break;
		}


		for (int i = 0 ; i < numWorkers; i++){

			nPending = pending.size();

			if (nPending > 0 ){

				int shortestQueue = shortestWorkQueue();
				int ql = workers[shortestQueue]->getPendingQueueLength();

				if ( ql < maxWorkerQueueLen ){
					GenericWorkUnit * w = pending[0];
					pending.erase( pending.begin() );
					workers[shortestQueue]->addWorkUnit( w, w->highPriority );
				}
			}
		}
		unlock();

		updateQueues();

		if (restTime > 0){
			ofSleepMillis(restTime);
		}
	}
	
	updateQueues();

	#ifdef TARGET_OSX
	if(!weAreBeingDeleted){
		pthread_detach(pthread_self()); //fixing that nasty zombie ofThread bug here
	}
	#endif
}


void DedicatedMultiQueue::setIndividualWorkerQueueMaxLen(int len) { 
	maxWorkerQueueLen = len; 
	for (int i = 0; i < numWorkers; i++){
		workers[i]->setMaxQueueLength(len);
	}
}

int DedicatedMultiQueue::shortestWorkQueue(){ // this needs to be made more efficient
	
	int shortest = INT_MAX;
	int index = 0;
	for (int i = 0; i < numWorkers; i++){
		int l = workers[i]->getPendingQueueLength();
		if ( l < shortest ){
			shortest = l;
			index = i;
		}
	}
	return index;
}


void DedicatedMultiQueue::updateQueues(){

	for (int i = 0; i < numWorkers; i++){
		GenericWorkUnit * w = workers[i]->retrieveNextProcessedUnit();
		if (w != NULL){
			lock();
			processed.push_back( w );
			unlock();
		}
	}
}


GenericWorkUnit* DedicatedMultiQueue::retrieveNextProcessedUnit(){

	GenericWorkUnit * w = NULL;
	lock();
		if ( processed.size() > 0){
			w = processed[0];
			processed.erase( processed.begin() );
			if(verbose) printf("DedicatedMultiQueue::retrieveNextProcessedUnit() %d\n", w->getID());
		}
	unlock();
	return w;
}


int DedicatedMultiQueue::getPendingQueueLength(){
	lock();
	int n = pending.size();
	unlock();
	return n;
}


int DedicatedMultiQueue::getProcessedQueueLength(){
	lock();
	int n = processed.size();
	unlock();
	return n;
}

int DedicatedMultiQueue::getTotalWaitingJobs(){
	lock();
	int n = 0;
	for(int i = 0; i < workers.size(); i++){
		n += workers[i]->getPendingQueueLength();
	}
	unlock();
	return n;
}





void DedicatedMultiQueue::draw( int x, int y, int tileW, bool drawIDs , int maxRows, int colDistance){

	ofSetColor(255,0,0);

	lock();

	int pendingN = pending.size();
	int processedN = processed.size();

	glPushMatrix();
	ofTranslate(x, y);

	int w = tileW;
	int gap = TILE_DRAW_SPACING;
	int h = WORK_UNIT_DRAW_H ;

	//pending
	ofSetColor(255,255,255);
	ofDrawBitmapString( "  pending", 0, h * BITMAP_MSG_HEIGHT);
	int realPending = pendingN;
	if (pendingN > MAX_PENDING_ON_SCREEN ) pendingN = MAX_PENDING_ON_SCREEN;	//crop to max...
	int i;
	for ( i = 0; i< pendingN; i++){
		GenericWorkUnit * gwu = pending[i];
		gwu->draw(TEXT_DRAW_WIDTH + gap + w * i, 0, tileW, drawIDs);
	}

	if (pendingN == MAX_PENDING_ON_SCREEN){	//indicate there's more coming...
		glColor4f(0.5, 0.5, 0.5, 0.33);
		ofRect( TEXT_DRAW_WIDTH + gap + w * i , 0 , tileW - TILE_DRAW_GAP_H , WORK_UNIT_DRAW_H - TILE_DRAW_GAP_V);
		i++;
		glColor4f(0.5, 0.5, 0.5, 0.22);
		ofRect( TEXT_DRAW_WIDTH + gap + w * i , 0 , tileW - TILE_DRAW_GAP_H , WORK_UNIT_DRAW_H - TILE_DRAW_GAP_V);
		i++;
		glColor4f(0.5, 0.5, 0.5, 0.11);
		ofRect( TEXT_DRAW_WIDTH + gap + w * i , 0 , tileW - TILE_DRAW_GAP_H , WORK_UNIT_DRAW_H - TILE_DRAW_GAP_V);
		i++;
		ofSetColor(128,128,128);
		ofDrawBitmapString( "(" + ofToString(realPending) + ")", TEXT_DRAW_WIDTH + gap + w * i, h * BITMAP_MSG_HEIGHT);
	}


	int c = 0;
	for (i = 0; i < numWorkers; i++){
		int yy = i %maxRows;
		workers[i]->draw(c * colDistance,h + (h) * yy, w, drawIDs, i);
		if ( (1+i)%maxRows == 0){
			c++;
		}
	}


	//processed
	ofSetColor(255,255,255);
	ofDrawBitmapString( "processed", 0, ( c==0 ? numWorkers%maxRows : maxRows  ) * h + h + h * BITMAP_MSG_HEIGHT);
	int realProcessedN = processedN;
	if (processedN > MAX_PENDING_ON_SCREEN) processedN = MAX_PENDING_ON_SCREEN; //CAPPING TO 100
	for (i = 0; i< processedN; i++){
		GenericWorkUnit * gwu = processed[i];
		gwu->draw(TEXT_DRAW_WIDTH + gap + w * i, ( c==0 ? numWorkers%maxRows  : maxRows  ) * h + h, tileW, drawIDs);
	}

	if (processedN == MAX_PENDING_ON_SCREEN){	//indicate there's more coming...
		glColor4ub(0, 200, 0, 128);
		ofRect( TEXT_DRAW_WIDTH + gap + w * i , ( c==0 ? numWorkers%maxRows : maxRows  ) * h + h , tileW - TILE_DRAW_GAP_H , WORK_UNIT_DRAW_H - TILE_DRAW_GAP_V);
		i++;
		glColor4ub(0, 200, 0, 60);
		ofRect( TEXT_DRAW_WIDTH + gap + w * i , ( c==0 ? numWorkers%maxRows : maxRows  ) * h + h , tileW - TILE_DRAW_GAP_H , WORK_UNIT_DRAW_H - TILE_DRAW_GAP_V);
		i++;
		glColor4ub(0, 200, 0, 20);
		ofRect( TEXT_DRAW_WIDTH + gap + w * i , ( c==0 ? numWorkers%maxRows : maxRows  ) * h + h , tileW - TILE_DRAW_GAP_H , WORK_UNIT_DRAW_H - TILE_DRAW_GAP_V);
		ofSetColor(128,128,128);
		i++;
		ofDrawBitmapString( "("+ofToString(realProcessedN)+")", TEXT_DRAW_WIDTH + gap + w * i, ( c==0 ? numWorkers%maxRows : maxRows  ) * h + h + h * BITMAP_MSG_HEIGHT);
	}

	glPopMatrix();
	unlock();

}

