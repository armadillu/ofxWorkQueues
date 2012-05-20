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

	
	void setPriority( int priority ){
		#ifndef TARGET_WIN32 	
		int p = ofClamp( priority, sched_get_priority_min(SCHED_OTHER), sched_get_priority_max(SCHED_OTHER) );
		struct sched_param param;	
		param.sched_priority = p;	
		if( pthread_setschedparam( myThread, SCHED_OTHER, &param) != 0 ){
			if (verbose) printf("Error setting pthread priority\n");
		}else{
			if (verbose) printf("setting thread priority to %d\n", p);
		}
		#endif
	}
};
