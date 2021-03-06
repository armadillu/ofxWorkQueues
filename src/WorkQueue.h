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
//#include "ofxPThread.h"
//#include <vector>
//#include <pthread.h>
//#include <sched.h>

static int numWorkQueues = 0; 

class WorkQueue : public ofThread{	//subclass the WorkQueue object to accomodate your desired work
	
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
		void setMaxQueueLength(int l){ maxQueueLen = l; pending.reserve(maxQueueLen); }
		void setMeasureTimes(bool m){ measureTimes = m; }
		void setQueueName(string name){ queueName = name; }
		void setShowTimesInSeconds(bool b){timesInSeconds = b;}

		float getAvgTimePerUnit(){return avgTimePerUnit;}
		void join();	//experimental!
		void setVerbose(bool v);

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

		int								numJobsExecuted;

		bool							weAreBeingDeleted; //to avoid doing the last join on destruction, as then we can't do waitForThread
		bool							timesInSeconds;
		void threadedFunction();
		void updateAverageTimes(float lastTime);


	
};
