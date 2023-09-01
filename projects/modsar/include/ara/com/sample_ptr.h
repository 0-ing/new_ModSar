
#ifndef _SAMPLE_PTR_HPP_
#define _SAMPLE_PTR_HPP_
#include <memory>

template <typename T>
class SamplePtr {
public:
    // Replaces the managed object
    void Reset(T* dataPtr = nullptr) { dataPtr_ = std::shared_ptr<T>(dataPtr); }

    // Returns the stored object, is the API of 1911
    T* Get() const noexcept { return dataPtr_.get(); }

    // This is not AutoSAR interface, applications is not allowed use!!!!
    // Returns the stored object, is the API of 1803
    T* get() const noexcept { return dataPtr_.get(); }

    long UseCount() const noexcept { return dataPtr_.use_count(); }
private:
    std::shared_ptr<T> dataPtr_;    
};

#endif // _SAMPLE_PTR_HPP_