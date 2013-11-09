#include "Main.h"

using namespace std;
using namespace cv;

Mat matFrame, matGray, matGrayPrevious, matMotion, matMotionDisplay;

int main(int argc, char *argv[])
{
	srand(time(0));
	framesPerSecond = 0; timeLast = time(&timeNow); tmpInt = 1;
	ticksNow = clock();
	VideoCapture cap(0);
	Ball balls[NUMBER_BALLS];

	if( !cap.isOpened() ) {
		cout << "ERROR  -  VideoCapture was not opened!!" << endl;
		return -1;
	} // end if
	
	//  INITIALIZE NAMED WINDOWS
	namedWindow(WIN_CAMERA, CV_WINDOW_AUTOSIZE);
	moveWindow(WIN_CAMERA, 0, 440);
	namedWindow(WIN_MOTION, CV_WINDOW_AUTOSIZE);
	moveWindow(WIN_MOTION, 640, 440);

	//  Initial LOAD, FLIP, 2GRAY, and CLONE
	cap >> matFrame;
	flip(matFrame, matFrame, 1);
	cvtColor(matFrame, matGray, CV_BGR2GRAY);
	matGrayPrevious = matGray.clone();
	
	//  Running LOOP  /////////////////////////////////////////////////////////
	while (cvWaitKey(20) == -1) {
		
		//  IDENTIFY MOTION
		absdiff(matGray, matGrayPrevious, matMotion);
		threshold(matMotion, matMotion, THRESHOLD_VALUE, 255, THRESH_BINARY);
		matMotionDisplay = matMotion.clone();

		//  ERODE and DILATE motion to reduce noise
		erode(matMotion, matMotion, Mat(getStructuringElement(MORPH_RECT,Size(3,3))));
		dilate(matMotion, matMotion, Mat(getStructuringElement(MORPH_RECT,Size(3,3))));

		
		//  DRAW Balls[]  -- Sub-Loop  -  Once for ea ball ///////////////////////////
		for (j = 0; j < NUMBER_BALLS; j++) {
			circle(matFrame, Point(balls[j].x, balls[j].y), balls[j].radius, balls[j].color, -1, CV_AA);
			if (BALLS_COLLIDE) circle(matMotion, Point(balls[j].x, balls[j].y), 
				balls[j].radius-3, Scalar(255,255,255), 1, CV_AA);
			else circle(matMotionDisplay, Point(balls[j].x, balls[j].y), 
				balls[j].radius-3, Scalar(255,255,255), 1, CV_AA);
			if (BALLS_COLLIDE) rectangle(matMotion, Point(balls[j].x-balls[j].radius+3, balls[j].y-balls[j].radius+3), 
				Point(balls[j].x+balls[j].radius-3, balls[j].y+balls[j].radius-3), Scalar(255,255,255), 1, 4);
			else rectangle(matMotionDisplay, Point(balls[j].x-balls[j].radius+3, balls[j].y-balls[j].radius+3), 
				Point(balls[j].x+balls[j].radius-3, balls[j].y+balls[j].radius-3), Scalar(255,255,255), 1, 4);

			//  COMPUTE DELTA changes for CONTACT with ball
				//  first make sure ball is completely inside WIN_CAMERA to avoid out of bounds exceptions
				//  and that sufficient time has past since last contact to avoid double tapping/overlapping 
			if (balls[j].x > balls[j].radius && balls[j].y > balls[j].radius && balls[j].x < 640-balls[j].radius && 
					balls[j].y < 480-balls[j].radius && clock() - balls[j].ticksLast > CLOCKS_PER_SEC * SECONDS_BETWEEN_CONTACTS) {
				for (i = 0; i < 4; i++) {								// zero all counters
					contactCounters[i]=0; 
					contactAccumulators[i]=0; 
				}
				for (i = 0; i < balls[j].radius*2; i++) {					// check for contact - top of box
					if (matMotion.at<uchar>(balls[j].y-balls[j].radius, balls[j].x-balls[j].radius+i) == 255) {
						contactAccumulators[0] += balls[j].x-balls[j].radius+i;	// add all points of contact
						contactCounters[0]++;							// count number of POCs
					}
				}
				for (i = 0; i < balls[j].radius*2; i++) {					// check for contact - right side of box
					if (matMotion.at<uchar>(balls[j].y-balls[j].radius+i, balls[j].x+balls[j].radius) == 255) {
						contactAccumulators[1] += balls[j].y-balls[j].radius+i;	// add all points of contact
						contactCounters[1]++;							// count number of POCs
					}
				}	
				for (i = 1; i < balls[j].radius*2-2; i++) {					// check for contact - left side box
					if (matMotion.at<uchar>(balls[j].y+balls[j].radius, balls[j].x-balls[j].radius+i) == 255) {
						contactAccumulators[2] += balls[j].x-balls[j].radius+i;	// add all points of contact
						contactCounters[2]++;							// count number of POCs
					}
				}
				for (i = 0; i < balls[j].radius*2; i++) {					// check for contact - bottom of box
					if (matMotion.at<uchar>(balls[j].y-balls[j].radius+i, balls[j].x-balls[j].radius) == 255) {
						contactAccumulators[3] += balls[j].y-balls[j].radius+i;	// add all points of contact
						contactCounters[3]++;							// count number of POCs
					}
				}
				contactCountersMax = 0;
				for (i = 1; i < 4; i++)									// determine side with greatest contact
					if (contactCounters[i] > contactCounters[contactCountersMax]) contactCountersMax = i;

				if (contactCounters[contactCountersMax] > 0) {
					switch(contactCountersMax) {						// set pocX and pocY for new delta copmutation
					case 0:
						pocX = contactAccumulators[0]/contactCounters[0]; pocY = balls[j].y-balls[j].radius;
						break;
					case 1:
						pocY = contactAccumulators[1]/contactCounters[1]; pocX = balls[j].x+balls[j].radius;
						break;
					case 2:
						pocX = contactAccumulators[2]/contactCounters[2]; pocY = balls[j].y+balls[j].radius;
						break;
					case 3:
						pocY = contactAccumulators[3]/contactCounters[3]; pocX = balls[j].x-balls[j].radius;
						break;
					}
					balls[j].dX = (int)((balls[j].x - pocX) * DELTA_COEFFICIENT);
					balls[j].dY = (int)((balls[j].y - pocY) * DELTA_COEFFICIENT);			//  compute reflection
					balls[j].ticksLast = clock();									//  update tick timerLast
					
				} // end if
			} // end COMPUTE DELTA changes for CONTACT

			//  BUMPERS for contact with frame edges
			if (balls[j].x < balls[j].radius) balls[j].dX = abs(balls[j].dX);
			else if (balls[j].x > 640-balls[j].radius) balls[j].dX = -1 * abs(balls[j].dX);
			else if (balls[j].y < balls[j].radius) balls[j].dY = abs(balls[j].dY);
			else if (balls[j].y > 480-balls[j].radius) balls[j].dY = -1 * abs(balls[j].dY);

			//  LIMIT DELTAS to MAX_DELTA
			if (balls[j].dX > MAX_DELTA) balls[j].dX = MAX_DELTA;  
			if (balls[j].dX < -1*MAX_DELTA) balls[j].dX = -1*MAX_DELTA;
			if (balls[j].dY > MAX_DELTA) balls[j].dY = MAX_DELTA;  
			if (balls[j].dY < -1*MAX_DELTA) balls[j].dY = -1*MAX_DELTA;

			//  UPDATE X and Y
			balls[j].x += balls[j].dX; balls[j].y += balls[j].dY; 

		} // end for - DRAW BALLS -  Sub Loop  -  once for each ball  ///////////////////////////////////

		//  SHOW images in windows (***resize is very resource intensive)
		//resize(matFrame, matFrame, Size(960,720));
		imshow(WIN_CAMERA, matFrame);
		if (BALLS_COLLIDE) imshow(WIN_MOTION, matMotion);
		else imshow(WIN_MOTION, matMotionDisplay);

		//  CALCULATE FPS
		timeLast = timeNow;
		time(&timeNow);
		if (timeNow == timeLast) tmpInt++;
		else { 
			if (framesPerSecond != tmpInt) {
				framesPerSecond = tmpInt; 
				cout << "FPS  " << framesPerSecond << " " << "\r";
				tmpInt = 1; 
			} // end if
		} // end else

		//  Subsequent LOAD, FLIP, CLONE and 2GRAY
		cap >> matFrame;
		flip(matFrame, matFrame, 1);
		matGrayPrevious = matGray.clone();
		cvtColor(matFrame, matGray, CV_BGR2GRAY);

	} // end Running LOOP  ////////////////////////////////////////////////////

	return 0;
} // end main


