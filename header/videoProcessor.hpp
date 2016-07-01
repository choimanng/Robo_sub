#ifndef VIDEOPROCESSOR_INCLUDED
#define VIDEOPROCESSOR_INCLUDED

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <math.h>
#include <string>
#include <fstream>

#include "roboHeaderFile.hpp"

using namespace cv;
using namespace std;

//UI key constant for visual debugging
extern int ESCAPE;
extern int SPACE;
extern int RIGHT;
extern int LEFT;

class VideoProcessor{
public:

//variables that affect the target's identification
int H_MIN = 0;
int H_MAX = 30;
int S_MIN = 0;
int S_MAX = 255;
int V_MIN = 200;
int V_MAX = 255;
int erodeDilateRepeat = 2;
Mat erodeDilateKernel = getStructuringElement( MORPH_RECT, Size(11 , 11));

//opencv operational objects
VideoCapture cap; //object used to read video
Mat unprocessedFrame; //original one frame from video
Mat hsvFrame; //segmented frame in hsv colorspace
Mat binaryFrame;  //black and white frame with white being the color of interest
vector<vector<Point> > contours;
int videoPos = 0;

//UI setting
int waitKeyTime = 1;

//for this abstract video pricessor, the recorded result is simply num of contours found
int* numOfTargetPerFrame;

//recording and comparing result
int* expectedValues;

//function that processes the video
VideoProcessor(string videoFilePath);
virtual void processFrame();
void recordCurrentFrameResult();
void processVideo();

//debugging or testing functions
void generateGUI();
virtual void updateGUI();
void processVideoDebug();
String getSetting();

//recording and comparing result
void loadExpectedValue(char* eVFilePath);
void writeResultToCSV(char* csvPath);
};


#endif // ROBOHEADERFILE_INCLUDED
