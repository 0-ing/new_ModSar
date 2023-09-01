/**
 * \copyright bcsc all rights reseverd
 * \brief 简单说明
 * \author ZYL
 * \date 2023/7/8
 */
#ifndef _EVENT_PROXY_HPP_
#define _EVENT_PROXY_HPP_

#include <functional>
#include <memory>

namespace ara
{
     namespace com
     {
          namespace event
          {
               typedef std::function<void(const std::shared_ptr<SamplePtr<DataType>> &)> Sample_handler_t;
               class EventProxy
               {
               public:
                    EventProxy(std::shared_ptr<Proxy> &proxy) : proxy_(proxy)
                    {
                    }
                    bool IsSubscribed();
                    void Unsubscribe();
                    void Subscribe(size_t SampleCount);
                    void GetNewSample(Sample_handler_t handler);

               private:
                    std::shared_ptr<Proxy> proxy_;
               };
          } // namespace name

     } // namespace name

} // namespace ara

#endif // _EVENT_PROXY_HPP_