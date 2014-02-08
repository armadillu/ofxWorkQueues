/*
 *  DedicatedMultiQueue.h
 *  emptyExample
 *
 *  Created by Oriol Ferrer Mesià on 13/01/11.
 *  Copyright 2011 uri.cat. All rights reserved.
 *
 */

#pragma once

#include "ofMain.h"
#include "GenericWorkUnit.h"
#include "WorkQueue.h"

#define WORKER_NAMES "      "

class DedicatedMultiQueue : public ofThread{

	public:

		DedicatedMultiQueue( int numWorkers_ = 4);
		~DedicatedMultiQueue();
	
		bool addWorkUnit(GenericWorkUnit* job, bool highPriority = false);
		GenericWorkUnit* retrieveNextProcessedUnit();
		
		void draw( int x, int y, int tileW = 24, bool drawIDs = false, int maxRows = 40, int ColDistance = 300);
		void update();
	
		void setRestTimeMillis(int millis){ restTime = millis; }
		void setVerbose(bool v){ verbose = v; }
		void setIndividualWorkerQueueMaxLen(int len);
		void setMaxPendingQueueLength(int l){ lock(); maxPendingQueueLength = l; unlock(); }
		void setMeasureTimes(bool m);
		void setNumWorkers(int num);		//use with caution, wont drop queues if u try decreasing! TODO!!

		int getPendingQueueLength();
		int getProcessedQueueLength();
		int getTotalWaitingJobs();
		
	private: 

		vector <WorkQueue*>				workers;

		vector <GenericWorkUnit*>		pending;
		vector <GenericWorkUnit*>		processed;
	
		int								numWorkers;
		int								restTime;
		int								maxWorkerQueueLen;
		int								maxPendingQueueLength;
	
		bool							verbose;
		bool							measureTime;

		bool							weAreBeingDeleted;
		void updateQueues();
		int shortestWorkQueue();
		void threadedFunction();
};
