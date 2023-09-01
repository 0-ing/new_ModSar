/**
 * \copyright BCSC all rights resvered
 * \author JJL
 * \date 2023/7/22
 */
#ifndef _ARA_CORE_PROMISE_H_
#define _ARA_CORE_PROMISE_H_

#include <exception>
#include <future>
#include <memory>
#include <mutex>
#include <system_error>
#include <utility>

#include "ara/core/error_code.h"
#include "ara/core/result.h"
#include "ara/core/future.h"

namespace ara
{
    namespace core
    {

        /**
         * \brief ara::core specific variant of std::promise class
         *
         * \tparam T  值的类型
         * \tparam E  错误的类型
         *
         * @traceid{SWS_CORE_00340}
         */
        template <typename T, typename E = ErrorCode>
        class Promise final
        {
            using R = Result<T, E>;
            using Lock = std::unique_lock<std::mutex>;

        public:
            /// Alias type for T
            using ValueType = T;

            /**
             * \brief 默认构造函数
             *
             * 此函数的行为与相应的std:：promise函数相同
             *
             * @traceid{SWS_CORE_00341}
             */
            Promise() : extra_state_(std::make_shared<internal::State<T, E>>()) {}

            /**
             * \brief Promise对象的析构函数
             *
             * 此函数的行为与相应的std:：promise函数相同
             *
             * @traceid{SWS_CORE_00349}
             */
            ~Promise() {}

            /**
             * \brief 禁用复制构造函数。
             *
             * @traceid{SWS_CORE_00350}
             */
            Promise(Promise const &) = delete;

            /**
             * \brief 赋值运算符被禁止
             *
             * @traceid{SWS_CORE_00351}
             */
            Promise &operator=(Promise const &) = delete;

            /**
             * \brief 移动构造函数
             *
             * 该功能应与相应的 std::promise function.
             *
             * \param other  the other instance
             *
             * @traceid{SWS_CORE_00342}
             */
            Promise(Promise &&other) noexcept
                : lock_(other.mutex_), delegate_promise_(std::move(other.delegate_promise_)), extra_state_(std::move(other.extra_state_))
            {

                lock_.unlock();
            }

            /**
             * \brief 移动function
             *
             * 此函数的行为应与相应的std:：promise函数相同。
             *
             * \param other  the other instance
             * \returns *this
             *
             * @traceid{SWS_CORE_00343}
             */
            Promise &operator=(Promise &&other) noexcept
            {
                if (this != &other)
                {
                    Lock lhsLock(mutex_, std::defer_lock);
                    Lock rhsLock(other.mutex_, std::defer_lock);
                    std::lock(lhsLock, rhsLock);
                    delegate_promise_ = std::move(other.delegate_promise_);
                    extra_state_ = std::move(other.extra_state_);
                }
                return *this;
            }

            /**
             * \brief 将此实例的内容与另一个实例的内容交换。
             *
             * 此函数的行为应与相应的std:：promise函数相同。
             *
             * \param other  the other instance
             *
             * @traceid{SWS_CORE_00352}
             */
            void swap(Promise &other) noexcept
            {
                Lock lhsLock(mutex_, std::defer_lock);
                Lock rhsLock(other.mutex_, std::defer_lock);
                std::lock(lhsLock, rhsLock);

                using std::swap;
                swap(delegate_promise_, other.delegate_promise_);
                swap(extra_state_, other.extra_state_);
            }
            /**
             * \brief 返回类型T的关联Future。
             *
             * 一旦此Promise收到结果或异常，就会设置返回的Future。
             * 此方法只能调用一次，因为不允许每个Promise有多个Futures。
             *
             * \returns a Future for T类型
             *
             * @traceid{SWS_CORE_00344}
             */
            Future<T, E> get_future() { return Future<T, E>(delegate_promise_.get_future(), extra_state_); }
            /**
             * \brief 将错误移动到共享状态，并使状态准备就绪。
             *
             * \param error  要存储的错误
             *
             * @traceid{SWS_CORE_00353}
             */
            // 在一个promise中，set_exception之后的SetError将引发异常
            void SetError(E &&err)
            {
                Lock lock(mutex_);
                R r = R::FromError(std::move(err));
                delegate_promise_.set_value(r);
                extra_state_->FireContinuation();
            }

            /**
             * \brief 将错误复制到共享状态，并使该状态就绪。
             *
             * \param error  要存储的错误
             *
             * @traceid{SWS_CORE_00354}
             */
            // 在一个promise中，set_exception之后的SetError将引发异常
            void SetError(E const &err)
            {
                Lock lock(mutex_);
                R r = R::FromError(err);
                delegate_promise_.set_value(r);
                extra_state_->FireContinuation();
            }

            /**
             * \brief Sets an exception.
             *
             * 对关联的Future调用Get（）将在调用Future方法的上下文中重新引发异常
             * \param p 要设置的exception_ptr
             *
             * \note  This method is DEPRECATED. The exception is defined by the error code
             */
            void set_exception(std::exception_ptr p)
            {
                Lock lock(mutex_);
                delegate_promise_.set_exception(p);
                extra_state_->FireContinuation();
            }

            /**
             * \brief 将结果推向未来。
             *
             * \param value 要存储的值
             *
             * @traceid{SWS_CORE_00346}
             */
            void set_value(T &&value)
            {
                R r = std::move(value);
                Lock lock(mutex_);
                delegate_promise_.set_value(r);
                extra_state_->FireContinuation();
            }

            /**
             * \brief 将结果复制到未来。
             *
             * \param value 要存储的值
             *
             * @traceid{SWS_CORE_00345}
             */
            void set_value(T const &value)
            {
                R r = value;
                Lock lock(mutex_);
                delegate_promise_.set_value(r);
                extra_state_->FireContinuation();
            }

        private:
            std::mutex mutex_;
            Lock lock_;
            std::promise<R> delegate_promise_;
            std::shared_ptr<internal::State<T, E>> extra_state_;
        };

        /**
         * \brief 无效承诺的显式专门化
         * \tparam E  the type of error
         *
         * @traceid{SWS_CORE_06340}
         */
        template <typename E>
        class Promise<void, E> final
        {
            using R = Result<void, E>;

            using Lock = std::unique_lock<std::mutex>;

        public:
            /// \copydoc Promise::ValueType
            using ValueType = void;

            /// \copydoc Promise::Promise
            /// @traceid{SWS_CORE_06341}
            Promise() : extra_state_(std::make_shared<internal::State<void, E>>()) {}

            /// \copydoc Promise::~Promise
            /// @traceid{SWS_CORE_06349}
            ~Promise() {}

            /// \copydoc Promise::Promise(const Promise&)
            /// @traceid{SWS_CORE_06350}
            Promise(Promise const &) = delete;

            /// \copydoc Promise::operator=(const Promise&)
            /// @traceid{SWS_CORE_06351}
            Promise &operator=(Promise const &) = delete;

            /**
             *
             * \copydoc Promise::Promise(Promise&&)
             * @traceid{SWS_CORE_06342}
             */
            Promise(Promise &&other) noexcept
                : delegate_promise_(std::move(other.delegate_promise_)), extra_state_(std::move(other.extra_state_)), lock_(other.mutex_)
            {
                lock_.unlock();
            }

            /**
             * \copydoc Promise::operator=(Promise&&)
             * @traceid{SWS_CORE_06343}
             */
            Promise &operator=(Promise &&other) noexcept
            {
                if (this != &other)
                {
                    Lock lhsLock(mutex_, std::defer_lock);
                    Lock rhsLock(other.mutex_, std::defer_lock);
                    std::lock(lhsLock, rhsLock);

                    delegate_promise_ = std::move(other.delegate_promise_);
                    extra_state_ = std::move(other.extra_state_);
                }
                return *this;
            }
            /**
             * \copydoc Promise::swap
             * @traceid{SWS_CORE_06352}
             */
            void swap(Promise &other) noexcept
            {
                Lock lhsLock(mutex_, std::defer_lock);
                Lock rhsLock(other.mutex_, std::defer_lock);
                std::lock(lhsLock, rhsLock);

                using std::swap;
                swap(delegate_promise_, other.delegate_promise_);
                swap(extra_state_, other.extra_state_);
            }

            /**
             * \copydoc Promise::get_future
             * @traceid{SWS_CORE_06344}
             */
            Future<void, E> get_future() { return Future<void, E>(delegate_promise_.get_future(), extra_state_); }

            /** \brief 准备好共享状态。
            
                @traceid{SWS_CORE_06345}
            */
            void set_value()
            {
                Lock lock(mutex_);
                delegate_promise_.set_value(R::FromValue());
                extra_state_->FireContinuation();
            }

            /**
             * @copydoc Promise::SetError(E&&)
             * @traceid{SWS_CORE_06353}
             * 在一个promise中，set_exception之后的SetError将引发异常
             */
            void SetError(E &&err)
            {
                R r = R::FromError(std::move(err));
                Lock lock(mutex_);
                delegate_promise_.set_value(r);
                extra_state_->FireContinuation();
            }

            /**
             * \copydoc Promise::SetError(const E&)
             * traceid{SWS_CORE_06354}
             * 在一个promise中，set_exception之后的SetError将引发异常
             */
            void SetError(E const &err)
            {
                R r = R::FromError(err);
                Lock lock(mutex_);
                delegate_promise_.set_value(r);
                extra_state_->FireContinuation();
            }

            /**
             * @copydoc Promise::set_exception
             */
            void set_exception(std::exception_ptr p)
            {
                Lock lock(mutex_);
                delegate_promise_.set_exception(p);
                extra_state_->FireContinuation();
            }

        private:
            std::promise<R> delegate_promise_;
            std::shared_ptr<internal::State<void, E>> extra_state_;
            std::mutex mutex_;
            Lock lock_;
        };

    } // namespace core
} // namespace ara

#endif // _ARA_CORE_PROMISE_H_
