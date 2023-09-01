
#ifndef _RPC_SERVICE_METHOD_HPP
#define _RPC_SERVICE_METHOD_HPP
#include <functional>
#include <memory>
#include "ara/com/types.h"

template <class RequestType, class ResponseType, class HostType>
// typedef std::function< void (const RequestType*, ResponseType*) > request_handler_t; // 典型的C 系命名风格 可以换成
using RequestHandler = std::function<void(const RequestType *, ResponseType *)>;

/***
 * 处理转发到客户定义的函数
 */
class RpcServiceMethod : public RpcMethod
{
public:
    // 用户定义的method 一般是 ara::core::Future<UseType> action(paramers)
    // 注意这里参数是拥有一定自定义行为的
    // grpc 仍然需要用户去处理 实际的发送 行为
    // 这里的类grpc代码可以使用工具生成

    RpcServiceMethod(const char *name,
                     const ara::com::InstanceIdentifier &instance_indetifer,
                     method_t method_id,
                     const RequestHandler &func,
                     HostType *host)
        : RpcMethod(name,instance_indetifer, method_id), host_(host),
          func_(func) {}

    // using Method to instead of using method directly
    // we need to hide vsomip usage in hpp file
    void RunHandler(const Method &param);
     
    RequestHandler func_;
    std::shared_ptr<HostType> host_;
};

#endif // _RPC_SERVICE_METHOD_HPP
