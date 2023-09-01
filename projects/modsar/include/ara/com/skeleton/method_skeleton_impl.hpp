/**
 * \copyright bcsc all rights reseverd
 * \brief 简单说明
 * \author ZYL
 * \date 2023/7/8
 */
#ifndef _METHOD_SKELETON_IMPL_HPP_
#define _METHOD_SKELETON_IMPL_HPP_
#include "method_skeleton.hpp"
#include "ara/com/types.h"
class MethodSkeletonImplemation :public MethodSkeleton
{

public:

    MethodSkeletonImplemation(const char* name ,const InstanceIdentifer& instanceIdentifer,
       std::shared_ptr<Skeleton> skeleton)
    {       
        
    }
    ~MethodSkeletonImplemation() 
    {
        
    }

    
private:

};

// 用户类生成代码
class IHelloworldMethodSkeletonImplemation : public MethodSkeletonImplemation {
public:
    /***
     * rpc::Requst 还是 RpcRequest? 
     */
    virtual status sayHello(rpc::Request &request, rpc::Response *response)
    {

    }

    virtual core::Futher<void> sayHello(const int &helloType) = 0;
};


class HelloMethodSkeletonImplemation : public IHelloworldMethodSkeletonImplemation {
public: 
    core::Futher<void> sayHello(const int &helloType)
    {
        // 用户真正要做
    }
};
#endif // _METHOD_SKELETON_IMPL_HPP_