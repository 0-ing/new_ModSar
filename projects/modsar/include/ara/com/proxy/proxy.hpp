#ifndef _PROXY_HPP_
#define _PROXY_HPP_
#include <memory>
#include "instance_identifer.h"

class Proxy : public Adapter
{
public:
     Proxy();
     void SendRequest(Message data);
     void RegisterMessageHandler(MethodId method_id, message_handler_t handler);
private:
     std::shared_ptr<stub> stub_;
};

#endif // _PROXY_HPP_