/*
 *  DedicatedMultiQueue.cpp
 *  emptyExample
 *
 *  Created by Oriol Ferrer Mesià on 13/01/11.
 *  Copyright 2011 uri.cat. All rights reserved.
 *
 */

#include "DedicatedMultiQueue.h"

DedicatedMultiQueue::DedicatedMultiQueue( int numWorkers_ ){
	if (numWorkers_ < 1) numWorkers_ = 1;
	numWorkers = numWorkers_;
	restTime = 1;
	measureTime = false;
	maxWorkerQueueLen = 10;
	maxPendingQueueLength = 100;
	timeToStop = false;
	verbose = false;
	for (int i = 0; i < numWorkers; i++){
		WorkQueue * wq = new WorkQueue();
		wq->setQueueName(WORKER_NAMES);
		//wq->setVerbose(true);
		workers.push_back( wq );
		
	}
}

DedicatedMultiQueue::~DedicatedMultiQueue(){
	
	timeToStop = true;	//lets flag the thread so that it doesnt try access stuff while we delete things around	
	if(verbose) printf("DedicatedMultiQueue::~DedicatedMultiQueue() waiting for queue thread...\n");
	if (isThreadRunning()){
		waitForThread(false);
	}
	
	for (int i = 0; i < numWorkers; i++){
		delete workers[i];
	}
	
	if(verbose) printf("DedicatedMultiQueue::~DedicatedMultiQueue() deleting pending and processed work units...\n");
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
	if(verbose) printf("~DedicatedMultiQueue done!\n");
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

void DedicatedMultiQueue::join(){

	if (threadRunning)
		waitForThread(false);
	
	for (int i = 0; i < numWorkers; i++){
		if(verbose) printf("asking to join WorkQueue %d...\n", i);
		workers[i]->join();
		if(verbose) printf("done joining WorkQueue %d...\n", i);
	}
}

bool DedicatedMultiQueue::addWorkUnit(GenericWorkUnit* job){

	int ql = getPendingQueueLength();
	
	if (ql < maxPendingQueueLength ){
		lock();
			pending.push_back(job);
		unlock();

		if (verbose) printf("DedicatedMultiQueue::addWorkUnit() ID = %d\n", job->getID());		
		if ( !isThreadRunning() ){	//if the queue is not running, lets start it
			startThread(true, false);
		}
		return true;
	}else{
		if (verbose) printf("DedicatedMultiQueue::addWorkUnit() rejecting job, pending queue is already too long!\n");	
		return false;
	}
}


void DedicatedMultiQueue::draw( int tileW, bool drawIDs , int maxRows, int colDistance){
	
	glColor3ub(255,0,0);

	lock();
	
		int pendingN = pending.size();
		int processedN = processed.size();
				
		glPushMatrix();
			
			int xOff = 0;
			int w = tileW;
			int h = WORK_UNIT_DRAW_H;
			int gap = TILE_DRAW_SPACING;
			
			//pending
			glColor3ub(255,255,255);			
			ofDrawBitmapString( "  pending", 0, h + h * 0.6);
			for (int i = 0; i< pendingN; i++){
				pending[i]->draw(TEXT_DRAW_WIDTH + gap + w * i, h, tileW, drawIDs);
			}

			
			int c = 0;
			for (int i = 0; i < numWorkers; i++){
				int yy = i %maxRows;
				glPushMatrix();
					glTranslatef(c * colDistance,h * yy,0);
					workers[i]->draw(w, drawIDs, i);				
				glPopMatrix();
				if ( (1+i)%maxRows == 0){
					c++;
				}
			}
			

			//processed
			glColor3ub(255,255,255);
			ofDrawBitmapString( "processed", 0, ( c==0 ? numWorkers%maxRows + 2 : maxRows + 2 ) * h + h * 0.6);
			if (processedN > 100) processedN = 100; //CAPPING TO 100
			for (int i = 0; i< processedN; i++){	
				processed[i]->draw(TEXT_DRAW_WIDTH + gap + w * i, ( c==0 ? numWorkers%maxRows + 2 : maxRows + 2 ) * h, tileW, drawIDs);
			}
	
		glPopMatrix();
	unlock();
	
}


void DedicatedMultiQueue::threadedFunction(){

	int nPending = getPendingQueueLength();
	if(verbose) printf("DedicatedMultiQueue::threadedFunction() start processing\n");
	setName("DedicatedMultiQueue Manager");
	int stillWorking = 0;
	
	while( ( nPending > 0 || stillWorking > 0 ) && !timeToStop ){

		if (nPending > 0 ){
			int shortestQueue = shortestWorkQueue();
			
			if ( workers[shortestQueue]->getPendingQueueLength() < maxWorkerQueueLen ){
				lock();
					GenericWorkUnit * w = pending[0];
					pending.erase( pending.begin() );
				unlock();
				
				workers[shortestQueue]->addWorkUnit( w );
			}
		}

		updateQueues();	
		lock();
			nPending = pending.size();
		unlock();
		
		stillWorking = 0;
		for (int i = 0 ; i < numWorkers; i++) {
			if ( workers[i]->getPendingQueueLength() > 0){
				stillWorking++;
			}
		}
		if (restTime>0)
			ofSleepMillis(restTime);
	}
	
	updateQueues();
	
	if (verbose) printf("DedicatedMultiQueue::threadedFunction() end processing %f (%d stillWorking)\n", ofGetElapsedTimef() , stillWorking);
	
	if (!timeToStop){
		if (verbose) printf("detaching DedicatedMultiQueue thread!\n");
		stopThread(true);		//why? cos this is a 1-off thread, once the task is finished, this thread is to be cleared. 
						//If not detached or joined with, it takes resources... neat, uh?		
	}
}


void DedicatedMultiQueue::setIndividualWorkerQueueMaxLen(int len) { 
	maxWorkerQueueLen = len; 
	for (int i = 0; i < numWorkers; i++){
		workers[i]->setMaxQueueLength(len);
	}
}

int DedicatedMultiQueue::shortestWorkQueue(){
	
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

	lock();
		GenericWorkUnit * w = NULL;
		if ( processed.size() > 0){
			w = processed[0];
			processed.erase( processed.begin() );
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

