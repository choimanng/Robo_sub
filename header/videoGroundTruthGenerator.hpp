#ifndef VIDEOGROUNDTRUTHGENERATOR_HPP_INCLUDED
#define VIDEOGROUNDTRUTHGENERATOR_HPP_INCLUDED

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <math.h>
#include <string>
#include <fstream>
#include "videoProcessor.hpp"
#include "downVideoProcessor.hpp"
using namespace cv;
using namespace std;



class VideoGroundTruthGenerator{
public:

//opencv operational objects
VideoCapture cap; //object used to read video
Mat unprocessedFrame; //original one frame from video
Mat hsvFrame; //segmented frame in hsv colorspace
Mat binaryFrame;  //black and white frame with white being the color of interest
vector<vector<Point> > contours;
int videoPos = 0;

//UI setting
int waitKeyTime = 1;

//recording the ground truth
int* expectedValues;
int currentFrameGroundTruth = 0;

VideoGroundTruthGenerator(string videoFilePath);

void recordCurrentFrameResult();

void generateGUI();

virtual void updateGUI();

int getCurrentFrame();

void processVideo();

void writeResultToCSV(char* csvPath);

};

#endif // ROBOHEADERFILE_INCLUDED
