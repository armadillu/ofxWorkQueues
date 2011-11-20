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
#include "ofAdvancedThread.h"
#include <vector>

static int numWorkQueues = 0;

class WorkQueue : public ofAdvancedThread{	//subclass this object to accomodate your desired work
	
	public:
		
		WorkQueue();	
		~WorkQueue();
	
		void draw( int width = 24, bool drawIDs = false, int queueID = -1);
	
		bool addWorkUnit( GenericWorkUnit * job);
		GenericWorkUnit* retrieveNextProcessedUnit();

		float getCurrentWorkUnitPercentDone();
		vector<int> getProcessedIDs();
		vector<int> getPendingIDs();
	
		int getPendingQueueLength();
		int getProcessedQueueLength();
		void setVerbose(bool v){ verbose = v;}
		void setMaxQueueLength(int l){ maxQueueLen = l; }
		void setMeasureTimes(bool m){ measureTimes = m; }
		void setQueueName(string name){ queueName = name; }
	
		void join();	//experimental!
	
	private: 
	
		vector <GenericWorkUnit*>		pending;
		vector <GenericWorkUnit*>		processed;	
	
		int								ID;
		int								maxQueueLen;
	
		bool							timeToStop;
		bool							askedToJoin;
		bool							measureTimes;
		bool							verbose;
		
		float							startTime;
		float							avgTimePerUnit;
		string							queueName;
	
		GenericWorkUnit*				currentWorkUnit;

		void threadedFunction();
		void updateAverageTimes(float lastTime);
	
};
