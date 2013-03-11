#include "async/threadUtilizer.hpp"
#include <iostream>

int main()
{
    std::cout<<"hw"<<std::endl;
    async::ThreadUtilizer tu;
    tu.te_utilize(std::chrono::nanoseconds(200));
    tu.te_utilize(std::chrono::minutes(200));
    tu.te_utilize(std::chrono::system_clock::now()+std::chrono::minutes(200));

    return 0;
}
