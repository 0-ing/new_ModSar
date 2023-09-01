#ifndef _SKELETON_HPP_
#define _SKELETON_HPP_
#include <memory>
#include "instance_identifer.h"

class Skeleton : public Adapter
{
public:
     /**
      * OfferService 需要传入 service_ideneifer 和 methoid 不？
      * ap 规范里 OfferService(void)
      * 具体实现可以这样
      */
     virtual void OfferService();
     void SendResponse(message data);
     /**
      * 类里绑定了 identifer 注册时如有需要直接使用
      */
     void RegisterServiceMethod(method_t method_id, message_handler_t handler);

private:
     std::shared_ptr<stub> stub_;

     InstanceIdentifer service_identifer; // skeleton 里面可以有很多Method 因此不用指定Methodid这里
};

class VSomeipSkeleton : public Skeleton
{
public:
     void OfferService()
     {
          OfferServiceUsingVsomeip(serviceid, instanceid);
     }
     InstanceIdentifer service_identifer;
};
#endif // _SKELETON_HPP_