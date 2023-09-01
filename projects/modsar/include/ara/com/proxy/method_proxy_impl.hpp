/**
 * \copyright bcsc all rights reseverd
 * \brief 简单说明
 * \author ZYL
 * \date 2023/7/8
 */

#ifndef METHOD_PROXY_IMPL_HPP_
#define METHOD_PROXY_IMPL_HPP_

#if 0 // 删除本头文件不直接依赖的头文件
#include <atomic>
#include <condition_variable>
#include <deque>
#include <map>
#include <mutex>
#include <set>
#include <string>
#include <thread>
#include <vector>

#include <boost/asio/signal_set.hpp>
#include <boost/asio/steady_timer.hpp>

#include <vsomeip/export.hpp>
#include <vsomeip/application.hpp>
#endif
#include "method_proxy.hpp"
#include "instance_identifer.h"
/***
 * \brief 方法代理的实现 头文件名称不采用缩写 尽量全称
 *  MethodProxyImplemation
 */
class MethodProxyImplemation : public MethodProxy
{

public:
    /**
     * \brief 封装 severt_t 和 instance_t 和 method_t
     *
     */
    MethodProxyImplemation(const char *name, const InstanceIdentifer &identifer,
                           std::shared_ptr<Proxy> proxy)
        : MethodProxy(proxy),
          rpcmethod_(name, identifer)
    {
        // 构造函数内不处理行为
        // FindMethodService(service_id, instance_id, method_id);
    }
    ~MethodProxyImplemation()
    {
    }
#if 0 // 这里实现放到proxy 里
    /**
     * 这边可以遵循ap 规范 使用 FinderHandler 
     * vsomeip 支持 methodid 级别的 查找机制？
     */
    void FindMethodService(const InstanceIdentifer &identifer, method_t method_id)
    {
        proxy_->FindService(service_id, instance_id, method_id);
    }

    void RegisterMessageHandler(service_t service_id, instance_t instance_id, method_t method_id, message_handler_t handler)
    {
        proxy_->RegisterMessageHandler(service_id, instance_id, method_id, handler);
    }

    BlockingCall *CreateCall(const ::helloworld::HelloRequest &request, ::helloworld::HelloReply *response)
    {
        auto *call = new BlockingCall<::helloworld::HelloRequest, ::helloworld::HelloReply, method_proxy>(rpcmethod_, request, response, this);
        return call;
    }

    Status sayhello(const ::helloworld::HelloRequest &request, ::helloworld::HelloReply *response)
    {
        return CreateCall(request, response, this).status(); // 这里问题会很多 内存泄漏
    }
#endif

private:
    const RpcMethod rpcmethod_;
};

class IHelloworldMethodProxy : public MethodProxyImplemation
{
public:
    Status sayHello(const ::helloworld::HelloRequest &request, ::helloworld::HelloReply *response)
    {
        Status status = ERROR;
        BlockingCall *call = new BlockingCall<::helloworld::HelloRequest,
                                              ::helloworld::HelloReply,
                                              method_proxy>(rpcmethod_,
                                                            request,
                                                            response,
                                                            this);
        if (call)
        {
            status = call->invoke();
        }
        // delete call
        delete call;
        return status;
    }
    // 实际上需要在Proxy对应的实现为
    core::Futher<void> sayHello1(const int &helloType) override
    {
        // 构建 request 和 response
        sayHello(request, response);
    }
    core::Futher<void> sayHello2(const int &helloType) override
    {
        // 构建 request 和 response
        sayHello(request, response);
    }

    ara::core::Result<ara::com::ServiceHandleContainer<IHelloworldMethodProxy::HandleType>> 
    FindService(ara::com::InstanceIdentifier instance)
    {
        ::com::SetDeploymentType(instance, ::ara::com::DeploymentType::<#if serviceId == "0x0">IPC<#else>SOME_IP</#if>);
        return ::com::sd::Runtime::GetInstance().findService(${name}::GetServiceIdentifier(), instance);
    }
};

#endif // METHOD_PROXY_IMPL_HPP_