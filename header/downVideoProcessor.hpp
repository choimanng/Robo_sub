struct PathMarker;

class DownVideoProcessor: public VideoProcessor{
public:

vector<PathMarker> pathMarkers;

//functions that process the videos
void processFrame();
PathMarker findPathMarker (vector<Point> rectContour);

//gui functions for visual confirmation, mainly for debugging
string convertInt(int number);
void setLabel(Mat im, string label, Point org);
void drawPathMarkers();
void updateGUI();

};
