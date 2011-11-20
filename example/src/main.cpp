#include "ofMain.h"
#include "testApp.h"
#include "ofAppGlutWindow.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

//========================================================================
int main( int argc, char *argv[] ){

    ofAppGlutWindow window;
	ofSetupOpenGL(&window, 1024,768, OF_WINDOW);			// <-------- setup the GL context

	ofRunApp( new testApp());

}
