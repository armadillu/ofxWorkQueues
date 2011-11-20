#pragma once

#include "ofConstants.h"
#include "ofTypes.h"


#ifdef TARGET_WIN32
	#include <process.h>
#else
    #include <pthread.h>
#endif

class ofAdvancedThread : public ofThread{

	public:

	void setName( string name ){ 

		#ifndef TARGET_WIN32 
		#ifdef MAC_OS_X_VERSION_10_6
			pthread_setname_np(name.c_str());
			#endif
		#endif
	}

};
