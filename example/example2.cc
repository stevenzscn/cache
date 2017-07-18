#include <iostream>
#include <string>
#include <unistd.h>
#include "cache.h"

int main()
{
    {
    cache::Cache<uint64_t> haha;
    std::string key = "123";
    haha.set(key, 777);
    haha.set(key, 999);
    haha.set("zhangshuai", 123);
    haha.set("zuozuomuxi", 222);
    haha.set("chenhuijie", 233);
    haha.set("nidaye", 123);
    haha.set("caonima", 123);
    haha.set("meimei", 123);
    haha.set("1", 1, 5);
    haha.set("2", 2, 5);
    haha.set("3", 3, 5);
    haha.set("4", 4, 5);
    haha.set("5", 5, 5);
    haha.set("6", 6, 5);
    haha.set("7", 7, 5);
    haha.set("8", 8, 5);
    haha.set("9", 9, 5);

    int value = 0;
    haha.get("zhangshuai", value);
    std::cout << "get zhangshuai:" << value << std::endl;

    sleep(1);
    std::cout << haha.print();

    sleep(1);
    time_t exp = 0;
    haha.ttl("8", exp);
    std::cout << "8 exp:" << exp << std::endl;

    haha.incr("3", value);
    std::cout << "3 incr:" << value << std::endl;

    int cnt = 0;
    while (1) {
        cnt++;
        sleep(1);
        std::cout << haha.print();
        if (cnt > 6) {
            break;
        }
    }
    }
    sleep(1);

    return 0;
}

