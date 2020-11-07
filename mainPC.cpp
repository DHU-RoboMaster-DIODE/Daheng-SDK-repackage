#include "mainPC.h"
#include "video.h"

mainPC::mainPC(int* setting)
{
    
}

void mainPC::ImageProducer()
{
    
    if (!a.videoCheck()) {
        cerr << "û���\n";
    }
    if (a.deviceNum >= 2) {//����������Ļ�����������������ץͬһʱ�̵�ͼ
        for (int i = 0; i < a.deviceNum; ++i) {
            if (a.videoOpen(i)) {
                a.setTrigMode(1, i);
                a.videoStart(i);
                a.executeSoftTrig();
                a.SetExposeTime(10000);
            }
        }
    }
    else {//һ�����������ͨͨ��������
        if (a.videoOpen()) {
            a.videoStart();
            a.setTrigMode();
            a.SetExposeTime(10000);
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
                waitKey(0);
            }
        }
    }
}