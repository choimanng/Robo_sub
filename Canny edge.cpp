#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <stdlib.h>
#include <stdio.h>
using namespace cv;

Mat src, src_gray;
Mat dst, detected_edges;
int edgeThresh = 1;
int lowThreshold;
int ratio = 3;
int kernel_size = 3;

void CannyThreshold(int, void*)
{
  /// Reduce noise with a kernel 3x3
  blur( src_gray, detected_edges, Size(3,3) );
  /// Canny detector
  Canny( detected_edges, detected_edges, lowThreshold, lowThreshold*ratio, kernel_size );
  /// Using Canny's output as a mask, we display our result
  src.copyTo( dst, detected_edges);
 }

int main( int argc, char** argv )
{
    namedWindow( "Edge Map", CV_WINDOW_AUTOSIZE );
    namedWindow( "Org", CV_WINDOW_AUTOSIZE );
    moveWindow("Org", 50, 100);
    moveWindow("Edge Map", 700, 100);

    VideoCapture cap =  VideoCapture("../../sample videos/Pipe.avi");     //input video file or from camera
    if(!cap.isOpened()) {return -1;}
    while(true)
    {
        cap >> src;
        //src = imread( "D/1255.jpg" );
        if( !src.data ) { return -1; }
        cvtColor( src, src_gray, CV_BGR2GRAY );
        dst.create( src.size(), src.type() );
        dst = Scalar::all(0);
        lowThreshold=100;
        CannyThreshold(lowThreshold, 0);
        imshow( "Edge Map", dst );
        imshow("Org", src);
        waitKey(500);
    }
    waitKey(0);
  return 0;
}
