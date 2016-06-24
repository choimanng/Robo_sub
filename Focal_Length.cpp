    //focalLength = (perceivedWidth * Distance) / objectWidth;
    float focalLength = 534.43;

    RotatedRect minRect = minAreaRect( Mat(contours[i]) );

    perceivedWidth = minRect.size.width;
    Distance = (objectWidth * focalLength) / perceivedWidth;

    perceivedHeight = minRect.size.height;
    Distance = (objectHeight * focalLength) / perceivedHeight;
