/*
 *  WorkQueue.h
 *  emptyExample
 *
 *  Created by Oriol Ferrer Mesià on 05/03/11.
 *  Copyright 2011 uri.cat. All rights reserved.
 *
 */

#pragma once

#include "ofMain.h"
#include "GenericWorkUnit.h"
#include "ofxPThread.h"
#include <vector>
#include <pthread.h>
#include <sched.h>

static int numWorkQueues = 0; 

class WorkQueue : public ofxPThread{	//subclass the WorkQueue object to accomodate your desired work
	
	public:
		
		WorkQueue();	
		~WorkQueue();
	
		void draw( int x, int y, int width = 24, bool drawIDs = false, int queueID = -1);
		void update();
	
		bool addWorkUnit( GenericWorkUnit * job, bool highPriority = false);
		GenericWorkUnit* retrieveNextProcessedUnit();

		float getCurrentWorkUnitPercentDone();
		vector<int> getProcessedIDs();
		vector<int> getPendingIDs();
	
		int getAproxPendingQueueLength(){ return pendingN; } //not to be trusted, will give a loose result, but will not lock so should be faster
		int getPendingQueueLength();
		int getProcessedQueueLength();
		void setVerbose(bool v);
		void setMaxQueueLength(int l){ maxQueueLen = l; pending.reserve(maxQueueLen); }
		void setMeasureTimes(bool m){ measureTimes = m; }
		void setQueueName(string name){ queueName = name; }
		void setThreadPriority( float p ); /* 0 - 1 >> 0 being lowest */
		void join();	//experimental!
	
	private: 
	
		vector <GenericWorkUnit*>		pending;
		vector <GenericWorkUnit*>		processed;	
	
		int								ID;
		int								maxQueueLen;
		int								pendingN;

		bool							measureTimes;
		bool							verbose;
		
		//float							startTime;
		float							avgTimePerUnit;
		string							queueName;
	
		GenericWorkUnit*				currentWorkUnit;
	
		float							priority;
		bool							priorityNeedsUpdating;

		int								numJobsExecuted;

		void threadedFunction();
		void applyThreadPriority();
		void updateAverageTimes(float lastTime);
	
};
