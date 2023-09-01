#include "ara/core/promise.h"
#include "stdio.h"
#include <thread>
#include <chrono>
int main()
{
    ara::core::Promise<int> promise;
    ara::core::Future<int>&& f = promise.get_future();
    std::thread a([&]{
        std::this_thread::sleep_for(  std::chrono::seconds(2));
       // promise.set_value(100);
        
        promise.SetError(ara::core::future_errc::future_already_retrieved);
    });
    a.detach();
    f.wait();
    auto res = f.GetResult();
    if (res.CheckError(ara::core::future_errc::future_already_retrieved))
        printf("%s\n",res.Error().Message().data());
    else
        printf("%d\n",res.Value());
    return 0;
}