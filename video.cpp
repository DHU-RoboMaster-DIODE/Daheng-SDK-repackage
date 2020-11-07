#include <tchar.h>
#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2\imgproc\imgproc.hpp>
#include <opencv2\opencv.hpp>
#include "video.h"
//using namespace cv;
using namespace std;

//请用户提前配置好工程头文件目录,需要包含GalaxyIncludes.h
#include"GalaxyIncludes.h"

#include "video.h"

volatile unsigned int prdIdx[2];
volatile unsigned int csmIdx[2];
struct Imagedata {//这个数据结构可以考虑换成队列
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
	try {
		IGXFactory::GetInstance().Init();
	}
	catch (CGalaxyException& e) {
		cout << "错误码: " << e.GetErrorCode() << endl;
		cout << "错误描述信息: " << e.what() << endl;
	}
}

video::~video()
{
	//关闭设备之后，不能再调用其他任何库接口
	try {
		IGXFactory::GetInstance().Uninit();
	}
	catch (CGalaxyException& e) {
		cout << "错误码: " << e.GetErrorCode() << endl;
		cout << "错误描述信息: " << e.what() << endl;
	}
	if (NULL != pCaptureEventHandler) {//销毁事件回调指针
		delete pCaptureEventHandler;
		pCaptureEventHandler = NULL;
	}
}

void video::initParam(int n)
{
	bool bBalanceWhiteAutoRead = false;													//白平衡是否可读
	m_objFeatureControlPtr[n]->GetEnumFeature("AcquisitionMode")->SetValue("Continuous");	//设置采集模式为连续采集模式
	m_bTriggerMode[n] = m_objFeatureControlPtr[n]->IsImplemented("TriggerMode");				//是否支持触发模式选择
	if (m_bTriggerMode[n]) {
		m_objFeatureControlPtr[n]->GetEnumFeature("TriggerMode")->SetValue("Off");			//设置触发模式关
	}

	m_bColorFilter[n] = m_objFeatureControlPtr[n]->IsImplemented("PixelColorFilter");			//是否支持Bayer格式
	m_bTriggerSource[n] = m_objFeatureControlPtr[n]->IsImplemented("TriggerSource");			//是否支持触发源选择
	m_bTriggerActive[n] = m_objFeatureControlPtr[n]->IsImplemented("TriggerActivation");		//是否支持触发极性选择
	m_bBalanceWhiteAuto[n] = m_objFeatureControlPtr[n]->IsImplemented("BalanceWhiteAuto");	//是否支持自动白平衡
	bBalanceWhiteAutoRead = m_objFeatureControlPtr[n]->IsReadable("BalanceWhiteAuto");		//白平衡是否可读


	if (m_bBalanceWhiteAuto) {//如果支持且可读，则获取设备当前白平衡模式
		if (bBalanceWhiteAutoRead) {
			m_strBalanceWhiteAutoMode[n] = m_objFeatureControlPtr[n]->GetEnumFeature("BalanceWhiteAuto")->GetValue();
		}
	}

	m_bBalanceWhiteRatioSelect[n] = m_objFeatureControlPtr[n]->IsImplemented("BalanceRatioSelector");//是否支持自动白平衡通道选择

	//获取曝光时间、增益及自动白平衡系数的最大值和最小值
	m_dShutterValueMax[n] = m_objFeatureControlPtr[n]->GetFloatFeature("ExposureTime")->GetMax();
	m_dShutterValueMin[n] = m_objFeatureControlPtr[n]->GetFloatFeature("ExposureTime")->GetMin();
	m_dGainValueMax[n] = m_objFeatureControlPtr[n]->GetFloatFeature("Gain")->GetMax();
	m_dGainValueMin[n] = m_objFeatureControlPtr[n]->GetFloatFeature("Gain")->GetMin();
	m_dBalanceWhiteRatioMax[n] = m_objFeatureControlPtr[n]->GetFloatFeature("BalanceRatio")->GetMax();
	m_dBalanceWhiteRatioMin[n] = m_objFeatureControlPtr[n]->GetFloatFeature("BalanceRatio")->GetMin();
#ifndef Release
	cout << "m_dShutterValueMax:" << m_dShutterValueMax[n] << endl
		<< "m_dShutterValueMin:" << m_dShutterValueMin[n] << endl
		<< "m_dGainValueMax:" << m_dGainValueMax[n] << endl
		<< "m_dGainValueMin:" << m_dGainValueMin[n] << endl
		<< "m_dBalanceWhiteRatioMax:" << m_dBalanceWhiteRatioMax[n] << endl
		<< "m_dBalanceWhiteRatioMin:" << m_dBalanceWhiteRatioMin[n] << endl;
#endif // !DEBUG

}

int video::videoCheck() {//搜索相机
	IGXFactory::GetInstance().UpdateDeviceList(1000, vectorDeviceInfo);
	//判断枚举到的设备是否大于零，如果不是则弹框提示
	deviceNum = vectorDeviceInfo.size();
	cout << deviceNum << endl;
	return deviceNum <= 0 ? 0 : deviceNum;
}
bool video::videoOpen(int n) {//初始化相机

	bool bIsDeviceOpen = false;       ///< 设备是否已打开标志
	bool bIsStreamOpen = false;       ///< 设备流是否已打开标志
	bool m_bSupportExposureEndEvent = false;       ///< 是否支持曝光结束标志
	try {
		cerr << vectorDeviceInfo[n].GetSN() << endl;
		m_objDevicePtr[n] = IGXFactory::GetInstance().OpenDeviceBySN(vectorDeviceInfo[n].GetSN(), GX_ACCESS_EXCLUSIVE);//打开设备
		bIsDeviceOpen = true;
		m_objFeatureControlPtr[n] = m_objDevicePtr[n]->GetRemoteFeatureControl();//获取属性控制器对象 
		uint32_t nStreamCount = m_objDevicePtr[n]->GetStreamCount();//获取流通道个数

		if (nStreamCount > 0) {//打开流
			m_objStreamPtr[n] = m_objDevicePtr[n]->OpenStream(0);
			m_objStreamFeatureControlPtr[n] = m_objStreamPtr[n]->GetFeatureControl();
			bIsStreamOpen = true;
		}
		else {
			throw exception("未发现设备流!");
		}
		//初始化相机参数
		initParam(n);
		m_bIsOpen[n] = true;
	}
	catch (CGalaxyException& e) {

		if (bIsStreamOpen) {//判断设备流是否已打开
			m_objStreamPtr[n]->Close();
		}
		if (bIsDeviceOpen) {//判断设备是否已打开
			m_objDevicePtr[n]->Close();
			return false;
		}
	}
	catch (std::exception& e) {

		if (bIsStreamOpen) {//判断设备流是否已打开
			m_objStreamPtr[n]->Close();
		}
		if (bIsDeviceOpen) {//判断设备是否已打开
			m_objDevicePtr[n]->Close();
		}
		return false;
	}
	return true;
}

bool video::videoStart(int n) {//创建流对象
	pCaptureEventHandler = new CSampleCaptureEventHandler(n);
	m_objStreamPtr[n]->RegisterCaptureCallback(pCaptureEventHandler, NULL);
	m_objStreamPtr[n]->StartGrab();//发送开采命令
	m_objFeatureControlPtr[n]->GetCommandFeature("AcquisitionStart")->Execute();
	return true;
}

bool video::getFrame(cv::Mat& img,int n) {//获取一帧图片
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

bool video::videoStopStream(int n) {//断开拉流
	m_objFeatureControlPtr[n]->GetCommandFeature("AcquisitionStop")->Execute();
	m_objStreamPtr[n]->StopGrab();//发送停采命令
	m_objStreamPtr[n]->UnregisterCaptureCallback();//注销采集回调
	return true;
}
void video::videoClose(int n) {//断开相机
	//注销远端设备事件
	m_objFeatureControlPtr[n]->UnregisterFeatureCallback(NULL);
	//注销设备掉线事件
	//ObjDevicePtr->UnregisterDeviceOfflineCallback(hDeviceOffline);

	//释放资源
	m_objStreamPtr[n]->Close();
	m_objDevicePtr[n]->Close();
}

bool video::setTrigMode(int mode,int n)
{
	try {
		m_bTriggerMode[n] = m_objFeatureControlPtr[n]->IsImplemented("TriggerMode");//是否支持触发模式选择
		if (m_bTriggerMode[n]) {
			m_objFeatureControlPtr[n]->GetEnumFeature("TriggerMode")->SetValue("Off");//设置触发模式关
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
	}
	catch (CGalaxyException& e) {
		cerr << e.what() << endl;
		return false;
	}
	catch (std::exception& e) {
		cerr << e.what() << endl;
		return false;
	}
	return false;
}

void video::executeSoftTrig() {
	try {//发送软触发命令(在触发模式开启时有效)
		for(int i=0;i<deviceNum;++i)
			m_objFeatureControlPtr[i]->GetCommandFeature("TriggerSoftware")->Execute();
	}
	catch (CGalaxyException& e) {
		cout << e.what() << endl;
		return;
	}
	catch (std::exception& e) {
		cout << e.what() << endl;
		return;
	}
}

void video::SetExposeTime(double m_dEditShutterValue,int n) {//设置曝光
	m_objFeatureControlPtr[n]->GetFloatFeature("ExposureTime")->SetValue(m_dEditShutterValue);
	if (!m_bIsOpen) {
		return;
	}
	double dShutterValueOld = m_dEditShutterValue;
	try {//判断输入值是否在曝光时间范围内，如果不是则设置与其最近的边界值
	
		if (m_dEditShutterValue > m_dShutterValueMax[n]) {
			m_dEditShutterValue = m_dShutterValueMax[n];
		}
		if (m_dEditShutterValue < m_dShutterValueMin[n]) {
			m_dEditShutterValue = m_dShutterValueMin[n];
		}
		m_objFeatureControlPtr[n]->GetFloatFeature("ExposureTime")->SetValue(m_dEditShutterValue);
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
}							
void video::SetAdjustPlus(double m_dEditGainValue,int n) {//设置增益
	if (!m_bIsOpen){
		return;
	}
	double dGainValueOld = m_dEditGainValue;
	try{
		//判断输入值是否在增益值范围内，如果不是则设置与其最近的边界值
		if (m_dEditGainValue > m_dGainValueMax[n]){
			m_dEditGainValue = m_dGainValueMax[n];
		}
		if (m_dEditGainValue < m_dGainValueMin[n]){
			m_dEditGainValue = m_dGainValueMin[n];
		}
		m_objFeatureControlPtr[n]->GetFloatFeature("Gain")->SetValue(m_dEditGainValue);
	}
	catch (CGalaxyException & e){
		m_dEditGainValue = dGainValueOld;
	}
	catch (std::exception & e){
		m_dEditGainValue = dGainValueOld;
	}
}
void video::setBufferSize(int nSize) {}
void video::setBalanceRatio(double m_dEditBalanceRatioValue,int n) {
	if (!m_bIsOpen[n]){
		return;
	}
	double dBalanceWhiteRatioOld = m_dEditBalanceRatioValue;
	try{
		//判断输入值是否在自动白平衡系数范围内，如果不是则设置与其最近的边界值
		if (m_dEditBalanceRatioValue > m_dBalanceWhiteRatioMax[n]){
			m_dEditBalanceRatioValue = m_dBalanceWhiteRatioMax[n];
		}
		if ((m_dEditBalanceRatioValue < m_dBalanceWhiteRatioMin[n])){
			m_dEditBalanceRatioValue = m_dBalanceWhiteRatioMin[n];
		}
		m_objFeatureControlPtr[n]->GetFloatFeature("BalanceRatio")->SetValue(m_dEditBalanceRatioValue);
	}
	catch (CGalaxyException & e){
		m_dEditBalanceRatioValue = dBalanceWhiteRatioOld;
		cout << e.what() << endl;
	}
	catch (std::exception & e){
		m_dEditBalanceRatioValue = dBalanceWhiteRatioOld;
		cout << e.what() << endl;
	}
}
void video::setResolution(int height, int width) {

} 
void video::setROI(int64_t nX, int64_t nY, int64_t nWidth, int64_t nHeight){

}
void video::setBinning(){

}
bool video::loadSetting(int mode){

	return true;
}

void video::setFrameRate(double rate) {

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
