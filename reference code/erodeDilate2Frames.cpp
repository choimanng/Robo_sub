void erodeDilate2Frames()
{
    Mat binaryFrame1;
    Mat binaryFrame2;
    Mat overlapFrame;
    Mat image_roi;
    int a =20;

    //erode & dilate the outer binaryframe 1
    erodeKernel = getStructuringElement( MORPH_RECT, Size(11 , 11));    // 11*11   20*20   30*30
    dilateKernel = erodeKernel;
    binaryFrame.copyTo(binaryFrame1);
    erode(binaryFrame1, binaryFrame1, erodeKernel);
    dilate(binaryFrame1, binaryFrame1, dilateKernel);

    //erode & dilate the inner binaryframe 2
    erodeKernel = getStructuringElement( MORPH_RECT, Size(a , a));    // 11*11   20*20   30*30
    dilateKernel = erodeKernel;
    binaryFrame.copyTo(binaryFrame2);
    erode(binaryFrame2, binaryFrame2, erodeKernel);
    dilate(binaryFrame2, binaryFrame2, dilateKernel);

    //Set up the center subRect and 4 sides regions
    Rect roiO(a, a, binaryFrame.cols-2*a, binaryFrame.rows-2*a);
    Rect roiA (0, 0, binaryFrame.cols, a);     // shape of roi
    Rect roiB (0, 0, a, binaryFrame.rows);     // shape of roi
    Rect roiC (0, binaryFrame.rows- a,binaryFrame.cols , a);     // shape of roi
    Rect roiD (binaryFrame.cols - a, 0, a, binaryFrame.rows);     // shape of roi

    //Set the regions to be white for future overlap of 2 binary frames
    image_roi = binaryFrame1(roiO);
    image_roi= Scalar(256);
    image_roi = binaryFrame2(roiA);
    image_roi= Scalar(256);
    image_roi = binaryFrame2(roiB);
    image_roi= Scalar(256);
    image_roi = binaryFrame2(roiC);
    image_roi= Scalar(256);
    image_roi = binaryFrame2(roiD);
    image_roi= Scalar(256);

    //Overlap the 2 binary frames to become one
    overlapFrame = binaryFrame1 & binaryFrame2;
    binaryFrame = overlapFrame;
}


