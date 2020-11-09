#include <tchar.h>
#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2\imgproc\imgproc.hpp>
#include <opencv2\opencv.hpp>
#include "video.h"
//using namespace cv;
using namespace std;

//���û���ǰ���úù���ͷ�ļ�Ŀ¼,��Ҫ����GalaxyIncludes.h
#include"GalaxyIncludes.h"

#include "video.h"

volatile unsigned int prdIdx[2];
volatile unsigned int csmIdx[2];
struct Imagedata {//������ݽṹ���Կ��ǻ��ɶ���
	cv::Mat img, img2;
	int status = 0;
	unsigned int frame;
	unsigned int payloadsize = 0;
	unsigned int width = 0;
	unsigned int height = 0;
	unsigned int pixelformat = 0;
	unsigned int frameid = 0;
	unsigned long long int timestamp = 0;
}datadata[2];

video::video()
{
#ifndef Release
	try {
#endif
		IGXFactory::GetInstance().Init();
#ifndef Release
	}
	catch (CGalaxyException& e) {
		cout << "������: " << e.GetErrorCode() << endl;
		cout << "����������Ϣ: " << e.what() << endl;
	}
#endif
}

video::~video()
{
#ifndef Release
	//�ر��豸֮�󣬲����ٵ��������κο�ӿ�
	try {
#endif
		IGXFactory::GetInstance().Uninit();
#ifndef Release
	}
	catch (CGalaxyException& e) {
		cout << "������: " << e.GetErrorCode() << endl;
		cout << "����������Ϣ: " << e.what() << endl;
	}
#endif
	if (NULL != pCaptureEventHandler) {//�����¼��ص�ָ��
		delete pCaptureEventHandler;
		pCaptureEventHandler = NULL;
	}
}

void video::initParam(int n)
{
	bool bBalanceWhiteAutoRead = false;													//��ƽ���Ƿ�ɶ�
	m_objFeatureControlPtr[n]->GetEnumFeature("AcquisitionMode")->SetValue("Continuous");	//���òɼ�ģʽΪ�����ɼ�ģʽ
	m_bTriggerMode[n] = m_objFeatureControlPtr[n]->IsImplemented("TriggerMode");				//�Ƿ�֧�ִ���ģʽѡ��
	if (m_bTriggerMode[n]) {
		m_objFeatureControlPtr[n]->GetEnumFeature("TriggerMode")->SetValue("Off");			//���ô���ģʽ��
	}

	m_bColorFilter[n] = m_objFeatureControlPtr[n]->IsImplemented("PixelColorFilter");			//�Ƿ�֧��Bayer��ʽ
	m_bTriggerSource[n] = m_objFeatureControlPtr[n]->IsImplemented("TriggerSource");			//�Ƿ�֧�ִ���Դѡ��
	m_bTriggerActive[n] = m_objFeatureControlPtr[n]->IsImplemented("TriggerActivation");		//�Ƿ�֧�ִ�������ѡ��
	m_bBalanceWhiteAuto[n] = m_objFeatureControlPtr[n]->IsImplemented("BalanceWhiteAuto");	//�Ƿ�֧���Զ���ƽ��
	bBalanceWhiteAutoRead = m_objFeatureControlPtr[n]->IsReadable("BalanceWhiteAuto");		//��ƽ���Ƿ�ɶ�


	if (m_bBalanceWhiteAuto) {//���֧���ҿɶ������ȡ�豸��ǰ��ƽ��ģʽ
		if (bBalanceWhiteAutoRead) {
			m_strBalanceWhiteAutoMode[n] = m_objFeatureControlPtr[n]->GetEnumFeature("BalanceWhiteAuto")->GetValue();
		}
	}

	m_bBalanceWhiteRatioSelect[n] = m_objFeatureControlPtr[n]->IsImplemented("BalanceRatioSelector");//�Ƿ�֧���Զ���ƽ��ͨ��ѡ��

	//��ȡ�ع�ʱ�䡢���桢�Զ���ƽ��ϵ����ͼ���ߵ����ֵ����Сֵ
	m_dShutterValueMax[n] = m_objFeatureControlPtr[n]->GetFloatFeature("ExposureTime")->GetMax();
	m_dShutterValueMin[n] = m_objFeatureControlPtr[n]->GetFloatFeature("ExposureTime")->GetMin();
	m_dGainValueMax[n] = m_objFeatureControlPtr[n]->GetFloatFeature("Gain")->GetMax();
	m_dGainValueMin[n] = m_objFeatureControlPtr[n]->GetFloatFeature("Gain")->GetMin();
	m_dBalanceWhiteRatioMax[n] = m_objFeatureControlPtr[n]->GetFloatFeature("BalanceRatio")->GetMax();
	m_dBalanceWhiteRatioMin[n] = m_objFeatureControlPtr[n]->GetFloatFeature("BalanceRatio")->GetMin();
	m_nWidthMax[n] = m_objFeatureControlPtr[n]->GetIntFeature("WidthMax")->GetValue();
	m_nHeightMax[n] = m_objFeatureControlPtr[n]->GetIntFeature("HeightMax")->GetValue();
#ifndef Release
	cout << "m_dShutterValueMax:" << m_dShutterValueMax[n] << endl
		<< "m_dShutterValueMin:" << m_dShutterValueMin[n] << endl
		<< "m_dGainValueMax:" << m_dGainValueMax[n] << endl
		<< "m_dGainValueMin:" << m_dGainValueMin[n] << endl
		<< "m_dBalanceWhiteRatioMax:" << m_dBalanceWhiteRatioMax[n] << endl
		<< "m_dBalanceWhiteRatioMin:" << m_dBalanceWhiteRatioMin[n] << endl
		<< "m_nWidthMax:" << m_nWidthMax[n] << endl
		<< "m_nHeightMax:" << m_nHeightMax[n] << endl;
#endif // !DEBUG

}

int video::videoCheck() {//�������
	IGXFactory::GetInstance().UpdateDeviceList(1000, vectorDeviceInfo);
	//�ж�ö�ٵ����豸�Ƿ�����㣬��������򵯿���ʾ
	deviceNum = vectorDeviceInfo.size();
	cout << deviceNum << endl;
	return deviceNum <= 0 ? 0 : deviceNum;
}
bool video::videoOpen(int n) {//��ʼ�����

	bool bIsDeviceOpen = false;       ///< �豸�Ƿ��Ѵ򿪱�־
	bool bIsStreamOpen = false;       ///< �豸���Ƿ��Ѵ򿪱�־
	bool m_bSupportExposureEndEvent = false;       ///< �Ƿ�֧���ع������־
#ifndef Release
	try {
#endif
		cerr << vectorDeviceInfo[n].GetSN() << endl;
		m_objDevicePtr[n] = IGXFactory::GetInstance().OpenDeviceBySN(vectorDeviceInfo[n].GetSN(), GX_ACCESS_EXCLUSIVE);//���豸
		bIsDeviceOpen = true;
		m_objFeatureControlPtr[n] = m_objDevicePtr[n]->GetRemoteFeatureControl();//��ȡ���Կ��������� 
		uint32_t nStreamCount = m_objDevicePtr[n]->GetStreamCount();//��ȡ��ͨ������

		if (nStreamCount > 0) {//����
			m_objStreamPtr[n] = m_objDevicePtr[n]->OpenStream(0);
			m_objStreamFeatureControlPtr[n] = m_objStreamPtr[n]->GetFeatureControl();
			bIsStreamOpen = true;
		}
		else {
#ifndef Release
			throw exception("δ�����豸��!");
#else
			return false;
#endif
		}
		//��ʼ���������
		initParam(n);
		m_bIsOpen[n] = true;
#ifndef Release
	}
	catch (CGalaxyException& e) {

		if (bIsStreamOpen) {//�ж��豸���Ƿ��Ѵ�
			m_objStreamPtr[n]->Close();
		}
		if (bIsDeviceOpen) {//�ж��豸�Ƿ��Ѵ�
			m_objDevicePtr[n]->Close();
			return false;
		}
	}
	catch (std::exception& e) {

		if (bIsStreamOpen) {//�ж��豸���Ƿ��Ѵ�
			m_objStreamPtr[n]->Close();
		}
		if (bIsDeviceOpen) {//�ж��豸�Ƿ��Ѵ�
			m_objDevicePtr[n]->Close();
		}
		return false;
	}
#endif
	return true;
}

bool video::videoStart(int n) {//����������
	pCaptureEventHandler = new CSampleCaptureEventHandler(n);
	m_objStreamPtr[n]->RegisterCaptureCallback(pCaptureEventHandler, NULL);
	m_objStreamPtr[n]->StartGrab();//���Ϳ�������
	m_objFeatureControlPtr[n]->GetCommandFeature("AcquisitionStart")->Execute();
	return true;
}

bool video::getFrame(cv::Mat& img,int n) {//��ȡһ֡ͼƬ
	int framenum;
	while (0 == prdIdx[n-1] - csmIdx[n - 1]);
	datadata[csmIdx[n - 1] % 1].img.copyTo(img);
	printf("%d %llu  %llu  %llu\n", n, datadata[csmIdx[n - 1] % 1].frame,
		datadata[csmIdx[n - 1] % 1].frameid, datadata[csmIdx[n - 1] % 1].timestamp);
	framenum = datadata[csmIdx[n - 1] % 1].frame;
	++(csmIdx[n - 1]);
	if (datadata[csmIdx[n - 1] % 1].status==-1 || img.empty() || img.channels() != 3 ) {
		return false;
	}
	return true;
}

bool video::videoStopStream(int n) {//�Ͽ�����
	m_objFeatureControlPtr[n]->GetCommandFeature("AcquisitionStop")->Execute();
	m_objStreamPtr[n]->StopGrab();//����ͣ������
	m_objStreamPtr[n]->UnregisterCaptureCallback();//ע���ɼ��ص�
	return true;
}
void video::videoClose(int n) {//�Ͽ����
	//ע��Զ���豸�¼�
	m_objFeatureControlPtr[n]->UnregisterFeatureCallback(NULL);
	//ע���豸�����¼�
	//ObjDevicePtr->UnregisterDeviceOfflineCallback(hDeviceOffline);

	//�ͷ���Դ
	m_objStreamPtr[n]->Close();
	m_objDevicePtr[n]->Close();
}

bool video::setTrigMode(int mode,int n)
{
#ifndef Release
	try {
#endif
		m_bTriggerMode[n] = m_objFeatureControlPtr[n]->IsImplemented("TriggerMode");//�Ƿ�֧�ִ���ģʽѡ��
		if (m_bTriggerMode[n]) {
			m_objFeatureControlPtr[n]->GetEnumFeature("TriggerMode")->SetValue("Off");//���ô���ģʽ��
		}
		else {
			//m_objFeatureControlPtr[n]->GetEnumFeature("TriggerSelector")->SetValue("FrameStart");
			if (1 == mode) {
				m_objFeatureControlPtr[n]->GetEnumFeature("TriggerMode")->SetValue("On");
				m_objFeatureControlPtr[n]->GetEnumFeature("TriggerSource")->SetValue("Software");
			}
			else {
				m_objFeatureControlPtr[n]->GetEnumFeature("TriggerMode")->SetValue("Off");
			}
		}
		return true;
#ifndef Release
	}
	catch (CGalaxyException& e) {
		cerr << e.what() << endl;
		return false;
	}
	catch (std::exception& e) {
		cerr << e.what() << endl;
		return false;
	}
#endif
	return false;
}

void video::executeSoftTrig() {
#ifndef Release
	try {//������������(�ڴ���ģʽ����ʱ��Ч)
#endif
		for(int i=0;i<deviceNum;++i)
			m_objFeatureControlPtr[i]->GetCommandFeature("TriggerSoftware")->Execute();
#ifndef Release
	}
	catch (CGalaxyException& e) {
		cout << e.what() << endl;
		return;
	}
	catch (std::exception& e) {
		cout << e.what() << endl;
		return;
	}
#endif
}

void video::SetExposeTime(double m_dEditShutterValue,int n) {//�����ع�
	m_objFeatureControlPtr[n]->GetFloatFeature("ExposureTime")->SetValue(m_dEditShutterValue);
	if (!m_bIsOpen) {
		return;
	}
	double dShutterValueOld = m_dEditShutterValue;
#ifndef Release
	try {//�ж�����ֵ�Ƿ����ع�ʱ�䷶Χ�ڣ����������������������ı߽�ֵ
#endif
		if (m_dEditShutterValue > m_dShutterValueMax[n]) {
			m_dEditShutterValue = m_dShutterValueMax[n];
		}
		if (m_dEditShutterValue < m_dShutterValueMin[n]) {
			m_dEditShutterValue = m_dShutterValueMin[n];
		}
		m_objFeatureControlPtr[n]->GetFloatFeature("ExposureTime")->SetValue(m_dEditShutterValue);
#ifndef Release
	}
	catch (CGalaxyException & e)
	{
		m_dEditShutterValue = dShutterValueOld;
		cerr << e.what() << endl;
	}
	catch (std::exception & e)
	{
		m_dEditShutterValue = dShutterValueOld;
		cerr << e.what() << endl;
	}
#endif
}							
void video::SetAdjustPlus(double m_dEditGainValue,int n) {//��������
	if (!m_bIsOpen){
		return;
	}
	double dGainValueOld = m_dEditGainValue;
#ifndef Release
	try{
#endif
		if (m_dEditGainValue == -1) {
			m_objFeatureControlPtr[n]->GetEnumFeature("GainAuto")->SetValue("Continuous");
		}
		else {
			m_objFeatureControlPtr[n]->GetEnumFeature("GainAuto")->SetValue("Off");
			//�ж�����ֵ�Ƿ�������ֵ��Χ�ڣ����������������������ı߽�ֵ
			if (m_dEditGainValue > m_dGainValueMax[n]) {
				m_dEditGainValue = m_dGainValueMax[n];
			}
			if (m_dEditGainValue < m_dGainValueMin[n]) {
				m_dEditGainValue = m_dGainValueMin[n];
			}
			m_objFeatureControlPtr[n]->GetFloatFeature("Gain")->SetValue(m_dEditGainValue);
		}
#ifndef Release
	}
	catch (CGalaxyException & e){
		m_dEditGainValue = dGainValueOld;
	}
	catch (std::exception & e){
		m_dEditGainValue = dGainValueOld;
	}
#endif
}
void video::setBalanceRatio(double m_dEditBalanceRatioValue,int n) {
	if (!m_bIsOpen[n]){
		return;
	}
	double dBalanceWhiteRatioOld = m_dEditBalanceRatioValue;
#ifndef Release
	try{
#endif
		if (m_dEditBalanceRatioValue == -1) { //�Զ���ƽ��
			m_objFeatureControlPtr[n]->GetEnumFeature("BalanceWhiteAuto")->SetValue("Continuous");
		}
		else {
			m_objFeatureControlPtr[n]->GetEnumFeature("BalanceWhiteAuto")->SetValue("Off");
			//�ж�����ֵ�Ƿ����Զ���ƽ��ϵ����Χ�ڣ����������������������ı߽�ֵ
			if (m_dEditBalanceRatioValue > m_dBalanceWhiteRatioMax[n]) {
				m_dEditBalanceRatioValue = m_dBalanceWhiteRatioMax[n];
			}
			if ((m_dEditBalanceRatioValue < m_dBalanceWhiteRatioMin[n])) {
				m_dEditBalanceRatioValue = m_dBalanceWhiteRatioMin[n];
			}
			m_objFeatureControlPtr[n]->GetFloatFeature("BalanceRatio")->SetValue(m_dEditBalanceRatioValue);
		}
#ifndef Release
	}
	catch (CGalaxyException & e){
		m_dEditBalanceRatioValue = dBalanceWhiteRatioOld;
		cout << e.what() << endl;
	}
	catch (std::exception & e){
		m_dEditBalanceRatioValue = dBalanceWhiteRatioOld;
		cout << e.what() << endl;
	}
#endif
}
void video::setROI(int64_t nX, int64_t nY, int64_t nWidth, int64_t nHeight, int n){
	if (!m_bIsOpen[n]) {
		return;
	}
#ifndef Release
	try {
#endif
		if (nWidth > m_nWidthMax[n]) { nWidth = m_nWidthMax[n]; }
		if (nHeight > m_nHeightMax[n]) { nHeight = m_nHeightMax[n]; }
		if (nWidth + nX > m_nWidthMax[n]) { //ƫ�����������ֵ
			nX = m_nWidthMax[n] - nWidth;
			nX = nX - nX % 16;
		}
		if (nHeight + nY > m_nHeightMax[n]) {
			nY = m_nHeightMax[n] - nHeight;
			nY = nY - nY % 16;
		}
		m_objFeatureControlPtr[n]->GetIntFeature("OffsetX")->SetValue(nX);
		m_objFeatureControlPtr[n]->GetIntFeature("OffsetY")->SetValue(nY);
		m_objFeatureControlPtr[n]->GetIntFeature("Width")->SetValue(nWidth);
		m_objFeatureControlPtr[n]->GetIntFeature("Height")->SetValue(nHeight);
#ifndef Release
	}
	catch (CGalaxyException& e) {
		cout << e.what() << endl;
	}
	catch (std::exception& e) {
		cout << e.what() << endl;
	}
#endif
}

void video::setFrameRate(double rate, int n) {
	if (!m_bIsOpen[n]) {
		return;
	}
#ifndef Release
	try {
#endif
		m_objFeatureControlPtr[n]->GetFloatFeature("AcquisitionFrameRate")->SetValue(rate);
#ifndef Release
	}
	catch (CGalaxyException& e) {
		cout << e.what() << endl;
	}
	catch (std::exception& e) {
		cout << e.what() << endl;
	}
#endif
}		

CSampleCaptureEventHandler::CSampleCaptureEventHandler(int n1) {
	n = n1;
}

void CSampleCaptureEventHandler::DoOnImageCaptured(CImageDataPointer& objImageDataPointer, void* pUserParam) {
	while (prdIdx[n - 1] - csmIdx[n - 1] >= 1);
	void* pRGB24Buffer = NULL;
	pRGB24Buffer = objImageDataPointer->ConvertToRGB24(GX_BIT_0_7, GX_RAW2RGB_NEIGHBOUR, true);
	Mat test;
	test.create(objImageDataPointer->GetHeight(), objImageDataPointer->GetWidth(), CV_8UC3);
	memcpy(test.data, pRGB24Buffer, objImageDataPointer->GetPayloadSize() * 3);
	flip(test, test, 0);
	datadata[prdIdx[n - 1] % 1].img = test;
	datadata[prdIdx[n - 1] % 1].frame++;
	datadata[prdIdx[n - 1] % 1].status = objImageDataPointer->GetStatus();
	/*datadata[prdIdx[n-1] % 1].payloadsize = objImageDataPointer->GetPayloadSize();
	datadata[prdIdx[n-1] % 1].width = objImageDataPointer->GetWidth();
	datadata[prdIdx[n-1] % 1].height = objImageDataPointer->GetHeight();
	datadata[prdIdx[n-1] % 1].pixelformat = objImageDataPointer->GetPixelFormat();*/
	datadata[prdIdx[n - 1] % 1].frameid = objImageDataPointer->GetFrameID();
	datadata[prdIdx[n - 1] % 1].timestamp = objImageDataPointer->GetTimeStamp();

	++(prdIdx[n - 1]);
}
