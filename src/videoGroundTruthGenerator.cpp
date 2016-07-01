#include "../header/videoGroundTruthGenerator.hpp"


using namespace cv;
using namespace std;

VideoGroundTruthGenerator::VideoGroundTruthGenerator(string videoFilePath){
    cap =  VideoCapture(videoFilePath);     //input video file or from camera
    if(!cap.isOpened()){
        cout << "couldn't load video. check path?";
    }
}

void VideoGroundTruthGenerator::recordCurrentFrameResult(){
    //is minus 1 because videoPos has to be updated in function updateGUI
    expectedValues[videoPos-1] = currentFrameGroundTruth;
}

//debugging or testing functions
void VideoGroundTruthGenerator::generateGUI(){
    //create the windows
    namedWindow( "Original", WINDOW_NORMAL );
    namedWindow( "Playback", WINDOW_NORMAL );
    createTrackbar("Frame", "Playback", &videoPos, cap.get(CV_CAP_PROP_FRAME_COUNT));
    createTrackbar("waitKeyTime", "Playback", &waitKeyTime, 100);

    //base width and height are derived from frame size, used to better position windows for debugging purpose
    //resize the windows
    int baseWidth, baseHeight;
    baseWidth = (int)(cap.get(CV_CAP_PROP_FRAME_WIDTH));
    baseHeight = (int)(cap.get(CV_CAP_PROP_FRAME_HEIGHT));
    resizeWindow("Original", baseWidth, baseHeight);
    resizeWindow("Playback", 800, 50);

    //move the windows
    moveWindow("Original", 0, 0);
    moveWindow("Playback", 0, baseHeight+25);
}

void VideoGroundTruthGenerator::updateGUI(){
    setLabel(unprocessedFrame, convertInt(currentFrameGroundTruth), Point(20, 20));
    imshow("Original", unprocessedFrame);

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

void VideoGroundTruthGenerator::processVideo(){
    //restart video no matter what
    videoPos = 0;
    cap.set(CV_CAP_PROP_POS_FRAMES,0);
    generateGUI();
    //reset result array
    expectedValues = new int[(int)cap.get(CV_CAP_PROP_FRAME_COUNT)];
    //process the video until runs out of frames
    int inputKey = -1;
    while(cap.read(unprocessedFrame) && inputKey != ESCAPE){
        updateGUI();
        inputKey = waitKey(waitKeyTime);
        cout << inputKey << endl;
        int numberKey = inputKey - ZERO;
        if(numberKey >= 0 && numberKey < 10){
            currentFrameGroundTruth = numberKey;
        }
        recordCurrentFrameResult();
        if(inputKey == SPACE){//space key pressed, then video enter paused state
            do{
                inputKey = waitKey();
                if(inputKey == RIGHT){
                    //forward one frame is similar to normal playing one frame then pause again
                    cap.read(unprocessedFrame);
                    updateGUI();
                }
                else if(inputKey == LEFT){
                    videoPos = videoPos-2 >= 0 ? videoPos-2 : 0;
                    cap.set(CV_CAP_PROP_POS_FRAMES, videoPos);
                    cap.read(unprocessedFrame);
                    updateGUI();
                    currentFrameGroundTruth = expectedValues[videoPos-1];
                }
                int numberKey = inputKey - ZERO;
                if(numberKey >= 0 && numberKey < 10){
                    currentFrameGroundTruth = numberKey;
                    recordCurrentFrameResult();
                }
            }while(inputKey != SPACE && inputKey != ESCAPE);//space key pressed again to resume
        }
    }
//    for(int i = 0; i < 100; i++)
//        cout << expectedValues[i] << endl;
}

void VideoGroundTruthGenerator::writeResultToCSV(char* csvPath){
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
