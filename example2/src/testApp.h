#ifndef _TEST_APP
#define _TEST_APP


#include "ofMain.h"

#include "DedicatedMultiQueue.h"
#include "ofxCvHaarFinder.h"

class testApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key){};
		void keyReleased(int key){};
		void mouseMoved(int x, int y ){};
		void mouseDragged(int x, int y, int button){};		
		void mouseReleased(int x, int y, int button){};
		void windowResized(int w, int h){};
		void mousePressed(int x, int y, int button){};
		void exit();
		
		ofVideoGrabber vidGrabber;
		int	camWidth;
		int	camHeight;
		
		DedicatedMultiQueue * q1;	// N balanced WorkQueues, N threads. If queue is not empty, only N threads spawned.
		ofxCvColorImage blurredImage;
};

#endif