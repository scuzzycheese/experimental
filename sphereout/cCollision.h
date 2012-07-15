#ifndef CCOLLISION_H
#define CCOLLISION_H

#include "cVec2.h"
#include "cShape.h"

#include <stdio.h>

struct sColInfo
{
	cVec2 mColNormal;
	float mTime;
};

class cCollision
{
	public:	
		//Returns true when a collision is detected
		static float findMinT(float minAT0, float maxAT0, float minBT0, float maxBT0, float minAT1, float maxAT1, float minBT1, float maxBT1);
		static sColInfo polyPoly(cPoly &polA, cPoly &polB);
		static bool polyCircle(cPoly &polA, cCircle &circB, sColInfo &colInfo);
		static bool circleCircle(cCircle &circA, cCircle &circB, sColInfo &colInfo);
};

#endif
