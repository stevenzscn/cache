#include "cache.h"

int main() {
    cache::Cache<int32_t> haha;

    haha.set("zs", 1);
    haha.set("zs", 2);

    int32_t out = 0;
    haha.get("zs", out);
    std::cout << "zs is: " << out << std::endl;



    std::cout << "to exit...\n";

    return 0;
}
