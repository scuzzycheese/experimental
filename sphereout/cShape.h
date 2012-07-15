#ifndef CSHAPE_H
#define CSHAPE_H

#include <float.h>

#include "cVec2.h"
#include "cMat2.h"
#include "cDraw.h"
#include "cShapeState.h"

class cShape
{
	public:

		cShapeState mState[2];
		int mCurState;

		unsigned int mColour;

		inline cShapeState *getCurState()
		{
			return &mState[mCurState];
		}
		inline cShapeState *getOldState()
		{
			return &mState[!mCurState];
		}

		//might have to think about this, but it's usefull
		float dt;

		cShape(); 
		~cShape();

		virtual void draw(cDraw &d) = 0;
		virtual void project(cVec2 &normal, float &minT0, float &maxT0, float &minT1, float &maxT1) = 0;

		void integrate(float dti);
};


class cCircle : public cShape
{
	protected:
		cVec2 mStartVec;
	public:
		float mRadius;
		cCircle();
		cCircle(float rad);
		void draw(cDraw &d);
		void project(cVec2 &normal, float &minT0, float &maxT0, float &minT1, float &maxT1);
};

class cPoly : public cShape
{

	public:
		cVec2 *mPoints;
		int mNumPoints;

		cPoly(int points);
		cPoly();
		void makeSquare(float width, float height);
		void project(cVec2 &normal, float &minT0, float &maxT0, float &minT1, float &maxT1);
		void draw(cDraw &d);
		~cPoly();
};

#endif
