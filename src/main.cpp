#include "../header/downVideoProcessor.hpp"
#include "../header/videoGroundTruthGenerator.hpp"
#include "../header/videoProcessor.hpp"

using namespace cv;
using namespace std;

//int main(int argc, char *argv[]){
//    cout << "unit test begin running" << endl;
//    DownVideoProcessor v("/home/mong/Desktop/robosub/opencv/sample videos/Pipe32x.avi");
//    v.loadExpectedValueFromCSV("/home/mong/Desktop/robosub/opencv/actual code/Robo_sub/result/000Pipes_Result_Final32x.csv");
//    v.processVideoDebug();
//}
vector<string> input;

static const char *filters[] = {
    "*.jpg", "*.jpeg", "*.gif", "*.png"
};

static int callback(const char *fpath, const struct stat *sb, int typeflag) {
    /* if it's a file */
    if (typeflag == FTW_F) {
        int i;
        /* for each filter, */
        for (i = 0; i < sizeof(filters) / sizeof(filters[0]); i++) {
            /* if the filename matches the filter, */
            if (fnmatch(filters[i], fpath, FNM_CASEFOLD) == 0) {
                /* do something */
                printf("found image: %s\n", fpath);
                input.push_back(fpath);
                break;
            }
        }
    }

    /* tell ftw to continue */
    return 0;
}

int main() {
    cout << "beging" << endl;
    ftw("/home/user/Desktop/mong/input", callback, 16);
    cout << input.size() << endl;

    DownVideoProcessor v(input);

}
