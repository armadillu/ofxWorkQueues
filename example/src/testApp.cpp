#include "testApp.h"
#include "MyWorkUnit.h"

int howManyPerCycle = 04;	//num threads to distribute the jobs on (per demoed structure)
int maxPending = 20;		//how many work units can there be pending on the queue (buffer length)
int maxFactorialToCalculate = 31;
int resultBuffer = 100;		//how many results to keep in buffer before gathering
float addWorkDuringFirstSeconds = 10;
bool verbose = false;
bool measureTimes = true;


void testApp::setup(){	

	ofSetVerticalSync(true);
	ofSetFrameRate(64);
	ofEnableAlphaBlending();
	ofBackground(190);

	#ifdef WORK_QUEUE
	q1 = new WorkQueue();		// A queue of work units, one processed after each other ( 1 thread )
	q1->setVerbose(verbose);
	q1->setMeasureTimes(measureTimes);	//measure how long each job takes, and draw the average 
	q1->setMaxQueueLength(maxPending);	//queued job buffer length. If try to add a job and buffer is longer than this, job will be rejected.
	#endif 

	#ifdef DEDICATED_MULTIQUEUE
	q2 = new DedicatedMultiQueue(howManyPerCycle);	// N balanced WorkQueues, N threads. If queue is never empty (there's always jobs to do), only N threads spawned ever.
	q2->setVerbose(verbose);
	q2->setMeasureTimes(measureTimes);			//measure how long each job takes, and draw the average 
	q2->setRestTimeMillis(1);					//how much the dispatcher sleeps after each dispatch. Low numbers make it more responsive, but takes more cpu
	q2->setMaxPendingQueueLength(maxPending);	//queued job buffer length. If try to add a job and buffer is longer than this, job will be rejected.
	q2->setIndividualWorkerQueueMaxLen(1);		//how many work units each thread queue can have
	#endif


	#ifdef DETACH_THREAD_QUEUE
	q3 = new DetachThreadQueue();	// N jobs processed concurrently, spawns a new thread per job
	q3->setVerbose(verbose);
	q3->setRestTimeMillis(0);					//how much the dispatcher sleeps after each dispathing
	q3->setMaxPendingQueueLength(maxPending);	//queued job buffer length. If try to add a job and buffer is longer than this, job will be rejected.
	q3->setMaxJobsAtATime(howManyPerCycle);		//how many jobs to process at a time (threads)
	#endif

}


void testApp::update(){

	//################################## Work Queue ##############################################
	#ifdef WORK_QUEUE

	q1->update();

	//add a job to the work queue
	if (ofGetElapsedTimef() < addWorkDuringFirstSeconds)
		addWorkUnitToWorkQueue(false);
	
	//then, keep trying to collect results.....
	if (q1->getProcessedQueueLength() > resultBuffer){ //always leave "resultBuffer" results on the queue
		GenericWorkUnit * wr1 = q1->retrieveNextProcessedUnit();

		if ( wr1 != NULL ){	//we got a result from the queue!
			MyWorkUnit * wu1 = (MyWorkUnit*)wr1; //force a cast to our WorkUnit Type
			if(verbose) cout <<"## got result for operation (" << wu1->getID() << ") WorkQueue. Fact(" << wu1->getInput() << ") = (" << wu1->getResult() << ")" << endl;
			delete wr1; //once we got our result, delete the work unit that was holding it
		}
	}
	#endif
	
	//################################## Dedicated MultiQueue ##############################################

	#ifdef DEDICATED_MULTIQUEUE

	q2->update();

	//add a new job to our DedicatedMultiQueue
	if (ofGetElapsedTimef() < addWorkDuringFirstSeconds)
		addWorkUnitToDedicatedMultiQueue(false);
	
	//collect results.....
	if (q2->getProcessedQueueLength() > resultBuffer){ //always leave "resultBuffer" results on the queue
		GenericWorkUnit * wr2 = q2->retrieveNextProcessedUnit();
		if (wr2 != NULL){
			MyWorkUnit * wu1 = (MyWorkUnit*)wr2; //force a cast to our WorkUnit Type
			if(verbose) cout <<"## got result for operation (" << wu1->getID() << ") DedicatedMultiQueue. Fact(" << wu1->getInput() << ") = (" << wu1->getResult() << ")" << endl;
			delete wr2; //once we got our result, delete the work unit that was holding it
		}
	}
	#endif
	
	//################################## DetachThreadQueue ##############################################		

	#ifdef DETACH_THREAD_QUEUE
	q3->update();
	
	//add a new job to our DetachThreadQueue
	if (ofGetElapsedTimef() < addWorkDuringFirstSeconds)
		addWorkUnitToDetachThreadQueue(false);
	
	//collect results...
	if (q3->getProcessedQueueLength() > resultBuffer){ //always leave "resultBuffer" results on the queue
		GenericWorkUnit * wr3 = q3->retrieveNextProcessedUnit();
		if (wr3 != NULL){
			MyWorkUnit * wu1 = (MyWorkUnit*)wr3; //force a cast to our WorkUnit Type
			if(verbose) cout <<"## got result for operation (" << wu1->getID() << ") DetachThreadQueue. Fact(" << wu1->getInput() << ") = (" << wu1->getResult() << ")" << endl;
			delete wr3; //once we got our result, delete the work unit that was holding it
		}
	}
	#endif
}


void testApp::draw(){
	
	glColor3ub(255,0,0);
	ofDrawBitmapString( ofToString( ofGetFrameRate(), 2), 10, 10);
	
	int cellWidth = 10;		//width of each cell
	bool drawID = false;	//draw each job's ID on top of its cell

	#ifdef WORK_QUEUE
	q1->draw(30, 30, cellWidth, drawID);
	#endif

	#ifdef DEDICATED_MULTIQUEUE
	q2->draw(30, 100, cellWidth, drawID);
	#endif

	#ifdef DETACH_THREAD_QUEUE
	q3->draw(30, 140 + 20 * (howManyPerCycle + 1), cellWidth, drawID);
	#endif
}


void testApp::exit(){
	#ifdef WORK_QUEUE
	delete q1;
	#endif

	#ifdef DEDICATED_MULTIQUEUE
	delete q2;
	#endif

	#ifdef DETACH_THREAD_QUEUE
	delete q3;
	#endif
	printf("exiting!\n");
};

void testApp::addWorkUnitToWorkQueue(bool highPriority){
	#ifdef WORK_QUEUE
	MyWorkUnit * w1 = new MyWorkUnit( (int)ofRandom(maxFactorialToCalculate) ); //calculate some random factorial
	bool didAdd = q1->addWorkUnit(w1, highPriority);
	if ( !didAdd ){	//if work unit was rejected (queue full), delete it
		delete w1;	
	}
	#endif
}

void testApp::addWorkUnitToDedicatedMultiQueue(bool highPriority){
	#ifdef DEDICATED_MULTIQUEUE
	MyWorkUnit * w2 = new MyWorkUnit( (int)ofRandom(maxFactorialToCalculate) );
	bool didAdd = q2->addWorkUnit(w2, highPriority);
	if ( !didAdd ){	//if work unit was rejected (queue full), delete it
		delete w2;
	}
	#endif
}

void testApp::addWorkUnitToDetachThreadQueue(bool highPriority){

	#ifdef DETACH_THREAD_QUEUE
	MyWorkUnit * w3 = new MyWorkUnit( (int)ofRandom(maxFactorialToCalculate) );
	bool didAdd = q3->addWorkUnit(w3, highPriority);
	if ( !didAdd ){	//if work unit was rejected (queue full), delete it
		delete w3;
	}
	#endif
}

void testApp::keyPressed(int key){

	//add one high-priority job to each queue on key press
	#ifdef WORK_QUEUE
	addWorkUnitToWorkQueue(true);
	#endif

	#ifdef DEDICATED_MULTIQUEUE
	addWorkUnitToDedicatedMultiQueue(true);
	#endif

	#ifdef DETACH_THREAD_QUEUE
	addWorkUnitToDetachThreadQueue(true);
	#endif
	
};

