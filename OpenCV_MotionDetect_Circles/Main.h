#include<iostream>
#include<time.h>
#include<cstdlib>
#include<Windows.h>

//#include<opencv2\core\core.hpp>
#include<opencv2\highgui\highgui.hpp>
#include<opencv2\imgproc\imgproc.hpp>

const char	 *WIN_CAMERA = "WebCam Feed";
const char	 *WIN_MOTION = "Processed Motion";
const int	 CAMERA_WIDTH = 640;
const int	 CAMERA_HEIGHT = 480;
const int	 THRESHOLD_VALUE = 45;				//  Amount of change in (gray) pixel intesity 
												//  for it to qualify as indicating motion

const double SECONDS_BETWEEN_CONTACTS = .2;		//  controls latency between contact checks
const float	 DELTA_COEFFICIENT = .8;			//  controls ball speeds (multiplier)
const int	 MAX_DELTA = 25;					//  top speed in pixels delta
const int	 NUMBER_BALLS = 6;
const int	 MAX_RADIUS = 45;					//  min/max ball radius
const int	 MIN_RADIUS = 20;
const bool	 BALLS_COLLIDE = false;				//  set to false if balls should not interact w/ ea other
	
time_t timeNow, timeLast;
clock_t ticksNow;
float secondsBetweenContacts;

int tmpInt, framesPerSecond, thresholdValue;
int i,j,k, pocX, pocY;
//int top_contactAccumulator, top_contactCounter;
//int left_contactAccumulator, left_contactCounter;
//int bottom_contactAccumulator, bottom_contactCounter;
//int right_contactAccumulator, right_contactCounter;

int contactAccumulators[4];							//  clockwise from top
int contactCounters[4];								//  clockwise from top
int contactCountersMax;

void computeReflection(int nX, int nY);

class Ball {
	public:
		int x, y, dX, dY, radius, r, g, b, colorWeight;
		clock_t ticksLast;
		cv::Scalar color;
		Ball() {
			radius = ( rand() % (MAX_RADIUS - MIN_RADIUS)) + MIN_RADIUS;
			x = rand() % (640 - 2 * radius) + radius;
			y = rand() % (480 - 2 * radius) + radius;
			dX = dY = 0;
			colorWeight = rand() % 3;
			r = (colorWeight==0) ? rand() % 105 + 140 : rand() % 50;
			g = (colorWeight==1) ? rand() % 105 + 140 : rand() % 50;
			b = (colorWeight==2) ? rand() % 105 + 140 : rand() % 50;
			color = cv::Scalar(b,g,r);
			ticksLast = clock();
		}
};

