#ifndef METHOD_HPP_
#define METHOD_HPP_
#include "instance_identifer.h"

/***
 * 头文件保持干净 减少不必要依赖
 *
 */
#define GRPC_ALLOW_EXCEPTIONS 1
template <class Callable>
Status CatchingFunctionHandler(Callable &&handler)
{
#if 0
#if GRPC_ALLOW_EXCEPTIONS
  try {
    return handler();
  } catch (...) {
    return ::grpc::Status(::grpc::StatusCode::UNKNOWN,
                          "Unexpected error in RPC handling");
  }
#else  // GRPC_ALLOW_EXCEPTIONS
  return handler();
#endif // GRPC_ALLOW_EXCEPTIONS

#endif // 0

  // 可以减少对异常的使用 通过返回值来查看具体执行状态
  return handler();
}

template <class ParamType, class ResponseType>
void RunHandlerHelper(const ParamType *param,
                      ResponseType *rsp, Status &status)
{

  std::string str(reinterpret_cast<const char *>(rsp)); // 减少 cast的使用 容易crash
  auto *payload = param->get_payload();
  payload->set_data((str.begin(), str.end()));
}

class MethodId {

};

class RpcMethod
{
public:
  enum RpcType
  {
    NORMAL_RPC = 0,
    CLIENT_STREAMING, // request streaming
    SERVER_STREAMING, // response streaming
    BIDI_STREAMING
  };

  RpcMethod(const char *name, const InstanceIdentifer &instance_identifer, method_t method_id)
      : name_(name),
        instance_identifer_(instance_identifer),
        method_id_(method_id),
        method_type_(NORMAL_RPC) {}

  const char *name() const { return name_; }
  service_t serviceId() const { return service_id_; }
  instance_t instanceId() const { return instance_id_; }
  method_t methodId() const { return method_id_; }
  RpcType method_type() const { return method_type_; }
  void SetMethodType(RpcType type) { method_type_ = type; }

private:
  const char *name_;
  InstanceIdentifer instance_identifer_;
  method_t method_id_;
  RpcType method_type_;
};

#endif // METHOD_HPP_