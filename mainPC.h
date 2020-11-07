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
};

