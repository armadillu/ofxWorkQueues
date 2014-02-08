#ifndef _TEST_APP
#define _TEST_APP


#include "ofMain.h"

//#define WORK_QUEUE 1
#define DEDICATED_MULTIQUEUE 1
//#define DETACH_THREAD_QUEUE 1

#include "WorkQueue.h"
#include "DedicatedMultiQueue.h"
#include "DetachThreadQueue.h"

class testApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key){};
		void mouseMoved(int x, int y ){};
		void mouseDragged(int x, int y, int button){};		
		void mouseReleased(int x, int y, int button){};
		void windowResized(int w, int h){};
		void mousePressed(int x, int y, int button){};
		void exit();
		
		void addWorkUnitToWorkQueue(bool highPriority);
		void addWorkUnitToDedicatedMultiQueue(bool highPriority);
		void addWorkUnitToDetachThreadQueue(bool highPriority);

		#ifdef WORK_QUEUE
		WorkQueue * q1;				// A queue of work units, ine process after each other ( 1 thread  )
		#endif

		#ifdef DEDICATED_MULTIQUEUE
		DedicatedMultiQueue * q2;	// N balanced WorkQueues, N threads. If queue is not empty, only N threads spawned.
		#endif

		#ifdef DETACH_THREAD_QUEUE
		DetachThreadQueue * q3;		// N jobs processed concurrently, spawns 1 new thread per job
		#endif
	
};

#endif
