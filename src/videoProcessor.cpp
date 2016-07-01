#include "../header/videoProcessor.hpp"

using namespace cv;
using namespace std;

//UI key constant for visual debugging
int ESCAPE = 1048603;
int SPACE = 1048608;
int RIGHT = 2555904;
int LEFT = 2424832;

VideoProcessor::VideoProcessor(string videoFilePath){
    cap =  VideoCapture(videoFilePath);     //input video file or from camera
    if(!cap.isOpened())return;
}

void VideoProcessor::processFrame(){
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

void VideoProcessor::recordCurrentFrameResult(){
    numOfTargetPerFrame[videoPos] = contours.size();
}

void VideoProcessor::processVideo(){
    //restart video no matter what
    videoPos = 0;
    cap.set(CV_CAP_PROP_POS_FRAMES,0);
    //reset result array
    numOfTargetPerFrame = new int[(int)cap.get(CV_CAP_PROP_FRAME_COUNT)];
    //process the video until runs out of frames
    while(cap.read(unprocessedFrame)){
        processFrame();
        recordCurrentFrameResult();
        videoPos++;
    }
}


//debugging or testing functions
void VideoProcessor::generateGUI(){
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

void VideoProcessor::updateGUI(){
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

void VideoProcessor::processVideoDebug(){
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
        recordCurrentFrameResult();
        updateGUI();
        inputKey = waitKey(waitKeyTime);
        if(inputKey == SPACE){//space key pressed, then video enter paused state
            do{
                inputKey = waitKey(0);
            }while(inputKey != SPACE && inputKey != ESCAPE);//space key pressed again to resume
        }
    }
}

String VideoProcessor::getSetting(){
    stringstream s;
    s << "H_MIN=" << H_MIN << "; H_MAX=" << H_MAX << "; S_MIN;=" << S_MIN << "; S_MAX=" << S_MAX << "; V_MIN=" << V_MIN << "; V_MAX=" << V_MAX <<
    "; erodeDilateKernel size=" << erodeDilateKernel.size() << "; erodeDilateRepeat=" << erodeDilateRepeat;
    return s.str();
}

//recording and comparing result

void VideoProcessor::loadExpectedValue(char* eVFilePath){
    //Open expected value file
    ifstream expectedValueFile;
    expectedValueFile.open(eVFilePath);

    //dynamically create the array to hold expected value data
    int frameCounte = (int)cap.get(CV_CAP_PROP_FRAME_COUNT);
    expectedValues = new int[frameCounte];
    for (int j=0; j<frameCounte; j++)
        expectedValueFile >> expectedValues[j];
    expectedValueFile.close();
}

void VideoProcessor::writeResultToCSV(char* csvPath){
    //open output file
    ofstream cvsResultFile;
    cvsResultFile.open(csvPath, ios_base::app);

    //write setting to csv file
    cvsResultFile << getSetting();

    //write error count
    int frameCount = (int)cap.get(CV_CAP_PROP_FRAME_COUNT);
    int errorCount = 0;
    int frameDifferenceCount = 0;
    for(int i = 0; i < frameCount; i++){
        errorCount += abs(expectedValues[i] - numOfTargetPerFrame[i]);
        if(expectedValues[i] != numOfTargetPerFrame[i]){
            frameDifferenceCount++;
        }
    }
    cvsResultFile << ",wrong_target_count=" << errorCount << "," << "frameDifferenceCount=" << frameDifferenceCount;

    //write individual frame's target count in this processing
    for (int j=0; j<frameCount; j++){
        cvsResultFile << "," << numOfTargetPerFrame[j];
    }
    cvsResultFile << endl;
    cvsResultFile.close();
//    for(int i = 0; i < 100; i++)
//        cout << expectedValues[i];
}
