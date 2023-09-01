#ifndef _METHOD_SKELETON_HPP_
#define _METHOD_SKELETON_HPP_
 
#include <memory>

/***
 * 这里说明 
 */
class MethodSkeleton
{
public:
    MethodSkeleton(std::shared_ptr<Skeleton> skeleton):skeleton_(skeleton)
    {
    }
    virtual ~MethodSkeleton() {}
    virtual void OfferService();
    virtual void StopOfferService();
    void SendResponse(const Message& data);
    /**
     * 具体实现需要放在sources 文件里 
     */
    std::shared_ptr<Skeleton> GetSkeleton();
private:

    std::shared_ptr<Skeleton> skeleton_;
};

#endif // _METHOD_SKELETON_HPP_