/**
 * \copyright bcsc all rights reseverd
 * \brief 简单说明
 * \author ZYL
 * \date 2023/7/8
 */
#ifndef _METHOD_PROXY_HPP_
#define _METHOD_PROXY_HPP_

#include <memory>

class MethodProxy
{

public:
    MethodProxy(std::shared_ptr<Proxy> &proxy);
    virtual ~MethodProxy() {}
    virtual void FindService();
    void SendRequest(Message data);
    std::shared_ptr<Proxy> GetProxy();

private:
    std::shared_ptr<Proxy> proxy_;
};
#endif // _METHOD_PROXY_HPP_