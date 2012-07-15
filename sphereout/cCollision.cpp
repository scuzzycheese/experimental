#include "cCollision.h"

//Returns true when a collision is detected
float cCollision::findMinT(float minAT0, float maxAT0, float minBT0, float maxBT0, float minAT1, float maxAT1, float minBT1, float maxBT1)
{
		float T0A, T0B, T1A, T1B;
		float retVal;
	
		if(minBT0 < maxAT0 && minAT0 < maxBT0)
		{
			return 0;
		}

		if(maxAT0 < minBT0 && maxAT1 > minBT1)
		{
			retVal = (minBT0 - maxAT0) / (maxAT1 - maxAT0 - minBT1 + minBT0);
			//retVal = (T0B - T0A) / (T1A - T0A - T1B + T0B);
		}
		else if(maxBT0 < minAT0 && maxBT1 > minAT1)
		{
			retVal = (maxBT0 - minAT0) / (minAT1 - minAT0 - maxBT1 + maxBT0);
			//retVal = (T0B - T0A) / (T1A - T0A - T1B + T0B);
		}
		else
		{
			return -1;
		}

		return retVal;
		//return (T0B - T0A) / (T1A - T0A - T1B + T0B);
}

sColInfo cCollision::polyPoly(cPoly &polA, cPoly &polB)
{
}
	
bool cCollision::polyCircle(cPoly &polA, cCircle &circB, sColInfo &colInfo)
{
	bool collision = true;
	cVec2 vec1;
	cVec2 vec2;
	float minAT0, maxAT0, minBT0, maxBT0;
	float minAT1, maxAT1, minBT1, maxBT1;
	colInfo.mTime = -1;

	for(int i = 0; i < polA.mNumPoints - 1; i ++)
	{
		vec1 = polA.getOldState()->mTrans * polA.mPoints[i];
		vec2 = polA.getOldState()->mTrans * polA.mPoints[i + 1];
		cVec2 normal = (vec1 - vec2).normalised();
		float tmp = normal.x;
		normal.x = normal.y;
		normal.y = -tmp;
		polA.project(normal, minAT0, maxAT0, minAT1, maxAT1);
		circB.project(normal, minBT0, maxBT0, minBT1, maxBT1);

		float newT = findMinT(minAT0, maxAT0, minBT0, maxBT0, minAT1, maxAT1, minBT1, maxBT1);
		if(newT < 0)
		{
			collision = false;
			break;
		}
		else if(newT > colInfo.mTime)
		{
			colInfo.mTime = newT;
			colInfo.mColNormal = normal;
		}
	}

	if(collision)
	{
		vec1 = polA.getOldState()->mTrans * polA.mPoints[0];
		cVec2 normal = (vec1 - vec2).normalised();
		float tmp = normal.x;
		normal.x = normal.y;
		normal.y = -tmp;
		polA.project(normal, minAT0, maxAT0, minAT1, maxAT1);
		circB.project(normal, minBT0, maxBT0, minBT1, maxBT1);
	

		float newT = findMinT(minAT0, maxAT0, minBT0, maxBT0, minAT1, maxAT1, minBT1, maxBT1);
		if(newT < 0)
		{
			collision = false;
		}
		else if(newT > colInfo.mTime)
		{
			colInfo.mTime = newT;
			colInfo.mColNormal = normal;
		}


	}

	if(collision)
	{
		cVec2 least(10000, 10000);
		for(int i = 0; i < polA.mNumPoints - 1; i ++)
		{
			cVec2 vec = (polA.getOldState()->mTrans * polA.mPoints[i]) - circB.getOldState()->mPos;
			least = (vec.lengthSqr() < least.lengthSqr()) ? vec : least;
		}
		least.normalise();
		polA.project(least, minAT0, maxAT0, minAT1, maxAT1);
		circB.project(least, minBT0, maxBT0, minBT1, maxBT1);


		float newT = findMinT(minAT0, maxAT0, minBT0, maxBT0, minAT1, maxAT1, minBT1, maxBT1);
		if(newT < 0)
		{
			collision = false;
		}
		else if(newT > colInfo.mTime)
		{
			colInfo.mTime = newT;
			colInfo.mColNormal = least;
		}

	}

	//these have to go
	if(collision)
	{
		polA.mColour = 0xFFFF0000;
		//printf("MINT: %f - %f, %f\n", colInfo.mTime, colInfo.mColNormal.x, colInfo.mColNormal.y);
		return true;
	}
	else
	{
		polA.mColour = 0xFFFFFFFF;
		return false;
	}
	return collision;
}
	
bool cCollision::circleCircle(cCircle &circA, cCircle &circB, sColInfo &colInfo)
{
	cVec2 disBet = circA.getCurState()->mPos - circB.getCurState()->mPos;
	float combRad = (circA.mRadius + circB.mRadius) * (circA.mRadius + circB.mRadius);

	if(disBet.lengthSqr() < combRad)
	{
		colInfo.mColNormal.x = disBet.y;
		colInfo.mColNormal.y = -disBet.x;
		colInfo.mColNormal.normalise();
		return true;
	}

	return false;
}
