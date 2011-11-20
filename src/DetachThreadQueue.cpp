/*
 *  DetachThreadQueue.cpp
 *  emptyExample
 *
 *  Created by Oriol Ferrer Mesià on 13/01/11.
 *  Copyright 2011 uri.cat. All rights reserved.
 *
 */

#include "DetachThreadQueue.h"

DetachThreadQueue::DetachThreadQueue(){
	maxProcessing = 4;
	restTime = 1;
	timeToStop = false;
	verbose = false;
	maxPendingQueueLength = 100;
}



DetachThreadQueue::~DetachThreadQueue(){
	
	timeToStop = true;	//lets flag the thread so that it doesnt try access stuff while we delete things around	
	if(verbose) printf("DetachThreadQueue::~DetachThreadQueue() waiting for queue thread...\n");
	if (isThreadRunning())
		waitForThread(false);
	
	if(verbose) printf("DetachThreadQueue::~DetachThreadQueuedeleting deleting pending, processing and processed work units...\n");
	while ( processing.size() > 0 ){
		GenericWorkUnit * w = processing[0];
		//w->lock();
		if ( w->isThreadRunning() ){
			w->cancel();
			if(verbose) printf("WorkUnit(%d) waitForThread at destructor \n", w->getID());
			w->waitForThread(false);
		}
		//w->unlock();
		processing.erase( processing.begin() );
		delete w;
	}	
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
	if(verbose) printf("~DetachThreadQueue() done!\n");
}



bool DetachThreadQueue::addWorkUnit(GenericWorkUnit* job){

	int ql = getPendingQueueLength();

	if (ql < maxPendingQueueLength ){
		lock();
			pending.push_back(job);
		unlock();

		if (verbose) printf("DetachThreadQueue::addWorkUnit() ID = %d\n", job->getID());	
		if ( !isThreadRunning() ){	//if the queue is not running, lets start it
			startThread(true, false);
		}
		return true;
	}else{
		if (verbose) printf("DetachThreadQueue::addWorkUnit() rejecting job, pending queue is already too long!\n");	
		return false;
	}
}


void DetachThreadQueue::draw( int tileW, bool drawIDs ){
	
	glColor3ub(255,0,0);

	lock();
	
		int pendingN = pending.size();
		int processingN = processing.size();
		int processedN = processed.size();
				
		glPushMatrix();
				
				int xOff = 0;
				int w = tileW;
				int h = WORK_UNIT_DRAW_H;
				int gap = TILE_DRAW_SPACING;

				//pending
				glColor3ub(255,255,255);
				ofDrawBitmapString( "pending", 0, h + h * 0.6);
				for (int i = 0; i< pendingN; i++){
					pending[i]->draw(TEXT_DRAW_WIDTH + gap + w * i, h, tileW, drawIDs);
				}

				//processing
				glColor3ub(255,255,255);
				ofDrawBitmapString( "processing", 0, 2 * h + h * 0.6);
				for (int i = 0; i< processingN; i++){					
					processing[i]->draw( TEXT_DRAW_WIDTH + gap + w * i, 2 * h, tileW, drawIDs);
				}

				//processed
				glColor3ub(255,255,255);
				ofDrawBitmapString( "processed", 0, 3 * h + h * 0.6);
				for (int i = 0; i< processedN; i++){
					processed[i]->draw( TEXT_DRAW_WIDTH + gap + w * i, 3 * h, tileW, drawIDs);
				}

		glPopMatrix();
	
	unlock();
	
}


void DetachThreadQueue::threadedFunction(){

	int nPending = getPendingQueueLength();
	int nProcessing = getProcessingQueueLength();
	if(verbose) printf("DetachThreadQueue::threadedFunction() start processing\n");
	setName("DetachThreadQueue Manager");
	
	while( ( nPending > 0 || nProcessing > 0 ) && !timeToStop ){

		if ( nPending > 0 && nProcessing < maxProcessing ){

			lock();
				GenericWorkUnit * w = pending[0];
				pending.erase( pending.begin() );
			unlock();

			w->processInThread();

			lock();
				processing.push_back(w);
			unlock();
			
		}else{
			//printf(".");
			ofSleepMillis(restTime);
		}

		updateQueues();

		nPending = pending.size();
		nProcessing = processing.size();
	}
	
	if(verbose) printf("DetachThreadQueue::threadedFunction() end processing %f\n", ofGetElapsedTimef() );
	
	if (!timeToStop){
		if (verbose) printf("detaching DetachThreadQueue thread!\n");
		stopThread(true);		//why? cos this is a 1-off thread, once the task is finished, this thread is to be cleared. 
						//If not detached or joined with, it takes resources... neat, uh?
	}
}


void DetachThreadQueue::updateQueues(){
	
	lock();
	int numProcessing = processing.size();
	
	for (int i = numProcessing-1; i >= 0; i--){
		if ( ( processing[i]->getStatus() == GenericWorkUnit::FAILED || processing[i]->getStatus() == GenericWorkUnit::PENDING_CANCELLATION || processing[i]->getStatus() == GenericWorkUnit::PROCESSED ) && processing[i]->isThreadRunning() == false ){
			if ( processing[i]->getStatus() == GenericWorkUnit::PENDING_CANCELLATION ) processing[i]->status == GenericWorkUnit::CANCELLED;
			processed.push_back(processing[i]);
			processing.erase( processing.begin() + i );
		}
	}	
	unlock();
}

GenericWorkUnit* DetachThreadQueue::retrieveNextProcessedUnit(){

	lock();
		GenericWorkUnit * w = NULL;
		if ( processed.size() > 0){
			w = processed[0];
			processed.erase( processed.begin() );
			if(verbose) printf("retrieveNextProcessedUnit>> %d status (%d)\n", w->getID(), w->getStatus());
		}
	unlock();
	return w;
}


int DetachThreadQueue::getPendingQueueLength(){
	lock();
	int n = pending.size();
	unlock();
	return n;
}


int DetachThreadQueue::getProcessedQueueLength(){
	lock();
	int n = processed.size();
	unlock();
	return n;
}


int DetachThreadQueue::getProcessingQueueLength(){
	lock();
	int n = processing.size();
	unlock();
	return n;
}
