#include <thread>
#include "mainPC.h"

int main()
{
    int setting;//先随便写个int，之后应该改为专门的配置类
    mainPC image_cons_prod(&setting);

    std::thread t1(&mainPC::ImageProducer, std::ref(image_cons_prod)); // pass by reference
    std::thread t2(&mainPC::ImageConsumer, std::ref(image_cons_prod));

    t1.join();
    t2.join();

    return 0;
}