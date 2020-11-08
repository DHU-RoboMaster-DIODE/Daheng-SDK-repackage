#include "mainPC.h"
#include "video.h"

using cv::FileStorage;
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
    if (a.deviceNum >= 2) {//有两个相机的话，就用软触发连续抓同一时刻的图
        for (int i = 0; i < a.deviceNum; ++i) {
            if (a.videoOpen(i)) {
                a.setTrigMode(1, i);
                a.videoStart(i);
                a.executeSoftTrig();
                a.SetExposeTime(10000);
            }
        }
    }
    else {//一个相机就普普通通连续拉流
        if (a.videoOpen()) {
            a.setROI(ROI[0], ROI[1], ROI[2], ROI[3]);
            a.videoStart();
            a.setTrigMode();
            a.SetAdjustPlus(AdjustPlus);
            a.setBalanceRatio(BalanceRatio);
            a.setFrameRate(FrameRate);//最大帧率
            a.SetExposeTime(ExposeTime);
        }
    }
    
}

void mainPC::ImageConsumer()
{
    Mat src;
    int i = 0;
    while (1) {
        string winname[2] = { "1","2" };
        for (int i = 0; i < a.deviceNum; ++i) {
            if (!a.getFrame(src, i)) {
                break;
            }
            else {
                imshow(winname[i], src);
                waitKey(1);
            }
        }
    }
}
