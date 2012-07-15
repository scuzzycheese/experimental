#include "cShape.h"


cShape::cShape() : mColour(0xFFFFFFFF), dt(1), mCurState(0)
{
}
cShape::~cShape()
{
}
void cShape::integrate(float dti)
{
	dt = dti;
	int oldState = mCurState;
	int newState = !mCurState;

	mState[newState].mPos = mState[oldState].mPos + mState[oldState].mVel * dt;
	mState[newState].mVel = mState[oldState].mVel + mState[oldState].mAcc * dt;

	mState[newState].mAngle = mState[oldState].mAngle + mState[oldState].mAngVel * dt;
	mState[newState].mAngVel = mState[oldState].mAngVel + mState[oldState].mAngAcc * dt;

	mState[newState].mTrans = cMat2(mState[newState].mAngle);
	mState[newState].mTrans.translate(mState[newState].mPos);

	mCurState = newState;
}



cCircle::cCircle()
{
}
cCircle::cCircle(float rad) : mRadius(rad), mStartVec(0, 0)
{
}
void cCircle::draw(cDraw &d)
{
	cVec2 vec;
	vec = getCurState()->mTrans * mStartVec;
	d.circle(vec.x, vec.y, mRadius, mColour, 1);
}
void cCircle::project(cVec2 &normal, float &minT0, float &maxT0, float &minT1, float &maxT1)
{
	cVec2 tmpVec = getOldState()->mTrans * mStartVec;
	float proj = tmpVec * normal;
	minT0 = proj - mRadius;
	maxT0 = proj + mRadius;

	tmpVec = getCurState()->mTrans * mStartVec;
	proj = tmpVec * normal;
	minT1 = proj - mRadius;
	maxT1 = proj + mRadius;
}





cPoly::cPoly(int points) : mNumPoints(points)
{
	mPoints = new cVec2[points];
}

cPoly::cPoly() : mPoints(NULL), mNumPoints(0)
{
}

void cPoly::makeSquare(float width, float height)
{
	mPoints = new cVec2[4];
	mNumPoints = 4;
	const float hWidth = width * 0.5f;
	const float hHeight = height * 0.5f;
	mPoints[0].set(hWidth, hHeight);
	mPoints[1].set(-hWidth, hHeight);
	mPoints[2].set(-hWidth, -hHeight);
	mPoints[3].set(hWidth, -hHeight);
}

void cPoly::project(cVec2 &normal, float &minT0, float &maxT0, float &minT1, float &maxT1)
{
	extern cDraw d;
	float minT;

	minT0 = 100000;
	maxT0 = -100000;
	for(int i = 0; i < mNumPoints; i ++)
	{
		cVec2 tmpVec = getOldState()->mTrans * mPoints[i];
		float proj = tmpVec * normal;
		minT0 = (minT0 > proj) ? proj : minT0;
		maxT0 = (maxT0 < proj) ? proj : maxT0;
	}

	minT1 = 100000;
	maxT1 = -100000;
	for(int i = 0; i < mNumPoints; i ++)
	{
		cVec2 tmpVec = getCurState()->mTrans * mPoints[i];
		float proj = tmpVec * normal;
		minT1 = (minT1 > proj) ? proj : minT1;
		maxT1 = (maxT1 < proj) ? proj : maxT1;
	}

}

void cPoly::draw(cDraw &d)
{
	cVec2 vec1, vec2;
	for(int i = 0; i < mNumPoints - 1; i ++)
	{
		vec1 = getCurState()->mTrans * mPoints[i];
		vec2 = getCurState()->mTrans * mPoints[i + 1];
		d.line(vec1.x, vec1.y, vec2.x, vec2.y, mColour);
	}
	vec1 = getCurState()->mTrans * mPoints[0];
	d.line(vec1.x, vec1.y, vec2.x, vec2.y, mColour);
}

cPoly::~cPoly()
{
	delete[](mPoints);
}

