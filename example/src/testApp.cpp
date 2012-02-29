#include "testApp.h"
#include "MyWorkUnit.h"

int workUnitsToProcess = 1000;	//how many jobs should be completed by each queue 
int howManyPerCycle = 4;			//num threads to distribute the jobs on
int maxPending = 100;				//how many work units can there be pending on the queue (buffer length)

void testApp::setup(){	

	ofSetFrameRate(60);
	ofEnableAlphaBlending();
	ofBackground(22, 22, 22);
	//ofSetVerticalSync(true);
	
	q1 = new WorkQueue();		// A queue of work units, ine process after each other ( 1 thread  )
	//q1->setVerbose(true);
	q1->setMeasureTimes(true);
	q1->setMaxQueueLength(maxPending);
	
	q2 = new DedicatedMultiQueue(howManyPerCycle );	// N balanced WorkQueues, N threads. If queue is never empty (always work to do), only N threads spawned.
	//q2->setVerbose(true);
	q2->setMeasureTimes(true);
	q2->setRestTimeMillis(1);	//how much the dispatcher sleeps after each dispathing
	q2->setMaxPendingQueueLength(maxPending);	//queued job buffer length. If try to add a job and buffer is longer than this, job will be rejected.
	q2->setIndividualWorkerQueueMaxLen(10);	//3 work units buffered per queue
	
	q3 = new DetachThreadQueue();	// N jobs processed concurrently, spawns 1 new thread per job
	//q3->setVerbose(true);
	q3->setRestTimeMillis(1);	//how much the dispatcher sleeps after each dispathing
	q3->setMaxPendingQueueLength(maxPending);	//queued job buffer length. If try to add a job and buffer is longer than this, job will be rejected.
	q3->setMaxJobsAtATime(howManyPerCycle);	//how many jobs to process at a time
	
	//counters for each queue
	processedInWorkQueue = 0;
	processedInDedicatedMultiQueue = 0;
	processedInDetachThreadQueue = 0;

}


void testApp::update(){
	
	if ( workUnitsToProcess > 0 ){

		//################################## Work Queue ##############################################
		
		//first, add our custom workUnit (job) (GenericWorkUnit subclass) to our WorkQueue
		MyWorkUnit * w1;
		if (processedInWorkQueue < workUnitsToProcess){
			w1 = new MyWorkUnit( (int)ofRandom(20) ); //calculate some random factorial
			q1->addWorkUnit(w1);
			processedInWorkQueue++;
		}
		
		//then, keep trying to collect results.....
		GenericWorkUnit * wr1 = q1->retrieveNextProcessedUnit();

		if ( wr1 != NULL ){	//we got a result from the queue!
			MyWorkUnit * wu1 = (MyWorkUnit*)wr1; //force a cast to our WorkUnit Type
			cout <<"## got result for operation (" << wu1->getID() << ") WorkQueue. Fact(" << wu1->getInput() << ") = (" << wu1->getResult() << ")" << endl;
			delete wr1;
		}
		
		
		//################################## Dedicated MultiQueue ##############################################
				
		//add a job to our DedicatedMultiQueue
		MyWorkUnit * w2;
		if (processedInDedicatedMultiQueue < workUnitsToProcess){
			w2 = new MyWorkUnit( (int)ofRandom(20) );
			q2->addWorkUnit(w2);
			processedInDedicatedMultiQueue++;
		}
		
		//collect results..... 
		GenericWorkUnit * wr2 = q2->retrieveNextProcessedUnit();
		if (wr2 != NULL){
			MyWorkUnit * wu1 = (MyWorkUnit*)wr2; //force a cast to our WorkUnit Type
			cout <<"## got result for operation (" << wu1->getID() << ") WorkQueue. Fact(" << wu1->getInput() << ") = (" << wu1->getResult() << ")" << endl;
			delete wr2;
		}

		
		//################################## DetachThreadQueue ##############################################		

		//add a job to our DetachThreadQueue
		MyWorkUnit * w3;
		if (processedInDetachThreadQueue < workUnitsToProcess){
			w3 = new MyWorkUnit( (int)ofRandom(20) );
			q3->addWorkUnit(w3);
			processedInDetachThreadQueue++;
		}

		//collect results...
		GenericWorkUnit * wr3 = q3->retrieveNextProcessedUnit();
		if (wr3 != NULL){
			MyWorkUnit * wu1 = (MyWorkUnit*)wr3; //force a cast to our WorkUnit Type
			cout <<"## got result for operation (" << wu1->getID() << ") WorkQueue. Fact(" << wu1->getInput() << ") = (" << wu1->getResult() << ")" << endl;
			delete wr3;
		}
	}
}


void testApp::draw(){
	
	glColor3ub(255,0,0);
	ofDrawBitmapString( ofToString( ofGetFrameRate(), 2), 10, 10);

	glTranslatef(20, 60, 0);
	
	int cellWidth = 10;		//width of each cell
	bool drawID = false;	//draw the ID on top of the cell
	
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
