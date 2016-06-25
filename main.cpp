#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <math.h>
#include <string>
#include <fstream>
using namespace cv;
using namespace std;

struct RectSpecs {
//specifiy a type of data structure for the rectangle's specs
    vector<vector<Point> > contour;
    Point2f pXY;    Point2f cXY;
    float width;    float height;       float angle;
    double area;    float distance;     bool ignore;
};

//set the saved result image's filename to record the result of different settings
string settingTitle = "Default";
string dataDir;

//UI key constant for visual debugging
int ESCAPE = 1048603;
int SPACE = 1048608;
int RIGHT = 2555904;
int LEFT = 2424832;

//Video file
string videoFilePath = "../../sample videos/Pipe.avi";
int videoFrameSize = 4964;
int endVideoPos = 4964;    //500
int expectedValue[4964];        //[videoFrameSize];

//expected value file
string eVFilePath = "Pipe_Data.txt";
int wrongContourNum = 0;

//values used for GUI manipulation
int videoPos = 0; //1200  //1820
bool isPaused = false;
int slowMotionms = 1;   //1;   //200;

//set the resize ratio of the frames
float resizeRatio = 1;

//predefined constant for identifying the orange color of pathmarker
int H_MIN = 0;
int H_MAX = 256;    //30
int S_MIN = 0;
int S_MAX = 256;
int V_MIN = 0;//200;
int V_MAX = 256;
int erodeKernelSize = 11;
int ignoreRectDist = 10;
int ignoreRectArea = 300;

//global objects used in main
VideoCapture cap;
Mat frame; //original one frame from video
Mat hsvFrame; //segmented frame in hsv colorspace
Mat binaryFrame; //black and white frame with white being the color of interest
Mat erodeKernel;
Mat dilateKernel;
vector<vector<Point> > contours;
vector<RectSpecs> pathMarkers;
ofstream resultFile;
ofstream dataFile;
ifstream expectedValueFile;

string convertInt(int number){
    stringstream ss;//create a stringstream
    ss << number;//add number to the stream
    return ss.str();//return a string with the contents of the stream
}

int waitKeyTime = 30;
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

void updateGUI(){
    //show the images in the windows
    imshow("Original", frame);
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

void SaveResult(){
    //Open result file
    if(!resultFile.is_open())
    {
        //Open the result text file
        resultFile.open("Result.txt", ios_base::app);
        //Create a data folder
        dataDir = "Data-" + settingTitle;
        string abc = "mkdir \"" + dataDir + "\"";
        system((char*)abc.c_str());
        //Create a data text file
        abc = dataDir + "/Data-" + settingTitle + ".txt";
        dataFile.open((char*)abc.c_str(), ios_base::app);
    }

    //save result images to jpg files for record
    if (expectedValue[videoPos] != pathMarkers.size())        //every time the contour numbers don't match the expected value
    {
        imwrite(dataDir + "/" + convertInt(expectedValue[videoPos]) + " vs " + convertInt(pathMarkers.size()) + " at " + convertInt(videoPos) + " (" + settingTitle + ").jpg", frame);
        wrongContourNum++;
    }

    //Save Result
    dataFile << convertInt(pathMarkers.size()) << endl;

    //Save Final Save Wrong Num Contour Results
    if (videoPos == endVideoPos)
    {
        resultFile << settingTitle + " has wrong frames : " + convertInt(wrongContourNum) << endl;
        wrongContourNum = 0;
    }
}

void loadExpectedValue(){
    //Open expected value file
    expectedValueFile.open((char*)eVFilePath.c_str());
    for (int j=0; j<=videoFrameSize; j++)
        expectedValueFile >> expectedValue[j];
}

void erodeDilate(int sizeIndex, Mat &frame){
    //erode & dilate the binaryframe
    erodeKernel = getStructuringElement( MORPH_RECT, Size(sizeIndex , sizeIndex));    // 11*11   20*20   30*30
    dilateKernel = erodeKernel;
    erode(binaryFrame, binaryFrame, erodeKernel);
    dilate(binaryFrame, binaryFrame, dilateKernel);
}

void setLabel(Mat im, string label, Point org){
    int fontface = FONT_HERSHEY_SIMPLEX;
    double scale = 0.4;
    int thickness = 1;
    int baseline = 0;
    Size text = getTextSize(label, fontface, scale, thickness, &baseline);
    rectangle(im, org + Point(0, baseline), org + Point(text.width, -text.height), CV_RGB(0,0,0), CV_FILLED);
    putText(im, label, org, fontface, scale, CV_RGB(255,255,255), thickness, 8);
}

RectSpecs findRectSpec (vector<Point> rectContour){
    //get the moments and centers of the contour
    Moments mu = moments( rectContour, false );
    Point2f cXY = Point2f(mu.m10/mu.m00, mu.m01/mu.m00);
    cXY = Point2f(cXY.x / resizeRatio, cXY.y / resizeRatio);

    //find the rotated rectangle & get its 4 vertices
    RotatedRect minRect = minAreaRect( Mat(rectContour) );
    Point2f rect_points[4];
    minRect.points( rect_points );

    //put the 4 vertices to create a contour for the minAreaRect
    vector<Point> contr(4);
    vector<vector<Point> > rectContours;
    contr[0]=rect_points[0];
    contr[1]=rect_points[1];
    contr[2]=rect_points[2];
    contr[3]=rect_points[3];
    rectContours.push_back(contr);

    //find the width & height of the rotated rectangle
    float rectWidth = max(minRect.size.width,minRect.size.height) / resizeRatio ;
    float rectHeight = min(minRect.size.width,minRect.size.height) / resizeRatio ;

    //find the midpoints of the 4 sides of the rectangle
    Point2f mp1 = Point2f((rect_points[0].x + rect_points[1].x) / 2, (rect_points[0].y + rect_points[1].y) / 2);
    Point2f mp2 = Point2f((rect_points[2].x + rect_points[3].x) / 2, (rect_points[2].y + rect_points[3].y) / 2);
    Point2f mp3 = Point2f((rect_points[1].x + rect_points[2].x) / 2, (rect_points[1].y + rect_points[2].y) / 2);
    Point2f mp4 = Point2f((rect_points[3].x + rect_points[0].x) / 2, (rect_points[3].y + rect_points[0].y) / 2);

    //compare the midpoints to find the one that will be used to calculate the angle of the rectangle
    Point2f pXY;
    if (sqrt(pow(mp2.y-mp1.y,2.0)+pow(mp2.x-mp1.x,2.0)) > sqrt(pow(mp4.y-mp3.y,2.0)+pow(mp4.x-mp3.x,2.0)))
    {
        if (mp1.y < mp2.y)
            pXY = mp1;
        else
            pXY = mp2;
    }
    else
    {
        if (mp3.y<mp4.y)
            pXY = mp3;
        else
            pXY = mp4;
    }

    //calculate the angle & add 180 degrees to any negative results caused by obtuse angle
    float rectAngle = int(atan((cXY.y-pXY.y)/(pXY.x-cXY.x))*180/M_PI);
    if (rectAngle < 0 )
        rectAngle = rectAngle + 180;

    //find the min distance of the contour to sides of the frame
    float dst = max(frame.cols,frame.rows);
    for (int j=0;j<4;j++)
    {
        if ( (rect_points[j].x - 0) < dst )
            dst =rect_points[j].x - 0;
        if ( (rect_points[j].y - 0) < dst )
            dst =rect_points[j].y - 0;
        if ( (frame.cols - rect_points[j].x) < dst )
            dst =frame.cols - rect_points[j].x;
        if ( (frame.rows - rect_points[j].y) < dst )
            dst =frame.rows - rect_points[j].y;
    }

    //put all specs to a "rectSpecs" data structure
    RectSpecs my_rect;
    my_rect.contour = rectContours;
    my_rect.width = rectWidth;
    my_rect.height = rectHeight;
    my_rect.area = contourArea(rectContours[0],false);
    my_rect.angle = rectAngle;
    my_rect.distance = dst;
    my_rect.pXY = pXY;
    my_rect.cXY = cXY;

    //set to ignore the contour as a pathMarker
    //if it's not near by the frame & the area of the contour is small
//    if (dst > ignoreRectDist && my_rect.area < ignoreRectArea)
//        my_rect.ignore = true;
//    else
//        my_rect.ignore = false;

    //return the rectSpecs
    return my_rect;
}

void drawPathMarkers(){
    for(int i = 0; i < pathMarkers.size(); i++)
    {
        //draw the rotated rectangle
        drawContours(frame, pathMarkers[i].contour, -1, Scalar(0,0,255), 2);
        //draw a small dot on the center
        circle(frame, pathMarkers[i].cXY, 7, Scalar(0, 0, 255), -1);
        //draw the horizontal line & angle line
        line(frame, pathMarkers[i].pXY, pathMarkers[i].cXY, Scalar(0, 0, 255), 2, 8 );
        line(frame, pathMarkers[i].cXY, Point2f(pathMarkers[i].cXY.x + 100,pathMarkers[i].cXY.y), Scalar(0, 0, 255), 2, 8 );
        //note the center coordinates, width, height & angle of the rectangle
        int textPosX = pathMarkers[i].cXY.x * resizeRatio + 10;
        int textPosY = pathMarkers[i].cXY.y * resizeRatio + 20;
        setLabel(frame, "Center: (" + convertInt(pathMarkers[i].cXY.x) + " , " + convertInt(pathMarkers[i].cXY.y) + ")", Point(textPosX,textPosY));
        setLabel(frame, "Width: " + convertInt(pathMarkers[i].width) + " , Height: " + convertInt(pathMarkers[i].height) , Point(textPosX,textPosY+20));
        setLabel(frame, "Angle (horizontal): " + convertInt(pathMarkers[i].angle) + " degrees", Point(textPosX,textPosY+40));
        setLabel(frame, "Angle (to turn): " + convertInt(90-pathMarkers[i].angle) + " degrees", Point(textPosX,textPosY+60));
        setLabel(frame, "Area: " + convertInt(pathMarkers[i].area), Point(textPosX,textPosY+80));
        setLabel(frame, "Min distance to frame: " + convertInt(pathMarkers[i].distance), Point(textPosX,textPosY+100));
    }
}

void processFrame(Mat &unprocessedFrame, Scalar lowerBoundHSV1, Scalar upperBoundHSV1, Mat &hsvFrame, Mat &binaryFrame, vector<vector<Point> > &unprocessedContours, vector<RectSpecs> &pathMarkers){
    //resize the frame
    //resize(frame,frame,Size(frame.cols * resizeRatio , frame.rows * resizeRatio ));
    cvtColor(unprocessedFrame, hsvFrame, CV_BGR2HSV);
    inRange(hsvFrame,Scalar(H_MIN,S_MIN,V_MIN),Scalar(H_MAX,S_MAX,V_MAX),binaryFrame);
    erodeDilate(11, binaryFrame);
    erodeDilate(11, binaryFrame);
    findContours(binaryFrame.clone(), contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
    pathMarkers.clear();
    for(int i = 0; i < unprocessedContours.size(); i++)
    {
        vector<Point> pathMarker(4);
        RectSpecs pathMarkerSpecs = findRectSpec(unprocessedContours[i]);
        pathMarkers.push_back(pathMarkerSpecs);
    }
}

void processFrame(Mat &unprocessedFrame, Scalar lowerBoundHSV1, Scalar upperBoundHSV1, Scalar lowerBoundHSV2, Scalar upperBoundHSV2, Mat &hsvFrame, Mat &binaryFrame, vector<vector<Point> > &unprocessedContours, vector<vector<Point> > &pathMarkers){
    //special case for red, which wraps around the hue axis
    if( &lowerBoundHSV2 != NULL){
        // \todo red hsv
    }
}

int main(int argc, char *argv[]){
    H_MIN = 0;
    H_MAX = 30;
    S_MIN = 0;
    S_MAX = 255;
    V_MIN = 200;
    V_MAX = 255;
    cap =  VideoCapture(videoFilePath);     //input video file or from camera
    if(!cap.isOpened()) {return -1;}
    generateGUI();
    int inputKey = -1;
    //using read instead of >> operator because read return false if reached the end
    while(cap.read(frame) && inputKey != ESCAPE && videoPos < endVideoPos){
        processFrame(frame, Scalar(H_MIN,S_MIN,V_MIN),Scalar(H_MAX,S_MAX,V_MAX), hsvFrame, binaryFrame, contours, pathMarkers);
        updateGUI();
        inputKey = waitKey(waitKeyTime);
        if(inputKey == SPACE){//space key pressed, then video enter paused state
            do{
                inputKey = waitKey(0);
                if(inputKey == RIGHT){//right key
                    processFrame(frame, Scalar(H_MIN,S_MIN,V_MIN),Scalar(H_MAX,S_MAX,V_MAX), hsvFrame, binaryFrame, contours, pathMarkers);
                    updateGUI();
                }else if(inputKey == LEFT && videoPos >= 0){
                    videoPos--;
                    processFrame(frame, Scalar(H_MIN,S_MIN,V_MIN),Scalar(H_MAX,S_MAX,V_MAX), hsvFrame, binaryFrame, contours, pathMarkers);
                    updateGUI();
                }
            }while(inputKey != SPACE && inputKey != ESCAPE);//space key pressed again to resume
        }
    }
}








