#include "testApp.h"
#include "MyWorkUnit.h"

int workUnitsToProcess = 30;
int howManyPerCycle = 6;	//num threads to distribute the job on
int maxPending = 100;		//how many work units can there be pending on the queue (buffer)
int iterations = 2000;		//how long in ms it takes to complete the work of each of our threads (each MyWorkUnit)

void testApp::setup(){
	

	ofSetFrameRate(60);
	ofBackground(22, 22, 22);
	ofSetVerticalSync(true);
	
	q1 = new WorkQueue();	
	//q1->setVerbose(true);
	q1->setMeasureTimes(true);
	q1->setMaxQueueLength(maxPending);
	
	q2 = new DedicatedMultiQueue(howManyPerCycle );	//4 concurrent queues
	//q2->setVerbose(true);
	q2->setMeasureTimes(true);
	q2->setRestTimeMillis(3);	//how much the dispatcher sleeps after each dispathing
	q2->setMaxPendingQueueLength(maxPending);
	q2->setIndividualWorkerQueueMaxLen(3);	//3 work units buffered per queue
	
	q3 = new DetachThreadQueue();
	//q3->setVerbose(true);
	q3->setRestTimeMillis(3);	//how much the dispatcher sleeps after each dispathing
	q3->setMaxPendingQueueLength(maxPending);
	q3->setMaxJobsAtATime(howManyPerCycle);	//workUnits at a time
	
	processedInWorkQueue = 0;
	processedInDedicatedMultiQueue = 0;
	processedInDetachThreadQueue = 0;

}


void testApp::update(){
	
	if ( workUnitsToProcess > 0 ){

//################################## Work Queue ##############################################
		
		//add our custom workUnit (job) (GenericWorkUnit subclass) to our WorkQueue
		MyWorkUnit * w1;
		if (processedInWorkQueue < workUnitsToProcess){
			w1 = new MyWorkUnit( iterations );
			//w1->setVerbose(true);
			q1->addWorkUnit(w1);
			processedInWorkQueue++;
		}
		
		//collect results.....
		GenericWorkUnit * wr1 = q1->retrieveNextProcessedUnit();
		if (wr1 != NULL){
			printf( "## got result for GenericWorkUnit %d from WorkQueue\n", w1->getID());
			delete wr1;
		}
		
		
//################################## Dedicated MultiQueue ##############################################
				
		//add a job to our DedicatedMultiQueue
		MyWorkUnit * w2;
		if (processedInDedicatedMultiQueue < workUnitsToProcess){
			w2 = new MyWorkUnit( iterations );
			q2->addWorkUnit(w2);
			processedInDedicatedMultiQueue++;
		}
		
		//collect results..... 
		GenericWorkUnit * wr2 = q2->retrieveNextProcessedUnit();
		if (wr2 != NULL){
			printf( "## got result for GenericWorkUnit %d from DedicatedMultiQueue\n", wr2->getID());
			delete wr2;
		}

		
//################################## DetachThreadQueue ##############################################		

		//add a job to our DetachThreadQueue
		MyWorkUnit * w3;
		if (processedInDetachThreadQueue < workUnitsToProcess){
			w3 = new MyWorkUnit( iterations );
			q3->addWorkUnit(w3);
			processedInDetachThreadQueue++;
		}

		//collect results...
		GenericWorkUnit * wr3 = q3->retrieveNextProcessedUnit();
		if (wr3 != NULL){
			printf( "## got result for GenericWorkUnit %d from DetachThreadQueue\n", wr3->getID());
			delete wr3;
		}
	}
}


void testApp::draw(){
	
	glColor3ub(255,0,0);
	ofDrawBitmapString( ofToString( ofGetFrameRate(), 2), 10, 10);

	glTranslatef(10, 10, 0);
	
	int cellWidth = 25;
	bool drawID = true;
	
	q1->draw(cellWidth, drawID);
	
	glTranslatef(0,70,0);
	q2->draw(cellWidth, drawID);
	
	glTranslatef(0, 70 + 20 * (howManyPerCycle + 1),0);
	q3->draw(cellWidth, drawID);
}


void testApp::exit(){
	delete q1;
	delete q2;
	delete q3;
	printf("exiting!\n");
};
