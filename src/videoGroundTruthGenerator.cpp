#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <math.h>
#include <string>
#include <fstream>

#include "roboHeaderFile.h"

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

VideoGroundTruthGenerator(string videoFilePath){
cout << "constructor" << endl;
    cap =  VideoCapture(videoFilePath);     //input video file or from camera
    if(!cap.isOpened())return;
}


void test(){
    cout << "test" << endl;
}

void recordCurrentFrameResult(){
    expectedValues[videoPos] = contours.size();
}

//debugging or testing functions
void generateGUI(){
    //create the windows
    namedWindow( "Original", WINDOW_NORMAL );
    namedWindow( "Playback", WINDOW_NORMAL );
    createTrackbar("Frame", "Playback", &videoPos, cap.get(CV_CAP_PROP_FRAME_COUNT));
    createTrackbar("waitKeyTime", "Playback", &waitKeyTime, 100);

    //base width and height are derived from frame size, used to better position windows for debugging purpose
    //resize the windows
    int baseWidth, baseHeight;
    baseWidth = (int)(cap.get(CV_CAP_PROP_FRAME_WIDTH)*0.8);
    baseHeight = (int)(cap.get(CV_CAP_PROP_FRAME_HEIGHT)*0.8);
    resizeWindow("Original", baseWidth, baseHeight);
    resizeWindow("HSV", baseWidth, baseHeight);
    resizeWindow("Binary", baseWidth, baseHeight);
    resizeWindow("Playback", 800, 50);

    //move the windows
    moveWindow("Original", 0, 0);
    moveWindow("HSV", baseWidth, 0);
    moveWindow("Binary", baseWidth*2, 0);
    moveWindow("Playback", 0, baseHeight+25);
    moveWindow("Control", 800, baseHeight+25);
}

virtual void updateGUI(){
    imshow("Original", unprocessedFrame);
    imshow("HSV", hsvFrame);
    imshow("Binary", binaryFrame);

    //update time trackbar
    if(cap.get(CV_CAP_PROP_POS_FRAMES)- videoPos != 1)
    {
        cap.set(CV_CAP_PROP_POS_FRAMES, videoPos);
    }
    else
    {
        videoPos++;
        setTrackbarPos("Frame", "Playback", videoPos);
    }
}

void processVideo(){
    //restart video no matter what
    videoPos = 0;
    cap.set(CV_CAP_PROP_POS_FRAMES,0);
    generateGUI();
    //reset result array
    expectedValues = new int[(int)cap.get(CV_CAP_PROP_FRAME_COUNT)];
    //process the video until runs out of frames
    int inputKey = -1;
    while(cap.read(unprocessedFrame) && inputKey != ESCAPE){
        cout << "what is go in" << endl;
        updateGUI();
        recordCurrentFrameResult();
        inputKey = waitKey(waitKeyTime);

        if(inputKey == RIGHT){

        }
        else if(inputKey == LEFT){

        }
        if(inputKey == SPACE){//space key pressed, then video enter paused state
            do{
//                cout << inputKey << endl;
                inputKey = waitKey(0);
            }while(inputKey != SPACE && inputKey != ESCAPE);//space key pressed again to resume
        }
    }
}



void writeResultToCSV(char* csvPath){
    //open output file
    ofstream cvsResultFile;
    cvsResultFile.open(csvPath, ios_base::app);

    //write setting to csv file
    cvsResultFile << "ground truth,n/a";

    //write individual frame's target count in this processing
    int frameCount = (int)cap.get(CV_CAP_PROP_FRAME_COUNT);
    for (int j=0; j<frameCount; j++){
        cvsResultFile << "," << expectedValues[j];
    }
    cvsResultFile << endl;
    cvsResultFile.close();
//    for(int i = 0; i < 100; i++)
//        cout << expectedValues[i];
}

};
