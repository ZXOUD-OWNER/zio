#include "head.hpp"
#include <mimalloc-new-delete.h>
int main(int argc, char **argv)
{
    Log_MQ mq(argv[0]);
    ZmqMiddle middle(Singleton::getInstance().getInstance().getConf());
    middle.start();
    return 0;
}