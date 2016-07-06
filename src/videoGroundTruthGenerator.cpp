#include "../header/videoGroundTruthGenerator.hpp"


using namespace cv;
using namespace std;

bool isNumberKey(int key){
    return 0 <= key-ZERO && key-ZERO <=9;
}

VideoGroundTruthGenerator::VideoGroundTruthGenerator(string videoFilePath){
    cap =  VideoCapture(videoFilePath);     //input video file or from camera
    if(!cap.isOpened()){
        cout << "couldn't load video. check path?";
    }
}

void VideoGroundTruthGenerator::recordCurrentFrameResult(){
    //is minus 1 because videoPos has to be updated in function updateGUI
    expectedValues[getCurrentFrame()] = currentFrameGroundTruth;
}

int VideoGroundTruthGenerator::getCurrentFrame(){
    //(int)cap.get(CV_CAP_PROP_POS_FRAMES) always return the next frame to be decoded
    return (int)cap.get(CV_CAP_PROP_POS_FRAMES)-1;
}

//debugging or testing functions
void VideoGroundTruthGenerator::generateGUI(){
    //create the windows
    namedWindow( "Original", WINDOW_NORMAL );
    namedWindow( "Playback", WINDOW_NORMAL );
    createTrackbar("Frame", "Playback", &videoPos, cap.get(CV_CAP_PROP_FRAME_COUNT));
    createTrackbar("waitKeyTime", "Playback", &waitKeyTime, 1000);

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
    setTrackbarPos("Frame", "Playback", getCurrentFrame());
}

void VideoGroundTruthGenerator::processVideo(){
    //restart video no matter what
    cap.set(CV_CAP_PROP_POS_FRAMES,0);
    generateGUI();
    //reset result array
    expectedValues = new int[(int)cap.get(CV_CAP_PROP_FRAME_COUNT)];
    cap.read(unprocessedFrame);
    updateGUI();
    int inputKey;
    //in beginning, wait until user enter a number key designating current target count
    do{
        inputKey = waitKey();
    }while(!isNumberKey(inputKey));
    currentFrameGroundTruth = inputKey-ZERO;
    recordCurrentFrameResult();
    updateGUI();

    //read video until eof
    while(inputKey!=ESCAPE && cap.read(unprocessedFrame)){
        updateGUI();
        recordCurrentFrameResult();
        inputKey = waitKey(waitKeyTime);
        if(inputKey == SPACE){
            do{
                inputKey = waitKey();
                if(isNumberKey(inputKey)){
                    currentFrameGroundTruth = inputKey-ZERO;
                    recordCurrentFrameResult();
                    updateGUI(); //in order to show the updated target count
                }
                else if(inputKey == RIGHT){//forward 1 frame
                    cap.read(unprocessedFrame);
                    currentFrameGroundTruth = expectedValues[getCurrentFrame()];
                    updateGUI();
                }
                else if(inputKey == LEFT){//backward 1 frame
                    cap.set(CV_CAP_PROP_POS_FRAMES, getCurrentFrame()-1);
                    cap.read(unprocessedFrame);
                    currentFrameGroundTruth = expectedValues[getCurrentFrame()];
                    updateGUI();
                }
                else if(inputKey == SHIFT_RIGHT){//forward 5 frames
                    cap.set(CV_CAP_PROP_POS_FRAMES, getCurrentFrame()+5);
                    cap.read(unprocessedFrame);
                    currentFrameGroundTruth = expectedValues[getCurrentFrame()];
                    updateGUI();
                }
                else if(inputKey == SHIFT_LEFT){//backward 5 frames
                    cap.set(CV_CAP_PROP_POS_FRAMES, getCurrentFrame()-5);
                    cap.read(unprocessedFrame);
                    currentFrameGroundTruth = expectedValues[getCurrentFrame()];
                    updateGUI();
                }
                else if(inputKey == CTRL_RIGHT){//forward 10 frames
                    cap.set(CV_CAP_PROP_POS_FRAMES, getCurrentFrame()+10);
                    cap.read(unprocessedFrame);
                    currentFrameGroundTruth = expectedValues[getCurrentFrame()];
                    updateGUI();
                }
                else if(inputKey == CTRL_LEFT){//backward 10 frames
                    cap.set(CV_CAP_PROP_POS_FRAMES, getCurrentFrame()-10);
                    cap.read(unprocessedFrame);
                    currentFrameGroundTruth = expectedValues[getCurrentFrame()];
                    updateGUI();
                }
                else if(inputKey == CTRL_SHIFT_RIGHT){//forward 30 frames
                    cap.set(CV_CAP_PROP_POS_FRAMES, getCurrentFrame()+30);
                    cap.read(unprocessedFrame);
                    currentFrameGroundTruth = expectedValues[getCurrentFrame()];
                    updateGUI();
                }
                else if(inputKey == CTRL_SHIFT_LEFT){//backward 30 frames
                    cap.set(CV_CAP_PROP_POS_FRAMES, getCurrentFrame()-30);
                    cap.read(unprocessedFrame);
                    currentFrameGroundTruth = expectedValues[getCurrentFrame()];
                    updateGUI();
                }
            }while(inputKey!=SPACE && inputKey!=ESCAPE);
        }
        else if(isNumberKey(inputKey)){
            currentFrameGroundTruth = inputKey-ZERO;
            recordCurrentFrameResult();
        }
    }
}

void VideoGroundTruthGenerator::writeResultToCSV(char* csvPath){
    //open output file
    ofstream csvResultFile;
    csvResultFile.open(csvPath, ios_base::app);
    if(!csvResultFile.is_open()){
        cout << "cannot open csv file. check file path?" << endl;
        return;
    }

    //write setting to csv file
    csvResultFile << endl << "ground truth,n/a,n/a,n/a,n/a";

    //write individual frame's target count in this processing
    int frameCount = (int)cap.get(CV_CAP_PROP_FRAME_COUNT);
    for (int j=0; j<frameCount; j++){
        csvResultFile << "," << expectedValues[j];
        cout << expectedValues[j] << endl;;
    }
    csvResultFile.close();
}
