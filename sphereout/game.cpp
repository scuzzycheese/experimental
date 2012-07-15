#include "vidBuffer.h"
#include "cDraw.h"
#include "game.h"
#include "cVec2.h"
#include "cShape.h"
#include "cMat2.h"
#include "cCollision.h"

#include <stdio.h>
#include <math.h>

#include "PT/PixelToaster.h"
using namespace PixelToaster;
cDraw d;

class cGame : public Listener
{
	public:
		float mX, mY;

		cGame() : mX(0.0f), mY(0.0f)
		{
		}

		int run()
		{
			vidBuffer<2> test(640, 480);
			d = cDraw(test.getBuffer(0), test.getWidth(), test.getHeight());
		
		   Display display("Collision Detection", 640, 480, Output::Default, Mode::TrueColor );

			//register this class as a listner for events
			display.listener(this);
		
			cCircle circle(25);
			circle.getCurState()->mPos.x = 100;
			circle.getCurState()->mPos.y = 100;
			circle.getCurState()->mVel.x = 50;
			circle.getCurState()->mVel.y = 100;
			//circle.getCurState()->mAcc.y = 100;

			cCircle circ2(10);
			circ2.getCurState()->mPos.x = 200;
			circ2.getCurState()->mPos.y = 200;
			circ2.getCurState()->mVel.x = 50;
			circ2.getCurState()->mVel.y = 100;

			
		
			cPoly shape;
			shape.makeSquare(225, 10);
			//cCircle shape(10);
			shape.getCurState()->mPos.x = 320;
			shape.getCurState()->mPos.y = 240;
			//shape.getCurState()->mVel.x = 50;
			//shape.getCurState()->mVel.y = 100;
			//shape.mAcc.x = 0;
			shape.getCurState()->mAngVel = 0.5f;
			//shape.mAcc.y = 100;
		
			//shape.mAngVel = 10.0f;
			//shape.getCurState()->mAngle = 10.0f;
			
			unsigned int rad = 20;
			while(1)
			{
				if(circle.getCurState()->mPos.x > 640)
				{
					circle.getCurState()->mVel ^= cVec2(-1, 0);
					circle.getCurState()->mPos.x = 640;
				}
				if(circle.getCurState()->mPos.y > 480)
				{
					circle.getCurState()->mVel ^= cVec2(0, -1);
					circle.getCurState()->mPos.y = 480;
				}
				if(circle.getCurState()->mPos.x < 0)
				{
					circle.getCurState()->mVel ^= cVec2(1, 0);
					circle.getCurState()->mPos.x = 0;
				}
				if(circle.getCurState()->mPos.y < 0)
				{
					circle.getCurState()->mVel ^= cVec2(0, 1);
					circle.getCurState()->mPos.y = 0;
				}


				if(circ2.getCurState()->mPos.x > 640)
				{
					circ2.getCurState()->mVel ^= cVec2(-1, 0);
					circ2.getCurState()->mPos.x = 640;
				}
				if(circ2.getCurState()->mPos.y > 480)
				{
					circ2.getCurState()->mVel ^= cVec2(0, -1);
					circ2.getCurState()->mPos.y = 480;
				}
				if(circ2.getCurState()->mPos.x < 0)
				{
					circ2.getCurState()->mVel ^= cVec2(1, 0);
					circ2.getCurState()->mPos.x = 0;
				}
				if(circ2.getCurState()->mPos.y < 0)
				{
					circ2.getCurState()->mVel ^= cVec2(0, 1);
					circ2.getCurState()->mPos.y = 0;
				}

	
				//shape.getOldState()->mPos = shape.getCurState()->mPos;

				//shape.getCurState()->mPos.x = mX;
				//shape.getCurState()->mPos.y = mY;
	
				//shape.mState[shape.mCurState].mTrans = cMat2(shape.mState[shape.mCurState].mAngle);
				//shape.mState[shape.mCurState].mTrans.translate(shape.mState[shape.mCurState].mPos);

				shape.integrate(1.0f / 10.0f);
				circle.integrate(1.0f / 10.0f);
				circ2.integrate(1.0f / 10.0f);
				//usleep(100000);
		
				test.copyBuffer(0, 1);	
		
				circle.draw(d);
				shape.draw(d);
				circ2.draw(d);

				sColInfo colInfo;
				if(cCollision::polyCircle(shape, circle, colInfo))
				{
					d.line(shape.getCurState()->mPos.x, shape.getCurState()->mPos.y, shape.getCurState()->mPos.x + colInfo.mColNormal.x * 10, shape.getCurState()->mPos.y + colInfo.mColNormal.y * 10, 0xFFFFFFFF);
					circle.getCurState()->mPos = circle.getOldState()->mPos;
					circle.getCurState()->mVel ^= colInfo.mColNormal;
					//circle.getCurState()->mVel += ((shape.getCurState()->mPos - shape.getOldState()->mPos) / 10.0f);
					circle.integrate(1.0f / 10.0f);
				}
				if(cCollision::polyCircle(shape, circ2, colInfo))
				{
					circ2.getCurState()->mPos = circ2.getOldState()->mPos;
					circ2.getCurState()->mVel ^= colInfo.mColNormal;
				}


				if(cCollision::circleCircle(circle, circ2, colInfo))
				{
					circ2.getCurState()->mPos = circ2.getOldState()->mPos;
					circ2.getCurState()->mVel ^= colInfo.mColNormal;
					circ2.integrate(1.0f / 10.0f);
				}				
		
				display.update((TrueColorPixel *)test.getBuffer(0));
			}
			return 0;
		}	

		void onMouseMove(DisplayInterface &display, Mouse mouse)
		{
			mX = mouse.x;
			mY = mouse.y;
		}

		bool onClose(DisplayInterface &display)
		{
			printf("Quitting\n");
			exit(0);
			return true;
		}
};


int main(int argc, char **argv)
{
	cGame game;
	return game.run();
}
