#include "mainPC.h"
#include "video.h"

using namespace std;
using namespace cv;

mainPC::mainPC(int* setting)
{
    FileStorage storage("setting.xml", FileStorage::READ);
    ROI[0] = storage["OffSetX"].real();
    ROI[1] = storage["OffSetY"].real();
    ROI[2] = storage["Width"].real();
    ROI[3] = storage["Height"].real();
    ExposeTime = storage["ExposeTime"].real();
    AdjustPlus = storage["AdjustPlus"].real();
    BalanceRatio = storage["BalanceRatio"].real();
    FrameRate = storage["FrameRate"].real();
}

void mainPC::ImageProducer()
{
    if (!a.videoCheck()) {
        cerr << "没相机\n";
    }
    if (a.videoOpen()) {
        a.streamControl(1);
        cerr << "ok\n";
    }
}

void mainPC::ImageConsumer()
{
    Mat src;
    while (1) {
        if (!a.getFrame(src)) {
            cerr << "没\n";
        }
        else {
            cerr << "2\n";
            //imshow("0", src);
            waitKey(1);
        }
    }
    cerr<<"out"<<endl;
}
