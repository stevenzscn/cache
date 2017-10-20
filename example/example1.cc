#include <iostream>
#include <string>
#include "cache.h"

int main()
{
    cache::Cache<int32_t> haha;
    std::string key = "123";
    haha.set(key, 777);
    haha.set(key, 999);
    haha.set("zhangshuai", 123);
    haha.set("zuozuomuxi", 222);
    haha.set("chenhuijie", 233);
    haha.set("aaaaa", 123);
    haha.set("bbbbb", 123);
    haha.set("meimei", 123);
    int value = 0;
    haha.get(key, value);
    std::cout << "get " << key << " value:" << value << std::endl;

    std::cout << haha.print();

    time_t exp = 0;
    haha.ttl("zuozuomuxi", exp);
    std::cout << "ttl " << "zuozuomuxi" << " exp:" << exp << std::endl;

    haha.incr(key, value);
    std::cout << "incr " << key << " value:" << value << std::endl;

    haha.incrby(key, 100, value);
    std::cout << "incrby 100 " << key << " value:" << value << std::endl;

    return 0;
}
