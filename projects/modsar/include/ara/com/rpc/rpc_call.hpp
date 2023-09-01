#ifndef _RPC_CALL_HPP_
#define _RPC_CALL_HPP_
#include <functional>
#include <memory>
 
typedef std::function< void (const std::shared_ptr< Message > &) > message_handler_t;

class ICall {
public:
    /**
     * 可以隐藏具体实现 该实现放 cpp里
    */
    virtual void invoke() = 0;
    Status status_;
};
/**
 * 完成消息转发
 * */
template <class InputMessage, class OutputMessage, class HostType>
class BlockingCall : public ICall
{
public:
    BlockingCall(  std::shared_ptr<HostType> host)
    :host_(host)
    {
        
    }
 
    core::Result<void> invoke(const RpcMethod& method,const InputMessage& request, OutputMessage* result , HostType* host)
    {
       return ICall::invoke();
    }
    Status status() { return status_; }

private:
    std::shared_ptr<vsomeip::message> request_;
    std::shared_ptr<HostType> host_;
    message_handler_t handler_;
  

};

/**
 * 完成消息转发
 * */
template <class InputMessage, class OutputMessage, class HostType>
class NonBlockingCall : public ICall {

};


#endif // _RPC_CALL_HPP_