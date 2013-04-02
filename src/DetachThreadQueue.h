/*
 *  DetachThreadQueue.h
 *  emptyExample
 *
 *  Created by Oriol Ferrer Mesià on 13/01/11.
 *  Copyright 2011 uri.cat. All rights reserved.
 *
 */

#pragma once

#include "ofMain.h"
#include "ofxPThread.h"
#include <queue>
#include <vector>
#include "GenericWorkUnit.h"

class DetachThreadQueue : public ofxPThread{


	public:

		DetachThreadQueue();
		~DetachThreadQueue();
		
		void draw( int x, int y, int tileW = 24, bool drawIDs = false );
		void update();
		bool addWorkUnit(GenericWorkUnit* job, bool highPriority = false);
		GenericWorkUnit* retrieveNextProcessedUnit();
	
		void setMaxJobsAtATime(int numJobs){ maxProcessing = numJobs; }
		void setRestTimeMillis(int millis){ restTime = millis; }
		void setVerbose(bool v){ verbose = v; }
		void setMaxPendingQueueLength(int l){ maxPendingQueueLength = l; }
		
		int getPendingQueueLength();
		int getProcessingQueueLength();
		int getProcessedQueueLength();
	
		void setPriority( int p ); // this will set the p of all the queues and the dispatcher thread

	private:

		void updateQueues();
	
		void threadedFunction();

		vector <GenericWorkUnit*>	pending;	
		vector <GenericWorkUnit*>	processing;
		vector <GenericWorkUnit*>	processed;
		
		int							maxProcessing;
		int							restTime;
		int							maxPendingQueueLength;
	
		int							priority;
	
		bool						timeToStop;
		bool						verbose;
};
