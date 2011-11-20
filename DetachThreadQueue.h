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
#include "ofAdvancedThread.h"
#include <queue>
#include <vector>
#include "GenericWorkUnit.h"

class DetachThreadQueue : public ofAdvancedThread{


	public:

		DetachThreadQueue();
		~DetachThreadQueue();
	
		bool addWorkUnit(GenericWorkUnit* job);
		GenericWorkUnit* retrieveNextProcessedUnit();
		void updateQueues();
		void draw( int tileW = 24, bool drawIDs = false );
	
		void setMaxJobsAtATime(int numJobs){ maxProcessing = numJobs; }
		void setRestTimeMillis(int millis){ restTime = millis; }
		void setVerbose(bool v){ verbose = v; }
		void setMaxPendingQueueLength(int l){ maxPendingQueueLength = l; }
	
		int getPendingQueueLength();
		int getProcessingQueueLength();
		int getProcessedQueueLength();

		void threadedFunction();

		vector <GenericWorkUnit*>	pending;	
		vector <GenericWorkUnit*>	processing;
		vector <GenericWorkUnit*>	processed;
		
		int							maxProcessing;
		int							restTime;
		int							maxPendingQueueLength;
	
		bool						timeToStop;
		bool						verbose;

};
