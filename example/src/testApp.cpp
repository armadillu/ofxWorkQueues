#include "testApp.h"
#include "MyWorkUnit.h"

int howManyPerCycle = 2;			//num threads to distribute the jobs on
int maxPending = 50;				//how many work units can there be pending on the queue (buffer length)
int maxFactorialToCalculate = 30;

void testApp::setup(){	

	ofSetFrameRate(60);
	ofEnableAlphaBlending();
	ofBackground(11, 11, 11);
	ofSetVerticalSync(true);
	
	q1 = new WorkQueue();		// A queue of work units, one processed after each other ( 1 thread )
	//q1->setVerbose(true);
	q1->setMeasureTimes(true);			//measure how long each job takes, and draw the average 
	q1->setMaxQueueLength(maxPending);	//queued job buffer length. If try to add a job and buffer is longer than this, job will be rejected.
	
	q2 = new DedicatedMultiQueue(howManyPerCycle );	// N balanced WorkQueues, N threads. If queue is never empty (there's always jobs to do), only N threads spawned ever.
	//q2->setVerbose(true);
	q2->setMeasureTimes(true);					//measure how long each job takes, and draw the average 
	q2->setRestTimeMillis(1);					//how much the dispatcher sleeps after each dispathing. Low numbers make it faster, but takes more cpu
	q2->setMaxPendingQueueLength(maxPending);	//queued job buffer length. If try to add a job and buffer is longer than this, job will be rejected.
	q2->setIndividualWorkerQueueMaxLen(10);		//how many work units each thread queue can have
	
	q3 = new DetachThreadQueue();	// N jobs processed concurrently, spawns a new thread per job
	//q3->setVerbose(true);
	q3->setRestTimeMillis(1);					//how much the dispatcher sleeps after each dispathing
	q3->setMaxPendingQueueLength(maxPending);	//queued job buffer length. If try to add a job and buffer is longer than this, job will be rejected.
	q3->setMaxJobsAtATime(howManyPerCycle);		//how many jobs to process at a time (threads)
	
	//counters of processed jobs for each queue
	processedInWorkQueue = 0;
	processedInDedicatedMultiQueue = 0;
	processedInDetachThreadQueue = 0;
}


void testApp::update(){
		
	q1->update();
	q2->update();
	q3->update();
	

	//################################## Work Queue ##############################################
	
	//add a job to the work queue
	addWorkUnitToWorkQueue(false);
	
	//then, keep trying to collect results.....
	GenericWorkUnit * wr1 = q1->retrieveNextProcessedUnit();

	if ( wr1 != NULL ){	//we got a result from the queue!
		MyWorkUnit * wu1 = (MyWorkUnit*)wr1; //force a cast to our WorkUnit Type
		cout <<"## got result for operation (" << wu1->getID() << ") WorkQueue. Fact(" << wu1->getInput() << ") = (" << wu1->getResult() << ")" << endl;
		delete wr1; //once we got our result, delete the work unit that was holding it
	}
	
	
	//################################## Dedicated MultiQueue ##############################################
			
	//add a new job to our DedicatedMultiQueue
	addWorkUnitToDedicatedMultiQueue(false);
	
	//collect results..... 
	GenericWorkUnit * wr2 = q2->retrieveNextProcessedUnit();
	if (wr2 != NULL){
		MyWorkUnit * wu1 = (MyWorkUnit*)wr2; //force a cast to our WorkUnit Type
		cout <<"## got result for operation (" << wu1->getID() << ") WorkQueue. Fact(" << wu1->getInput() << ") = (" << wu1->getResult() << ")" << endl;
		delete wr2; //once we got our result, delete the work unit that was holding it
	}

	
	//################################## DetachThreadQueue ##############################################		

	//add a new job to our DetachThreadQueue
	addWorkUnitToDetachThreadQueue(false);
	
	//collect results...
	GenericWorkUnit * wr3 = q3->retrieveNextProcessedUnit();
	if (wr3 != NULL){
		MyWorkUnit * wu1 = (MyWorkUnit*)wr3; //force a cast to our WorkUnit Type
		cout <<"## got result for operation (" << wu1->getID() << ") WorkQueue. Fact(" << wu1->getInput() << ") = (" << wu1->getResult() << ")" << endl;
		delete wr3; //once we got our result, delete the work unit that was holding it
	}
	
}


void testApp::draw(){
	
	glColor3ub(255,0,0);
	ofDrawBitmapString( ofToString( ofGetFrameRate(), 2), 10, 10);

	glTranslatef(20, 60, 0);
	
	int cellWidth = 10;		//width of each cell
	bool drawID = false;	//draw each job's ID on top of its cell
	
	q1->draw(cellWidth, drawID);
	
	glTranslatef(0,70,0);
	q2->draw(cellWidth, drawID);
	
	glTranslatef(0, 70 + 20 * (howManyPerCycle + 1), 0);
	q3->draw(cellWidth, drawID);
}


void testApp::exit(){
	delete q1;
	delete q2;
	delete q3;
	printf("exiting!\n");
};

void testApp::addWorkUnitToWorkQueue(bool highPriority){
	
	MyWorkUnit * w1 = new MyWorkUnit( (int)ofRandom(maxFactorialToCalculate) ); //calculate some random factorial
	bool didAdd = q1->addWorkUnit(w1, highPriority);
	if ( !didAdd ){	//if work unit was rejected (queue full), delete it
		delete w1;	
	}else{
		processedInWorkQueue++;
	}
}

void testApp::addWorkUnitToDedicatedMultiQueue(bool highPriority){
	
	MyWorkUnit * w2 = new MyWorkUnit( (int)ofRandom(maxFactorialToCalculate) );
	bool didAdd = q2->addWorkUnit(w2, highPriority);
	if ( !didAdd ){	//if work unit was rejected (queue full), delete it
		delete w2;
	}else{
		processedInDedicatedMultiQueue++;
	}
}

void testApp::addWorkUnitToDetachThreadQueue(bool highPriority){
	
	MyWorkUnit * w3 = new MyWorkUnit( (int)ofRandom(maxFactorialToCalculate) );
	bool didAdd = q3->addWorkUnit(w3, highPriority);
	if ( !didAdd ){	//if work unit was rejected (queue full), delete it
		delete w3;
	}else{
		processedInDetachThreadQueue++;
	}
}


void testApp::keyPressed(int key){

	addWorkUnitToWorkQueue(true);
	addWorkUnitToDedicatedMultiQueue(true);
	addWorkUnitToDetachThreadQueue(true);
};

