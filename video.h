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
//�û��̳е����¼�������

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

    void initParam(int n=0);                                            //��ʼ������Done
    int videoCheck();										            //�������Done
    bool videoOpen(int n=0);										    //��ʼ�����Done
    bool videoStart(int n=0);										    //����������Done
    bool getFrame(Mat& img,int n=0);                                    //��ȡһ֡ͼƬDone
    bool videoStopStream(int n=0);									    //�Ͽ�����Done
    void videoClose(int n=0);										    //�Ͽ����Done
    bool setTrigMode(int mode = 0,int n=0);                             //ON:mode=1 OFF:mode=0 Done

    void executeSoftTrig();									            //ִ��һ������Done       
    void SetExposeTime(double m_dEditShutterValue,int n=0);				//�����ع�Done
    void SetAdjustPlus(double m_dEditGainValue,int n=0);				//��������Done
    void setBufferSize(int nSize);                                      //SJR:������д��
    void setBalanceRatio(double m_dEditBalanceRatioValue,int n=0);      //SJR:û����
    void setResolution(int height = 720, int width = 1280);             //SJR:������д��
    void setROI(int64_t nX, int64_t nY, int64_t nWidth, int64_t nHeight);//SJR:������д��
    void setBinning();                                                  //SJR:������д��
    bool loadSetting(int mode);                                         //SJR:������д��


    void setFrameRate(double rate = 210);					            //����֡��//SJR:������д��
    int  deviceNum = 0;                                                 //�������

private:
    bool bIsDeviceOpen = false;         // �豸�Ƿ�򿪱�־
    bool bIsStreamOpen = false;         // �豸���Ƿ�򿪱�־

    GxIAPICPP::gxdeviceinfo_vector vectorDeviceInfo;                    //����б�
    
    ICaptureEventHandler* pCaptureEventHandler ;                        //�ɼ��ص�����

    CGXDevicePointer                  m_objDevicePtr[2];                // �豸���
    CGXStreamPointer                  m_objStreamPtr[2];                // �豸��
    CGXFeatureControlPointer          m_objFeatureControlPtr[2];        // ���Կ�����
    CGXFeatureControlPointer          m_objStreamFeatureControlPtr[2];  // �������������

    bool                              m_bIsTrigValid;                   // SJR:û���ϣ�Ӧ�õ���һ����ֹ���������ķ�����
                                                                        //�����Ƿ���Ч��־:��һ�δ�������ִ��ʱ�����ñ�־��Ϊfalse

    bool                              m_bIsOpen[2];                  // �豸�򿪱�ʶ
    bool                              m_bIsSnap[2];                  // �豸�ɼ���ʶ
    bool                              m_bColorFilter[2];             // �Ƿ�֧�ֲ�ɫ���
    bool                              m_bTriggerMode[2];             // �Ƿ�֧�ִ���ģʽ
    bool                              m_bTriggerSource[2];           // �Ƿ�֧�ִ���Դ
    bool                              m_bTriggerActive[2];           // �Ƿ�֧�ִ�������
    bool                              m_bBalanceWhiteAuto[2];        // �Ƿ�֧���Զ���ƽ��
    bool                              m_bBalanceWhiteRatioSelect[2]; // �Ƿ�֧�ְ�ƽ��ͨ��ѡ��
    double                            m_dShutterValueMax[2];         // �ع�ʱ�����ֵ      
    double                            m_dShutterValueMin[2];         // �ع�ʱ����Сֵ     
    double                            m_dGainValueMax[2];            // �������ֵ
    double                            m_dGainValueMin[2];            // ������Сֵ
    double                            m_dBalanceWhiteRatioMax[2];    // �Զ���ƽ��ϵ�����ֵ
    double                            m_dBalanceWhiteRatioMin[2];    // �Զ���ƽ��ϵ����Сֵ
    int                               m_nTriggerModeOld[2];          // ��¼����ģʽ
    int                               m_nTriggerSourceOld[2];        // ��¼����Դ
    int                               m_nTriggerActiveOld[2];        // ��¼��������
    int                               m_nBalanceWhiteAutoOld[2];     // ��¼�Զ���ƽ��
    int                               m_nBanlanceWhiteRatioOld[2];   // ��¼�Զ���ƽ��ϵ��
    gxstring                          m_strBalanceWhiteAutoMode[2];  // ��¼�Զ���ƽ�ⷽʽ 
};
