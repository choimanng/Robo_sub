#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <math.h>
#include <string>
#include <fstream>

using namespace cv;
using namespace std;

//UI key constant for visual debugging
const int ESCAPE = 1048603;
const int SPACE = 1048608;
const int RIGHT = 2555904;
const int LEFT = 2424832;

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
int waitKeyTime = 30;

//for this abstract video pricessor, the recorded result is simply num of contours found
int* numOfTargetPerFrame;

VideoProcessor(string videoFilePath){
    cap =  VideoCapture(videoFilePath);     //input video file or from camera
    if(!cap.isOpened())return;
}

virtual void processFrame(){
    //resize the frame
    //resize(frame,frame,Size(frame.cols * resizeRatio , frame.rows * resizeRatio ));
    cvtColor(unprocessedFrame, hsvFrame, CV_BGR2HSV);
    //special case for red in which the lower bound's hue is numerically higher than upper bound's hue, eg, 170 - 10
    Scalar lowerBoundHSV = Scalar(H_MIN,S_MIN,V_MIN); // \todo acutal run does not need this
    Scalar upperBoundHSV = Scalar(H_MAX,S_MAX,V_MAX);
    // @todo if after calibration, hue does not need to wrap around axis for red, get rid of this if statement
    if(lowerBoundHSV[0] > upperBoundHSV[0]){
        Mat temp;
        inRange(hsvFrame,lowerBoundHSV,Scalar(180, upperBoundHSV[1], upperBoundHSV[2]),binaryFrame);
        inRange(hsvFrame,Scalar(0, lowerBoundHSV[1], lowerBoundHSV[2]),upperBoundHSV,temp);
        bitwise_or(temp, binaryFrame, binaryFrame);
    }
    else{
        inRange(hsvFrame,lowerBoundHSV,upperBoundHSV,binaryFrame);
    }

    //repeat "erodeDilateRepeat" many times of eroding and dilating
    // @todo get rid of this after calibration
    for(int i = 0; i < erodeDilateRepeat; i++){
        erode(binaryFrame, binaryFrame, erodeDilateKernel);
        dilate(binaryFrame, binaryFrame, erodeDilateKernel);
    }
    findContours(binaryFrame.clone(), contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
}

void recordCurrentFrameResult(){
    numOfTargetPerFrame[videoPos] = contours.size();
}

void processVideo(){
    //restart video no matter what
    videoPos = 0;
    cap.set(CV_CAP_PROP_POS_FRAMES,0);
    //reset result array
    numOfTargetPerFrame = new int[(int)cap.get(CV_CAP_PROP_FRAME_COUNT)];
    //process the video until runs out of frames
    for(;cap.read(unprocessedFrame); videoPos++){
        processFrame();
        recordCurrentFrameResult();
        cout << numOfTargetPerFrame[videoPos] << endl;
    }
}


//debugging or testing functions
void generateGUI(){
    //create the windows
    namedWindow( "Original", WINDOW_NORMAL );
    namedWindow( "HSV", WINDOW_NORMAL );
    namedWindow( "Binary", WINDOW_NORMAL );
    namedWindow( "Playback", WINDOW_NORMAL );
    namedWindow( "Control", WINDOW_NORMAL );
    createTrackbar("Frame", "Playback", &videoPos, cap.get(CV_CAP_PROP_FRAME_COUNT));
    createTrackbar("H min", "Control", &H_MIN, 180);
    createTrackbar("H max", "Control", &H_MAX, 180);
    createTrackbar("S min", "Control", &S_MIN, 255);
    createTrackbar("S max", "Control", &S_MAX, 255);
    createTrackbar("V min", "Control", &V_MIN, 255);
    createTrackbar("V max", "Control", &V_MAX, 255);
    createTrackbar("waitKeyTime", "Control", &waitKeyTime, 100);

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

void processVideoDebug(){
    //restart video no matter what
    videoPos = 0;
    cap.set(CV_CAP_PROP_POS_FRAMES,0);
    generateGUI();
    //reset result array
    numOfTargetPerFrame = new int[(int)cap.get(CV_CAP_PROP_FRAME_COUNT)];
    //process the video until runs out of frames
    int inputKey = -1;
    while(cap.read(unprocessedFrame) && inputKey != ESCAPE){
        processFrame();
        updateGUI();
        recordCurrentFrameResult();
        inputKey = waitKey(waitKeyTime);
        if(inputKey == SPACE){//space key pressed, then video enter paused state
            do{
                inputKey = waitKey(0);
            }while(inputKey != SPACE && inputKey != ESCAPE);//space key pressed again to resume
        }
    }
}

String getSetting(){
    stringstream s;
    s << "H_MIN=" << H_MIN << "; H_MAX=" << H_MAX << "; S_MIN;=" << S_MIN << "; S_MAX=" << S_MAX << "; V_MIN=" << V_MIN << "; V_MAX=" << V_MAX <<
    "; erodeDilateKernel size=" << erodeDilateKernel.size() << "; erodeDilateRepeat=" << erodeDilateRepeat;
    return s.str();
}

void writeResultToCSV(string csvPath){

}
};
