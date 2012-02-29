/*
 *  GenericWorkUnit.h
 *  emptyExample
 *
 *  Created by Oriol Ferrer Mesià on 05/03/11.
 *  Copyright 2011 uri.cat. All rights reserved.
 *
 */

#pragma once

#include "ofMain.h"
#include "ofAdvancedThread.h"
#include <vector>

#define WORK_UNIT_DRAW_H	16
#define TILE_DRAW_GAP_H		2
#define TILE_DRAW_GAP_V		2

#define TILE_DRAW_SPACING	8
#define TEXT_DRAW_WIDTH		75

#define MAX_PENDING_ON_SCREEN 75

static int numWorkUnits = 0;


class GenericWorkUnit : public ofAdvancedThread{	//subclass this object to accomodate your desired work
	
	friend class DetachThreadQueue;
	
	public:
		
		enum WorkUnitStatus { UNPROCESSED = 0, PROCESSING, PROCESSED, PENDING_CANCELLATION,  CANCELLED, FAILED };
	
		GenericWorkUnit();
		void processInThread();		//call this to spawn a thread that will do the work (process()) and clean up after himself
		void cancel();				//flags the thread to stop its work, call this if you want to stop early

		virtual void process();		//subclass this method to do your work! <<<<<<<<<
		void preProcess();	
		void postProcess();		
		
		inline float getPercentDone(){ return processPercent;}
		inline void setPercentDone(float pct){ processPercent = pct;}
		WorkUnitStatus getStatus(){ return status;}
		int getID(){ return ID; }
			
		void setVerbose(bool b){ debug = b; }
		virtual void setGLColorAccordingToStatus();
	
		virtual void draw(int x, int y, int tileW, bool drawIDs);

	protected:

		WorkUnitStatus		status;			
		int					ID;				// grows automatically for each created GenericWorkUnit

	private:
	
		float				processPercent;	// you are suposed to update this, [0..1]
		void				threadedFunction();
		bool				debug;

};
