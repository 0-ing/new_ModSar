#include "ara/core/result.h"
#include "stdio.h"
#include "ara/core/core_error_domain.h"
ara::core::Result<void> sayHello()
{
    ara::core::ErrorCode code(ara::core::CoreErrc::kInvalidMetaModelPath);
    ara::core::Result<void> err(code);
    return err;
}
int main()
{
    ara::core::Result<int> result(1);

    printf("%d\n ",result.Value());
    ara::core::ErrorCode code(ara::core::CoreErrc::kInvalidMetaModelPath);
    ara::core::Result<void> err(code);
    printf("%s\n",err.Error().Message().data());

    auto xx = sayHello() ;
    xx.Error().Message().data();
}