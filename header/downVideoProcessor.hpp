#ifndef DOWNVIDEOPROCESSOR_HPP_INCLUDED
#define DOWNVIDEOPROCESSOR_HPP_INCLUDED

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <math.h>
#include <string>
#include <fstream>

#include "roboHeaderFile.hpp"
#include "videoProcessor.hpp"

using namespace std;
using namespace cv;

struct PathMarker {
//specifiy a type of data structure for the rectangle's specs
    vector<vector<Point> > contour;
    Point2f pXY;    Point2f cXY;
    float width;    float height;       float angle;
    double area;    float distance;     bool ignore;
};

class DownVideoProcessor: public VideoProcessor{
public:

vector<PathMarker> pathMarkers;

DownVideoProcessor(string videoFilePath);

//functions that process the videos
void processFrame();
PathMarker findPathMarker (vector<Point> rectContour);

//gui functions for visual confirmation, mainly for debugging
string convertInt(int number);
void setLabel(Mat im, string label, Point org);
void drawPathMarkers();
void updateGUI();

};

#endif // ROBOHEADERFILE_INCLUDED
