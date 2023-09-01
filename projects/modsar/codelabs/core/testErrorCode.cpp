#include "ara/core/error_code.h"
#include "ara/core/core_error_domain.h"
#include "ara/core/future_error_domain.h"
#include "stdio.h"

int main() {
    
    ara::core::CoreErrorDomain domain;
   
    ara::core::ErrorCode code((ara::core::ErrorDomain::CodeType)ara::core::CoreErrc::kInvalidMetaModelPath,domain);

    ara::core::ErrorCode code1(ara::core::future_errc::future_already_retrieved);

    printf("message:%s\n",code.Message().data());
    printf("message:%s\n",code1.Message().data());
    //message:Invalid meta model path
    //message:future already retrieved

}