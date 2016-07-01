#include "../header/downVideoProcessor.hpp"

using namespace std;
using namespace cv;

void setLabel(Mat im, string label, Point org){
    int fontface = FONT_HERSHEY_SIMPLEX;
    double scale = 0.4;
    int thickness = 1;
    int baseline = 0;
    Size text = getTextSize(label, fontface, scale, thickness, &baseline);
    rectangle(im, org + Point(0, baseline), org + Point(text.width, -text.height), CV_RGB(0,0,0), CV_FILLED);
    putText(im, label, org, fontface, scale, CV_RGB(255,255,255), thickness, 8);
}

string convertInt(int number){
    stringstream ss;//create a stringstream
    ss << number;//add number to the stream
    return ss.str();//return a string with the contents of the stream
}

DownVideoProcessor::DownVideoProcessor(string videoFilePath):VideoProcessor(videoFilePath){
}

void DownVideoProcessor::processFrame(){
    VideoProcessor::processFrame();
    pathMarkers.clear();
    for(int i = 0; i < contours.size(); i++)
    {
        vector<Point> pathMarker(4);
        PathMarker pathMarkerSpecs = findPathMarker(contours[i]);
        pathMarkers.push_back(pathMarkerSpecs);
    }
}

PathMarker DownVideoProcessor::findPathMarker (vector<Point> rectContour){
    //get the moments and centers of the contour
    Moments mu = moments( rectContour, false );
    Point2f cXY = Point2f(mu.m10/mu.m00, mu.m01/mu.m00);
//    cXY = Point2f(cXY.x / resizeRatio, cXY.y / resizeRatio);
    cXY = Point2f(cXY.x , cXY.y );

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
    float rectWidth = max(minRect.size.width,minRect.size.height);// / resizeRatio ;
    float rectHeight = min(minRect.size.width,minRect.size.height);// / resizeRatio ;

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
    float dst = max(unprocessedFrame.cols,unprocessedFrame.rows);
    for (int j=0;j<4;j++)
    {
        if ( (rect_points[j].x - 0) < dst )
            dst =rect_points[j].x - 0;
        if ( (rect_points[j].y - 0) < dst )
            dst =rect_points[j].y - 0;
        if ( (unprocessedFrame.cols - rect_points[j].x) < dst )
            dst =unprocessedFrame.cols - rect_points[j].x;
        if ( (unprocessedFrame.rows - rect_points[j].y) < dst )
            dst =unprocessedFrame.rows - rect_points[j].y;
    }

    //put all specs to a "rectSpecs" data structure
    PathMarker my_pathMarker;
    my_pathMarker.contour = rectContours;
    my_pathMarker.width = rectWidth;
    my_pathMarker.height = rectHeight;
    my_pathMarker.area = contourArea(rectContours[0],false);
    my_pathMarker.angle = rectAngle;
    my_pathMarker.distance = dst;
    my_pathMarker.pXY = pXY;
    my_pathMarker.cXY = cXY;

    //set to ignore the contour as a pathMarker
    //if it's not near by the frame & the area of the contour is small
//    if (dst > ignoreRectDist && my_rect.area < ignoreRectArea)
//        my_rect.ignore = true;
//    else
//        my_rect.ignore = false;

    //return the rectSpecs
    return my_pathMarker;
}

void DownVideoProcessor::drawPathMarkers(){
    for(int i = 0; i < pathMarkers.size(); i++)
    {
        //draw the rotated rectangle
        drawContours(unprocessedFrame, pathMarkers[i].contour, -1, Scalar(0,0,255), 2);
        //draw a small dot on the center
        circle(unprocessedFrame, pathMarkers[i].cXY, 7, Scalar(0, 0, 255), -1);
        //draw the horizontal line & angle line
        line(unprocessedFrame, pathMarkers[i].pXY, pathMarkers[i].cXY, Scalar(0, 0, 255), 2, 8 );
        line(unprocessedFrame, pathMarkers[i].cXY, Point2f(pathMarkers[i].cXY.x + 100,pathMarkers[i].cXY.y), Scalar(0, 0, 255), 2, 8 );
        //note the center coordinates, width, height & angle of the rectangle
        int textPosX = pathMarkers[i].cXY.x+10;// * resizeRatio + 10;
        int textPosY = pathMarkers[i].cXY.y+20;// * resizeRatio + 20;
        setLabel(unprocessedFrame, "Center: (" + convertInt(pathMarkers[i].cXY.x) + " , " + convertInt(pathMarkers[i].cXY.y) + ")", Point(textPosX,textPosY));
        setLabel(unprocessedFrame, "Width: " + convertInt(pathMarkers[i].width) + " , Height: " + convertInt(pathMarkers[i].height) , Point(textPosX,textPosY+20));
        setLabel(unprocessedFrame, "Angle (horizontal): " + convertInt(pathMarkers[i].angle) + " degrees", Point(textPosX,textPosY+40));
        setLabel(unprocessedFrame, "Angle (to turn): " + convertInt(90-pathMarkers[i].angle) + " degrees", Point(textPosX,textPosY+60));
        setLabel(unprocessedFrame, "Area: " + convertInt(pathMarkers[i].area), Point(textPosX,textPosY+80));
        setLabel(unprocessedFrame, "Min distance to frame: " + convertInt(pathMarkers[i].distance), Point(textPosX,textPosY+100));
    }
}

void DownVideoProcessor::updateGUI(){
    drawPathMarkers();
    VideoProcessor::updateGUI();
}
