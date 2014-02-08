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

#define WORK_UNIT_DRAW_H		16.0f
#define TILE_DRAW_GAP_H			2.0f
#define TILE_DRAW_GAP_V			2.0f

#define TILE_DRAW_SPACING		8.0f
#define TEXT_DRAW_WIDTH			75.0f
#define HIGH_PRIORITY_MARK_SIZE	0.2f
#define BITMAP_MSG_HEIGHT		0.7

#define MAX_PENDING_ON_SCREEN	50

static int numWorkUnits = 0;

class GenericWorkUnit : public ofThread{	//subclass this object to accomodate your desired work
	
	friend class DetachThreadQueue;
	
	public:
		
		enum WorkUnitStatus { UNPROCESSED = 0, PROCESSING, PROCESSED, PENDING_CANCELLATION,  CANCELLED, FAILED };
	
		GenericWorkUnit();
		virtual ~GenericWorkUnit(){};
		void processInThread();		//call this to spawn a thread that will do the work (process()) and clean up after himself
		void cancel();				//flags the thread to stop its work, call this if you want to stop early

		virtual void process() = 0;		//subclass this method to do your work! <<<<<<<<<
			
		inline float getPercentDone(){ return processPercent;}
		WorkUnitStatus getStatus(){ return status;}
		int getID(){ return ID; }
			
		void setVerbose(bool b){ debug = b; }	
		virtual void draw(int x, int y, int tileW, bool drawIDs);
		
	protected:

		WorkUnitStatus		status;			
		int					ID;				// grows automatically for each created GenericWorkUnit
	
		void				setStatusFailed(){ status = FAILED; }
		bool				isJobPendingCancelation(){ return status == PENDING_CANCELLATION; }

		virtual void		setGLColorAccordingToStatus(); //subclass to colorize your work units
		inline void			setPercentDone(float pct){ processPercent = pct;}
	
	private:

		void				preProcess();	
		void				postProcess();		

		float				processPercent;	// you are suposed to update this, [0..1] if you want to get progressbar updates
		void				threadedFunction();

		bool				debug;
		bool				highPriority;
	
		friend class		WorkQueue; 
		friend class		DedicatedMultiQueue;

};
