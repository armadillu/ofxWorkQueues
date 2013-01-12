#include "testApp.h"
#include "BlurWorkUnit.h"


int numThreads = 4;			//num threads to distribute the jobs on

void testApp::setup(){	

	ofSetFrameRate(60);
	ofEnableAlphaBlending();
	ofBackground(22, 22, 22);
	ofSetVerticalSync(true);
	
	camWidth 		= 640;	// try to grab at this size. 
	camHeight 		= 480;
	
	vidGrabber.initGrabber(camWidth,camHeight);

	q = new DedicatedMultiQueue( numThreads );	// N balanced WorkQueues, N threads. If queue is never empty (always work to do), only N threads spawned.
	//q->setVerbose(true);
	q->setMeasureTimes(true);
	q->setRestTimeMillis(1);	//how much the dispatcher sleeps after each dispathing
	
	//buffer lengths. For realtime stuff you might want to keep as small as possible
	q->setMaxPendingQueueLength( 8 );	//queued job buffer length. If try to add a job and buffer is longer than this, job will be rejected (Drop frame)
	q->setIndividualWorkerQueueMaxLen(1);	//N work units buffered per queue. This length is critical to keep images "sorted"
	
	blurredImage.allocate(camWidth, camHeight);
}


void testApp::update(){
	
	vidGrabber.grabFrame();
	q->update();
	
	if (vidGrabber.isFrameNew()){
		//add a job to our DedicatedMultiQueue
		BlurWorkUnit * w;
		w = new BlurWorkUnit( vidGrabber.getPixels(), camWidth, camHeight );
		if ( !q->addWorkUnit(w) ){	//if our work unit wastn accepted, delete it (dont leak)
			delete w;
		}
	}

	//collect results..... 
	GenericWorkUnit * processedWU = q->retrieveNextProcessedUnit();
	if (processedWU != NULL){
		BlurWorkUnit * fcwu = (BlurWorkUnit*)processedWU; //force a cast to our WorkUnit Type	
		blurredImage = fcwu->getResult();
		delete processedWU;
	}
}


void testApp::draw(){

	glColor3ub(255,255,255);
	vidGrabber.draw(0,0);
	blurredImage.draw(camWidth, 0);
	glColor4ub(255,255,255, 16);
	vidGrabber.draw(camWidth,0);
	
	glColor3ub(255,0,0);
	ofDrawBitmapString( ofToString( ofGetFrameRate(), 2), 10, 15);
	
	glTranslatef(20, 520, 0); //move below cam image to draw queue
	
	int cellWidth = 14;		//width of each cell
	bool drawID = true;	//draw the ID on top of the cell
	
	q->draw(cellWidth, drawID);
}


void testApp::exit(){
	delete q;
	printf("exiting!\n");
};
