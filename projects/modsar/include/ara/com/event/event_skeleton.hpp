/**
 * \copyright bcsc all rights reseverd
 * \brief 简单说明
 * \author ZYL
 * \date 2023/7/8
 */
#ifndef _EVENT_SKELETON_HPP_
#define _EVENT_SKELETON_HPP_
#include <memory>

template<class SampleType>
class EventSkeleton
{
public:
    EventSkeleton(std::shared_ptr<Skeleton> skeleton):skeleton_(skeleton)
    {
    }      
    typename std::enable_if<IsRawMemory<SampleType>::value, RawMemory>::type Allocate(size_t size)
    {
        
    }
    
    void Send(SampleType&& data);    
    std::shared_ptr<Skeleton> GetSkeleton()
    {
        return skeleton_;
    }
private:

    std::shared_ptr<Skeleton> skeleton_;
};

#endif // _EVENT_SKELETON_HPP_