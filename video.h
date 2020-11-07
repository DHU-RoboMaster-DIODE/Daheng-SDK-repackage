#pragma once

#include "GalaxyIncludes.h"
#include "GXBitmap.h"

#include <opencv2/highgui.hpp>
#include <opencv2/core/core.hpp>  
#include <opencv2/imgproc/imgproc.hpp>  
#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>  
#include <cstdio>

using namespace std;
using cv::Mat;
using cv::namedWindow;
using cv::imshow;
using cv::waitKey;

using namespace std;
//用户继承掉线事件处理类

class CSampleCaptureEventHandler : public ICaptureEventHandler
{
public:
    CSampleCaptureEventHandler(int n1);
    void DoOnImageCaptured(CImageDataPointer& objImageDataPointer, void* pUserParam);
private:
    int n;
};
class video
{
public:
    video();
    ~video();

    void initParam(int n=0);                                            //初始化参数Done
    int videoCheck();										            //搜索相机Done
    bool videoOpen(int n=0);										    //初始化相机Done
    bool videoStart(int n=0);										    //创建流对象Done
    bool getFrame(Mat& img,int n=0);                                    //获取一帧图片Done
    bool videoStopStream(int n=0);									    //断开拉流Done
    void videoClose(int n=0);										    //断开相机Done
    bool setTrigMode(int mode = 0,int n=0);                             //ON:mode=1 OFF:mode=0 Done

    void executeSoftTrig();									            //执行一次软触发Done       
    void SetExposeTime(double m_dEditShutterValue,int n=0);				//设置曝光Done
    void SetAdjustPlus(double m_dEditGainValue,int n=0);				//设置增益Done
    void setBufferSize(int nSize);                                      //SJR:不是我写的
    void setBalanceRatio(double m_dEditBalanceRatioValue,int n=0);      //SJR:没测试
    void setResolution(int height = 720, int width = 1280);             //SJR:不是我写的
    void setROI(int64_t nX, int64_t nY, int64_t nWidth, int64_t nHeight);//SJR:不是我写的
    void setBinning();                                                  //SJR:不是我写的
    bool loadSetting(int mode);                                         //SJR:不是我写的


    void setFrameRate(double rate = 210);					            //设置帧率//SJR:不是我写的
    int  deviceNum = 0;                                                 //相机个数

private:
    bool bIsDeviceOpen = false;         // 设备是否打开标志
    bool bIsStreamOpen = false;         // 设备流是否打开标志

    GxIAPICPP::gxdeviceinfo_vector vectorDeviceInfo;                    //相机列表
    
    ICaptureEventHandler* pCaptureEventHandler ;                        //采集回调对象

    CGXDevicePointer                  m_objDevicePtr[2];                // 设备句柄
    CGXStreamPointer                  m_objStreamPtr[2];                // 设备流
    CGXFeatureControlPointer          m_objFeatureControlPtr[2];        // 属性控制器
    CGXFeatureControlPointer          m_objStreamFeatureControlPtr[2];  // 流层控制器对象

    bool                              m_bIsTrigValid;                   // SJR:没用上，应该得有一个防止连续软触发的方法。
                                                                        //触发是否有效标志:当一次触发正在执行时，将该标志置为false

    bool                              m_bIsOpen[2];                  // 设备打开标识
    bool                              m_bIsSnap[2];                  // 设备采集标识
    bool                              m_bColorFilter[2];             // 是否支持彩色相机
    bool                              m_bTriggerMode[2];             // 是否支持触发模式
    bool                              m_bTriggerSource[2];           // 是否支持触发源
    bool                              m_bTriggerActive[2];           // 是否支持触发极性
    bool                              m_bBalanceWhiteAuto[2];        // 是否支持自动白平衡
    bool                              m_bBalanceWhiteRatioSelect[2]; // 是否支持白平衡通道选择
    double                            m_dShutterValueMax[2];         // 曝光时间最大值      
    double                            m_dShutterValueMin[2];         // 曝光时间最小值     
    double                            m_dGainValueMax[2];            // 增益最大值
    double                            m_dGainValueMin[2];            // 增益最小值
    double                            m_dBalanceWhiteRatioMax[2];    // 自动白平衡系数最大值
    double                            m_dBalanceWhiteRatioMin[2];    // 自动白平衡系数最小值
    int                               m_nTriggerModeOld[2];          // 记录触发模式
    int                               m_nTriggerSourceOld[2];        // 记录触发源
    int                               m_nTriggerActiveOld[2];        // 记录触发极性
    int                               m_nBalanceWhiteAutoOld[2];     // 记录自动白平衡
    int                               m_nBanlanceWhiteRatioOld[2];   // 记录自动白平衡系数
    gxstring                          m_strBalanceWhiteAutoMode[2];  // 记录自动白平衡方式 
};
