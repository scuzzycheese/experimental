#ifndef CSHAPESTATEH
#define CSHAPESTATEH

class cShapeState
{

	public:
		cVec2 mPos;
		cVec2 mVel;
		cVec2 mAcc;
		
		float mAngle;
		float mAngVel;
		float mAngAcc;

		cMat2 mTrans;

		cShapeState() : mAngle(0), mAngVel(0), mAngAcc(0)
		{
		}

};

#endif
