#include "cMat2.h"


cMat2::cMat2(float mat[9])
{
	memcpy(mMat, mat, sizeof(mat));
}
cMat2::cMat2(float mat3[3][3])
{
	memcpy(mat3, mMat3, sizeof(mat3));
}
cMat2::cMat2()
{
	memset(mMat, 0x00, sizeof(mMat));
	mMat3[0][0] = 1.0f; 
	mMat3[1][1] = 1.0f; 
	mMat3[2][2] = 1.0f; 
}
cMat2::cMat2(float angle)
{
	memset(mMat, 0x00, sizeof(mMat));
	mMat3[0][0] = cos(angle);
	mMat3[1][0] = sin(angle);
	mMat3[0][1] = sin(angle);
	mMat3[1][1] = -cos(angle);
	mMat3[2][2] = 1.0f;
}
