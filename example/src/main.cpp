#include "ofMain.h"
#include "testApp.h"
#include "ofAppGlutWindow.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

//========================================================================
int main( int argc, char *argv[] ){

    ofAppGlutWindow window;
	ofSetupOpenGL(&window, 800,500, OF_FULLSCREEN);			// <-------- setup the GL context

	ofRunApp( new testApp());

}
