#pragma once
#include "video.h"
class mainPC
{
public:
	int setting;
	mainPC(int * setting);
	void ImageProducer();
	void ImageConsumer();
private:
	video a;
	int ROI[4];
	int ExposeTime;
	double AdjustPlus;
	double BalanceRatio;
	double FrameRate;
};

