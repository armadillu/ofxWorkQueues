//
//  ofxPThread.h
//
//  Created by Oriol Ferrer Mesià on 01/04/13.
//	mostly ripped from the old OF v 0070 ofThread, with quite some modifications to make it fit the pthread life cycle better
//
//

#ifndef ofxWorkQueues_ofxPThread_h
#define ofxWorkQueues_ofxPThread_h

#include "ofMain.h"
#include <pthread.h>
#include <sched.h>

class ofxPThread{

	public:

	ofxPThread(){
		threadRunning = false;
		stopThreadRequested = false;
		cleanedUp = false;
		verbose = false;
		someThreadRequestedToJoin = false;
	};

	//to be called from main thread
	~ofxPThread(){
		if (isThreadRunning()){
			waitForThread();
		}
	}


	//to be called from main thread
	void startThread(bool _verbose = true){
		if( threadRunning ){
			if(verbose)printf("ofThread: thread already running\n");
			return;
		}

		threadRunning = true;
		// create PTHREAD_CREATE_DETACHED thread, so that can be joined later if required
		pthread_create(&myThread, NULL, thread, (void *)this);
		verbose = _verbose;
	};


	//to be called from either thread
	void setThreadToCleanUpOnExit(){
		if (!cleanedUp){
			cleanedUp = true;
			if(verbose)printf("ofThread: setting thread to clean up after ending (pthread_detach)\n");
			pthread_detach(myThread); //tell system that thread's resources are to be released upon its termination
		}else{
			if(verbose)printf("ofThread: cant setThreadToCleanUpOnExit(), already set to do so!\n");
		}
	}

	//to be called from either thread
	void requestThreadToStop(){
		if(verbose)printf("ofThread: requestThreadToStop()\n");
		if(threadRunning){
			if (stopThreadRequested) if(verbose)printf("ofThread: thread has already been requested to stop!\n");
			stopThreadRequested = true; //raise a flag for the thread to stop when possible
		}else{
			if(verbose)printf("ofThread: thread already stopped\n");
		}
	}


	//to be called from main thread
	void waitForThread(){

		if (cleanedUp){
			if(verbose)printf("ofThread: waitForThread() not possible! thread has already been cleanedUp!\n");
		}else{
			if (threadRunning){
				// Reset the thread state
				requestThreadToStop();
				if(pthread_self()==myThread) printf("ofThread: error, waitForThread should only be called from outside the thread");
				if(verbose)printf("ofThread: waiting for thread to stop\n");
				someThreadRequestedToJoin = true;
				pthread_join(myThread, NULL);
				threadRunning = false;

				if(verbose)printf("ofThread: waitForThread joined!\n");
			}else{
				if(verbose)printf("ofThread: cant waitForThread()! thread already stopped\n");
			}
		}
	};


	//to be called from either thread
	bool isThreadRunning(){
		return threadRunning;
	}

	//to be called from either thread
	bool isThreadExpectedToStop(){
		return stopThreadRequested;
	}

	//to be called from either thread
	bool hasSomeThreadRequestedToJoin(){
		return someThreadRequestedToJoin;
	}


	//to be called from either thread
	void lock(){
		if(verbose)printf("ofThread: waiting till mutex is unlocked\n");
		mutex.lock();
	}

	//to be called from either thread
	void unlock(){
		mutex.unlock();
		if(verbose)printf("ofThread: we are out -- mutex is now unlocked \n");
	};


	//to be called from the new thread
	void setName( string name ){
		if (isThreadRunning()){
			pthread_setname_np(name.c_str());
		}
	}


	//to be called from either thread
	void setPriority( float priority ){ /* [0..1]  0 being lowest, 1 being higest */
		if (isThreadRunning()){
			int p = ofMap( priority, 0, 1, sched_get_priority_min(SCHED_RR), sched_get_priority_max(SCHED_RR), true );
			struct sched_param param;
			param.sched_priority = p;

			if( pthread_setschedparam( myThread, SCHED_RR, &param) != 0 ){
				if(verbose) printf("Error setting pthread priority\n");
			}else{
				if(verbose) printf("setting thread priority to %d\n", p);
			}
		}
	}
	

protected:

	//you need to overide this with the function you want to thread
	virtual void threadedFunction(){
		if(verbose)printf("ofThread: overide threadedFunction with your own\n");
	}

	static void * thread(void * objPtr){
		ofxPThread* me	= (ofxPThread*)objPtr;
		me->threadedFunction();
		if (!me->someThreadRequestedToJoin){ //if we havent been joined (waitForThread) with another thread
			me->setThreadToCleanUpOnExit(); //if user hasn't released resources himself, we do
		}
		//reset all members vars in case we are requested to start a new therad
		me->stopThreadRequested = false;
		me->cleanedUp = false;
		me->verbose = false;
		me->someThreadRequestedToJoin = false;
		me->threadRunning = false;
		pthread_exit(NULL); //this returns!
	}

private:

		pthread_t		myThread;
		ofMutex			mutex;


		bool			someThreadRequestedToJoin;
		bool			cleanedUp; // if we already called pthread_detach() on the thread
		bool			stopThreadRequested; //use this flag to signal the thread to stop
		bool			threadRunning; 
		bool			verbose;
};


#endif
