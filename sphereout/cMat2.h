#ifndef CMAT2_H
#define CMAT2_H

#include <math.h>
#include <string.h>

#include "cVec2.h"

class cMat2
{

	public:
		union
		{
			float mMat[9];
			float mMat3[3][3];
		};

		cMat2(float mat[9]);
		cMat2(float mat3[3][3]);
		cMat2();
		cMat2(float angle);

		inline cMat2 operator *(cMat2 &in)
		{
			cMat2 newMat;
			for(int i = 0; i < 3; i ++)
			{
				newMat.mMat3[i][0] = mMat3[0][i] * in.mMat3[0][0] + mMat3[1][i] * in.mMat3[0][1] + mMat3[2][i] * in.mMat3[0][2];
				newMat.mMat3[i][1] = mMat3[0][i] * in.mMat3[1][0] + mMat3[1][i] * in.mMat3[1][1] + mMat3[2][i] * in.mMat3[1][2];
				newMat.mMat3[i][2] = mMat3[0][i] * in.mMat3[2][0] + mMat3[1][i] * in.mMat3[2][1] + mMat3[2][i] * in.mMat3[2][2];
			}
			return newMat;
		}

		inline cVec2 operator *(cVec2 &in)
		{
			cVec2 newVec;
			newVec.x = mMat3[0][0] * in.x + mMat3[0][1] * in.y + mMat3[0][2] * 1;
			newVec.y = mMat3[1][0] * in.x + mMat3[1][1] * in.y + mMat3[1][2] * 1;
			return newVec;	
		} 
		inline void translate(const cVec2 &in)
		{
			mMat3[0][2] = in.x;
			mMat3[1][2] = in.y;
		}

};

#endif
