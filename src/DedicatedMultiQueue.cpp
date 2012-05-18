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

void DedicatedMultiQueue::update(){
	for(int i = 0; i < workers.size(); i++){
		workers[i]->update();
	}
	if ( !isThreadRunning() ){
		int nPending = pending.size();
		if ( nPending > 0 ){	//if the queue is not running, lets start it
			startThread(true, false);
		}
	}
}

void DedicatedMultiQueue::draw( int tileW, bool drawIDs , int maxRows, int colDistance){
	
	ofSetColor(255,0,0);

	lock();
	
		int pendingN = pending.size();
		int processedN = processed.size();
			
		glPushMatrix();
			
			int xOff = 0;
			int w = tileW;
			int h = WORK_UNIT_DRAW_H;
			int gap = TILE_DRAW_SPACING;
			
			//pending
			ofSetColor(255,255,255);			
			ofDrawBitmapString( "  pending", 0, h * 0.6);
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
				ofDrawBitmapString( "(" + ofToString(realPending) + ")", TEXT_DRAW_WIDTH + gap + w * i, h * 0.6);
			}

			
			int c = 0;
			for (i = 0; i < numWorkers; i++){
				int yy = i %maxRows;
				glPushMatrix();
					glTranslatef(c * colDistance,h + (h) * yy,0);
					workers[i]->draw(w, drawIDs, i);				
				glPopMatrix();
				if ( (1+i)%maxRows == 0){
					c++;
				}
			}
			

			//processed
			ofSetColor(255,255,255);
			ofDrawBitmapString( "processed", 0, ( c==0 ? numWorkers%maxRows : maxRows  ) * h + h + h * 0.6);
			int realProcessedN = processedN;
			if (processedN > MAX_PENDING_ON_SCREEN) processedN = MAX_PENDING_ON_SCREEN; //CAPPING TO 100
			for (i = 0; i< processedN; i++){	
				GenericWorkUnit * gwu = processed[i];
				gwu->draw(TEXT_DRAW_WIDTH + gap + w * i, ( c==0 ? numWorkers%maxRows  : maxRows  ) * h + h, tileW, drawIDs);
			}
	
			if (processedN == MAX_PENDING_ON_SCREEN){	//indicate there's more coming...
				i++;
				glColor4ub(64, 64, 64, 128);
				ofRect( TEXT_DRAW_WIDTH + gap + w * i , ( c==0 ? numWorkers%maxRows  : maxRows  ) * h + h , tileW - TILE_DRAW_GAP_H , WORK_UNIT_DRAW_H - TILE_DRAW_GAP_V);
				i++;
				glColor4ub(64, 64, 64, 100);
				ofRect( TEXT_DRAW_WIDTH + gap + w * i , ( c==0 ? numWorkers%maxRows  : maxRows  ) * h + h , tileW - TILE_DRAW_GAP_H , WORK_UNIT_DRAW_H - TILE_DRAW_GAP_V);
				i++;
				glColor4ub(64, 64, 64, 70);
				ofRect( TEXT_DRAW_WIDTH + gap + w * i , ( c==0 ? numWorkers%maxRows  : maxRows  ) * h + h , tileW - TILE_DRAW_GAP_H , WORK_UNIT_DRAW_H - TILE_DRAW_GAP_V);
				ofSetColor(128,128,128);
				ofDrawBitmapString( "("+ofToString(realProcessedN)+")", TEXT_DRAW_WIDTH + gap + w * i, ( c==0 ? numWorkers%maxRows : maxRows  ) * h + h + h * 0.6);
			}
	
		glPopMatrix();
	unlock();
	
}


void DedicatedMultiQueue::threadedFunction(){

	int nPending = getPendingQueueLength();
	if(verbose) printf("DedicatedMultiQueue::threadedFunction() start processing\n");
	t = ofGetElapsedTimef();
	setName("DedicatedMultiQueue Manager");
	int stillWorking = 0;
	
	while( ( nPending > 0 || stillWorking > 0 ) && !timeToStop ){

		lock();
		for (int i = 0 ; i < numWorkers; i++){
			
			nPending = pending.size();
			if (nPending > 0 ){

				int shortestQueue = shortestWorkQueue();
				int ql = workers[shortestQueue]->getPendingQueueLength();

				if ( ql < maxWorkerQueueLen ){
					GenericWorkUnit * w = pending[0];
					pending.erase( pending.begin() );
					workers[shortestQueue]->addWorkUnit( w );
				}
			}			
		}
		unlock();
		
		updateQueues();	
		
		lock();
		stillWorking = 0;
		for (int i = 0 ; i < numWorkers; i++) {
			if ( workers[i]->getPendingQueueLength() > 0){
				stillWorking++;
			}
		}
		unlock();
		if (restTime>0)
			ofSleepMillis(restTime);
	}
	
	updateQueues();
	
	if (verbose){ 
		t = ofGetElapsedTimef() - t;
		printf("DedicatedMultiQueue::threadedFunction() end processing %f (%d stillWorking)\n", t , stillWorking);
	}
	
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
	
	lock();		
	for (int i = 0; i < numWorkers; i++){
		GenericWorkUnit * w = workers[i]->retrieveNextProcessedUnit();
		if (w != NULL){
			processed.push_back( w );
		}
	}	
	unlock();
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

