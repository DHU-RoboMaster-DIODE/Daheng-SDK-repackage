#include <thread>

#include "mainPC.h"

int main()
{
    int setting;//�����д��int��֮��Ӧ�ø�Ϊר�ŵ�������
    mainPC image_cons_prod(&setting);

    std::thread t1(&mainPC::ImageProducer, std::ref(image_cons_prod)); // pass by reference
    std::thread t2(&mainPC::ImageConsumer, std::ref(image_cons_prod));

    t1.join();
    t2.join();
    return 0;
}