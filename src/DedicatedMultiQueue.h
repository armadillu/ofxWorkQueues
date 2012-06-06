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
#include "ofAdvancedThread.h"
#include <queue>
#include <vector>

#define WORKER_NAMES "      "

class DedicatedMultiQueue : public ofAdvancedThread{

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
		void setMaxPendingQueueLength(int l){ maxPendingQueueLength = l; }
		void setMeasureTimes(bool m);
		void setNumWorkers(int num);	//use with caution, wont drop queues if u try decreasing! TODO!!
		void setThreadPriority( int p );		// this will set the p of all the queues and the dispatcher thread
	
		void join();	//experimental!

		int getPendingQueueLength();
		int getProcessedQueueLength();
		
		
	private: 

		vector <WorkQueue*>				workers;

		vector <GenericWorkUnit*>		pending;
		vector <GenericWorkUnit*>		processed;
	
		int								numWorkers;
		int								restTime;
		int								maxWorkerQueueLen;
		int								maxPendingQueueLength;
	
		bool							timeToStop;
		bool							verbose;
		bool							measureTime;
	
		int								priority;

		void updateQueues();
		int shortestWorkQueue();
		void threadedFunction();

};
