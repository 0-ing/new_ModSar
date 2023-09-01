/**
 * \copyright BCSC all rights resvered
 * \author JJL
 * \date 2023/7/22
 */

#ifndef _ARA_CORE_FUTURE_H_
#define _ARA_CORE_FUTURE_H_

#include <chrono>
#include <future>
#include <system_error>
#include <cassert>
#include <utility>
#include <memory>

#include "ara/core/error_code.h"
#include "ara/core/result.h"
#include "ara/core/core_error_domain.h"
#include "ara/core/exception.h"
#include "ara/core/future_error_domain.h"

namespace ara
{
    namespace core
    {

        /* Forward declaration */
        template <typename, typename>
        class Future;

        namespace internal
        {
            // Re-implementation of C++14's std::enable_if_t
            template <bool Condition, typename U = void>
            using enable_if_t = typename std::enable_if<Condition, U>::type;

            template <typename T>
            class unique_function : public std::function<T>
            {
                template <typename Fn, typename En = void>
                struct wrapper;

                // specialization for MoveConstructible-only Fn
                template <typename Fn>
                struct wrapper<Fn, enable_if_t<!std::is_copy_constructible<Fn>::value && std::is_move_constructible<Fn>::value>>
                {
                    Fn fn;

                    explicit wrapper(Fn &&fn) : fn(std::forward<Fn>(fn)) {}

                    wrapper(wrapper &&) = default;
                    wrapper &operator=(wrapper &&) = default;

                    // these two functions are instantiated by std::function
                    // and are never called
                    wrapper(const wrapper &rhs) : fn(const_cast<Fn &&>(rhs.fn))
                    {
                        throw 0;
                    } // hack to initialize fn for non-DefaultContructible types
                    wrapper &operator=(wrapper &) { throw 0; }

                    template <typename... Args>
                    auto operator()(Args &&...args) -> decltype(fn(std::forward<Args>(args)...))
                    {
                        return fn(std::forward<Args>(args)...);
                    }
                };

                using base = std::function<T>;

            public:
                unique_function() noexcept = default;

                template <typename Fn>
                unique_function &operator=(Fn &&f)
                {
                    base::operator=(wrapper<Fn>{std::forward<Fn>(f)});
                    return *this;
                }

                using base::operator();
            };
            /**
             * \brief 类状态在回调时维护操作（Set、Fire或Execute）
             *
             * \private
             */

            template <typename T, typename E = ara::core::ErrorCode>
            class State : public std::enable_shared_from_this<State<T, E>>
            {
            public:
                State() {}

                ~State() {}

                /**
                 * \brief 执行存储的延续。
                 */
                void FireContinuation()
                {
                    if (HasContinuation())
                    {
                        unique_continuation_(get_future());
                    }
                }

                /**
                 * \brief 设置连续性。
                 *
                 * \param func 要设置的延续。
                 * \param predecessor_future 要作为参数传递给continuation的Future对象。
                 */
                template <typename Func, class Predecessor>
                void SetContinuation(Func &&func, Predecessor &predecessor_future)
                {
                    unique_continuation_ = std::move(func);
                    delegate_future = std::move(predecessor_future.delegate_future_);
                }

                std::mutex mutex_;

            private:
                /**
                 * \brief 如果存在延续，则返回。
                 *
                 * \return 如果存在延续，则为true，否则为false。
                 */
                bool HasContinuation() const { return static_cast<bool>(unique_continuation_); }

                ara::core::Future<T, E> get_future()
                {
                    return ara::core::Future<T, E>(std::move(delegate_future), this->shared_from_this());
                }

                unique_function<void(ara::core::Future<T, E>)> unique_continuation_;
                std::future<ara::core::Result<T, E>> delegate_future;
            };

        } // namespace internal

        /* Forward declaration */
        template <typename, typename>
        class Promise;
        /**
         * \brief Specifies the state of a Future as returned by wait_for() and wait_until().
         * 
         * 这些定义相当于std:：future_status中的定义。但是，这里没有等效于std:：future_status：：deferred的项。
         * 枚举项的数值是由实现定义的。
         * 
         * @traceid{SWS_CORE_00361}
         */
        enum class future_status : uint8_t
        {
            kReady = 1, ///< the shared state is ready
            kTimeout,   ///< the shared state did not become ready before the specified timeout has passed
        };

        inline std::ostream &operator<<(std::ostream &out, FutureException const &ex)
        {
            return (out << "FutureException: " << ex.Error() << " (" << ex.what() << ")");
        }

        /**
         * \brief 提供特定于ara：：core的Future操作，以收集异步调用的结果。
         * \tparam a 值的类型
         * \tparam E 错误的类型
         * 
         * 它的大部分功能都委托给std:：future，并且所有类似std:：future的方法都保证行为相同。
         * 如果实例的valid（）成员函数返回true，则保证所有其他方法都能处理该函数例子
         * 否则，它们可能会失败，无论有没有例外。引发的异常类型为std:：future_error。
         * 当使用move构造函数或move移动future时，通常会发生具有无效future的情况分配
         * @traceid{SWS_CORE_00321}
         */
        template <typename T, typename E = ErrorCode>
        class Future final
        {
            using R = Result<T, E>;
            using Lock = std::unique_lock<std::mutex>;

        public:
            /// Alias type for T
            using ValueType = T;
            /// Alias type for the Promise type collaborating with this Future type
            using PromiseType = Promise<T, E>;
            /// Alias type for the future_status type
            using Status = future_status;

            template <typename, typename>
            friend class Future;

            /**
             * \brief 默认构造函数
             * 
             * @traceid{SWS_CORE_00322}
             */
            Future() noexcept = default;

            /**
             * \brief 从另一个实例移动构造。
             * 
             * \param other  另一个实例
             * 
             * @traceid{SWS_CORE_00323}
             */
            Future(Future &&other) noexcept
                : delegate_future_(std::move(other.delegate_future_)), extra_state_(std::move(other.extra_state_)), lock_(other.mutex_)
            {
                lock_.unlock();
            }

            /**
             * \brief 从另一个实例中指定的另一个instanceMove。
             * 
             * \param other  另一个实例
             * \returns *this
             * 
             * @traceid{SWS_CORE_00325}
             */
            Future &operator=(Future &&other) noexcept
            {
                if (this != &other)
                {
                    Lock lhsLock(mutex_, std::defer_lock);
                    Lock rhsLock(other.mutex_, std::defer_lock);
                    std::lock(lhsLock, rhsLock);

                    delegate_future_ = std::move(other.delegate_future_);
                    extra_state_ = std::move(other.extra_state_);
                }
                return *this;
            }

            /**
             * \brief Future对象的析构函数
             * 
             * 
             * @traceid{SWS_CORE_00333}
             */
            ~Future() noexcept = default;

            /**
             * \brief 应禁用复制构造函数。
             * 
             * 
             * @traceid{SWS_CORE_00334}
             */
            Future(Future const &) = delete;

             /**
             * \brief 应禁用复制分配操作员。
             * 
             * 
             * @traceid{SWS_CORE_00335}
             */
            Future &operator=(Future const &) = delete;

            /**
             * 停止定义noexcept，以便将其他异常扩展到更高级别并修复一些崩溃
             * 
             * \brief 获取结果。
             * 
             * 获取结果。与Get（）类似，此调用会阻塞，直到值或错误可用。但是，此调用永远不会抛出异常。
             * 
             * \returns 具有值或错误的结果
             * 
             * error域：error已放入@note中的错误。如果忽略此函数的返回值，
             * 则添加[[nodiscard]]以发出编译器警告是c++14不支持的
             * @traceid{SWS_CORE_00336}
             */
            Result<T, E> GetResult() noexcept
            {
#ifndef ARA_NO_EXCEPTIONS
                try
                {
                    return delegate_future_.get();
                }
                catch (std::future_error const &ex)
                {
                    std::error_code const &ec = ex.code();
                    future_errc err;
                    if (ec == std::future_errc::broken_promise)
                    {
                        err = future_errc::broken_promise;
                    }
                    else if (ec == std::future_errc::future_already_retrieved)
                    {
                        err = future_errc::future_already_retrieved;
                    }
                    else if (ec == std::future_errc::promise_already_satisfied)
                    {
                        err = future_errc::promise_already_satisfied;
                    }
                    else if (ec == std::future_errc::no_state)
                    {
                        err = future_errc::no_state;
                    }
                    else
                    {
                        // Should rather use a vendor/demonstrator-specific ErrorDomain here?
                        return R::FromError(CoreErrc::kInvalidArgument);
                    }
                    return R::FromError(err);
                }
                catch (std::exception &e)
                {
                    // catching exception from delegate_future_.get()
                    std::cerr << "Exception in ara::core::Future<T,E>::GetResult(): " << e.what() << '\n';
                    future_errc err = future_errc::broken_promise;
                    return R::FromError(err);
                }
#else
                // TODO: Somehow query the future object whether it contains an exception,
                // and if so, translate it into a Result<...>
                // This does not seem possible with C++14's std::future, because it lacks boost::future's
                // has_exception() method. Unfortunately, boost::future cannot be compiled with
                // -fno-exceptions, so that one is out as well.
                return delegate_future_.get();
#endif
            }

#ifndef ARA_NO_EXCEPTIONS
            
            /**
             * \brief 获取值。
             * 
             * 此调用将被阻止，直到结果或异常可用为止。
             * 当编译器工具链不支持C++异常时，此函数不参与重载解析。
             * 
             * \return T型值
             * @error域：error已通过Promise放入相应Promise的错误：：SetError
             * @traceid{SWS_CORE_00326}
             */
            /// @brief 获取值。
            T get() { return GetResult().ValueOrThrow(); }
#endif
            /**
             * \brief 检查Future是否有效，即它是否具有共享状态。
             * 
             * \return 如果Future可用，则为true，否则为false
             * 
             * 
             * @traceid{SWS_CORE_00327}
             */
            bool valid() const noexcept { return delegate_future_.valid(); }
            
            /**
             * \brief 等待值或错误可用。
             * 
             * 此方法返回后，保证get（）不阻塞，is_ready（）将返回true。
             * 
             * @traceid{SWS_CORE_00328}
             */
            void wait() const { delegate_future_.wait(); }

            /**
             * \brief 等待给定的时间段，或者直到值或错误可用。
             * 
             * 如果Future准备就绪或达到超时，该方法将返回。
             * 
             * \param timeoutDuration 等待的最长持续时间
             * \returns tatus，指示超时是否命中或值是否可用。在std:：future_status:：deferred-to exclude-Wreturn
             * 值编译器警告的情况下，返回0。调用方负责验证非零返回值，并在该函数返回0时记录错误。
             * @traceid{SWS_CORE_00329}
             */
            template <typename Rep, typename Period>
            future_status wait_for(std::chrono::duration<Rep, Period> const &timeout_duration) const
            {
                switch (delegate_future_.wait_for(timeout_duration))
                {
                case std::future_status::ready:
                    return future_status::kReady;
                case std::future_status::timeout:
                    return future_status::kTimeout;
                default:
                    assert(!"this std::future_status should not occur in our setup");
                    return static_cast<future_status>(0);
                }
            }
            /**
             * \brief 等待，直到给定的时间，或者直到值或错误可用。
             * 
             * 如果Future准备就绪或达到超时，该方法将返回。
             * 
             * \param deadline 等待的最新时间点
             * \returns 状态，指示是否已达到时间或某个值是否可用。在std：：future_status：：deferred
             * 以消除-Wreturn值编译器警告的情况下返回0。调用方负责验证非零返回值，并在该函数返回0时记录错误
             * 
             * @traceid{SWS_CORE_00330}
             */
            template <typename Clock, typename Duration>
            future_status wait_until(std::chrono::time_point<Clock, Duration> const &deadline) const
            {
                switch (delegate_future_.wait_until(deadline))
                {
                case std::future_status::ready:
                    return future_status::kReady;
                case std::future_status::timeout:
                    return future_status::kTimeout;
                default:
                    assert(!"this std::future_status should not occur in our setup");
                    return static_cast<future_status>(0);
                }
            }

            /// \brief Trait that detects whether a type is a Future<...>
            template <typename U>
            struct is_future : std::false_type
            {
            };

            template <typename U, typename G>
            struct is_future<Future<U, G>> : std::true_type
            {
            };

            /// \brief Trait that detects whether a type is a Result<...>
            template <typename U>
            struct is_result : std::false_type
            {
            };

            template <typename U, typename G>
            struct is_result<Result<U, G>> : std::true_type
            {
            };

            template <typename F>
            using CallableReturnsResult = std::enable_if_t<is_result<std::result_of_t<std::decay_t<F>(Future<T, E>)>>::value>;

            template <typename F>
            using CallableReturnsFuture = std::enable_if_t<is_future<std::result_of_t<std::decay_t<F>(Future<T, E>)>>::value>;

            template <typename F>
            using CallableReturnsValueType = std::enable_if_t<!is_future<std::result_of_t<std::decay_t<F>(Future<T, E>)>>::value && !is_result<std::result_of_t<std::decay_t<F>(Future<T, E>)>>::value && !std::is_void<std::result_of_t<std::decay_t<F>(Future<T, E>)>>::value>;

            template <typename F>
            using CallableReturnsVoid = std::enable_if_t<std::is_void<std::result_of_t<std::decay_t<F>(Future<T, E>)>>::value>;

            /// Alias type for T
            using value_type = T;
            /// Alias type for E
            using error_type = E;

        private:
            template <class Successor>
            void handle_future_error(Successor &successor_promise_, const std::future_error &ex)
            {
                std::error_code const &ec = ex.code();
                future_errc err;
                switch (ec.value())
                {
                case static_cast<int>(std::future_errc::broken_promise):
                    err = future_errc::broken_promise;
                    break;
                case static_cast<int>(std::future_errc::future_already_retrieved):
                    err = future_errc::future_already_retrieved;
                    break;
                case static_cast<int>(std::future_errc::promise_already_satisfied):
                    err = future_errc::promise_already_satisfied;
                    break;
                case static_cast<int>(std::future_errc::no_state):
                    err = future_errc::no_state;
                    break;
                default:
                    successor_promise_.SetError(CoreErrc::kInvalidArgument);
                    return;
                }
                successor_promise_.SetError(err);
            }

            template <class Successor>
            void handle_exception(Successor &successor_promise_, const std::exception &ex)
            {
                std::cerr << "Exception in continuation attached to then(): " << ex.what() << '\n';
                future_errc err = future_errc::broken_promise;
                successor_promise_.SetError(err);
            }

            template <typename F,
                      class Successor,
                      class Predecessor,
                      typename std::enable_if_t<!std::is_void<
                          typename std::result_of_t<std::decay_t<F>(Future<T, E>)>::value_type>::value> * = nullptr>
            void fulfill_promise_future(F &&func_, Successor &successor_promise_, Future<Predecessor, E> &predecessor_future_)
            {
                try
                {
                    using U = std::result_of_t<std::decay_t<F>(Future<T, E>)>;

                    using T3 = typename U::value_type;
                    using E3 = typename U::error_type;

                    // callback returns another future: inner_future, when inner_future is done, next_future can be done
                    Future<T3, E3> inner_future;
                    inner_future = std::forward<F>(func_)(std::move(predecessor_future_));

                    if (!inner_future.valid())
                    {
                        successor_promise_.SetError(ara::core::future_errc::broken_promise);
                        return;
                    }

                    std::unique_lock<std::mutex> inner_lock(inner_future.extra_state_->mutex_);

                    auto inner_continuation = [outer_promise = std::move(successor_promise_)](Future<T3, E3> inner_future) mutable
                    {
                        auto result = inner_future.GetResult();
                        if (result.HasValue())
                        {
                            outer_promise.set_value(result.Value());
                        }
                        else
                        {
                            outer_promise.SetError(result.Error());
                        }
                    };

                    if (inner_future.is_ready())
                    {
                        inner_lock.unlock();
                        inner_continuation(std::move(inner_future));
                    }
                    else
                    {
                        inner_future.extra_state_->SetContinuation(std::move(inner_continuation), inner_future);
                    }
                }
                catch (std::future_error const &ex)
                {
                    handle_future_error(successor_promise_, ex);
                }
                catch (std::exception const &e)
                {
                    handle_exception(successor_promise_, e);
                }
            }

            template <typename F,
                      class Successor,
                      class Predecessor,
                      typename std::enable_if_t<
                          std::is_void<typename std::result_of_t<std::decay_t<F>(Future<T, E>)>::value_type>::value> * = nullptr>
            void fulfill_promise_future(F &&func_, Successor &successor_promise_, Future<Predecessor, E> &predecessor_future_)
            {
                try
                {
                    using U = std::result_of_t<std::decay_t<F>(Future<T, E>)>;

                    using T3 = typename U::value_type;
                    using E3 = typename U::error_type;

                    // callback returns another future: inner_future, when inner_future is done, next_future can be done
                    Future<T3, E3> inner_future;
                    inner_future = std::forward<F>(func_)(std::move(predecessor_future_));

                    if (!inner_future.valid())
                    {
                        successor_promise_.SetError(ara::core::future_errc::broken_promise);
                        return;
                    }

                    std::unique_lock<std::mutex> inner_lock(inner_future.extra_state_->mutex_);

                    auto inner_continuation = [outer_promise = std::move(successor_promise_)](Future<T3, E3> inner_future) mutable
                    {
                        auto result = inner_future.GetResult();
                        if (result.HasValue())
                        {
                            outer_promise.set_value();
                        }
                        else
                        {
                            outer_promise.SetError(result.Error());
                        }
                    };

                    if (inner_future.is_ready())
                    {
                        inner_lock.unlock();
                        inner_continuation(std::move(inner_future));
                    }
                    else
                    {
                        inner_future.extra_state_->SetContinuation(std::move(inner_continuation), inner_future);
                    }
                }
                catch (std::future_error const &ex)
                {
                    handle_future_error(successor_promise_, ex);
                }
                catch (std::exception const &e)
                {
                    handle_exception(successor_promise_, e);
                }
            }

            template <typename F,
                      class Successor,
                      class Predecessor,
                      typename std::enable_if_t<!std::is_void<
                          typename std::result_of_t<std::decay_t<F>(Future<T, E>)>::value_type>::value> * = nullptr>
            void fulfill_promise_result(F &&func_, Successor &successor_promise_, Future<Predecessor, E> &predecessor_future_)
            {
                try
                {
                    auto result = std::forward<F>(func_)(std::move(predecessor_future_));
                    if (result.HasValue())
                    {
                        successor_promise_.set_value(result.Value());
                    }
                    else
                    {
                        successor_promise_.SetError(result.Error());
                    }
                }
                catch (std::future_error const &ex)
                {
                    handle_future_error(successor_promise_, ex);
                }
                catch (std::exception const &e)
                {
                    handle_exception(successor_promise_, e);
                }
            }

            template <typename F,
                      class Successor,
                      class Predecessor,
                      typename std::enable_if_t<
                          std::is_void<typename std::result_of_t<std::decay_t<F>(Future<T, E>)>::value_type>::value> * = nullptr>
            void fulfill_promise_result(F &&func_, Successor &successor_promise_, Future<Predecessor, E> &predecessor_future_)
            {
                try
                {
                    auto result = std::forward<F>(func_)(std::move(predecessor_future_));
                    if (result.HasValue())
                    {
                        successor_promise_.set_value();
                    }
                    else
                    {
                        successor_promise_.SetError(result.Error());
                    }
                }
                catch (std::future_error const &ex)
                {
                    handle_future_error(successor_promise_, ex);
                }
                catch (std::exception const &e)
                {
                    handle_exception(successor_promise_, e);
                }
            }

            template <typename F, class Successor, class Predecessor>
            void
            fulfill_promise_valuetype(F &&func_, Successor &successor_promise_, Future<Predecessor, E> &predecessor_future_)
            {
                try
                {
                    auto result = std::forward<F>(func_)(std::move(predecessor_future_));
                    successor_promise_.set_value(result);
                }
                catch (std::future_error const &ex)
                {
                    handle_future_error(successor_promise_, ex);
                }
                catch (std::exception const &e)
                {
                    handle_exception(successor_promise_, e);
                }
            }

            template <typename F, class Successor, class Predecessor>
            void fulfill_promise_void(F &&func_, Successor &successor_promise_, Future<Predecessor, E> &predecessor_future_)
            {
                try
                {
                    std::forward<F>(func_)(std::move(predecessor_future_));
                    successor_promise_.set_value();
                }
                catch (std::future_error const &ex)
                {
                    handle_future_error(successor_promise_, ex);
                }
                catch (std::exception const &e)
                {
                    handle_exception(successor_promise_, e);
                }
            }

        public:
            /// @brief Register a callable that gets called when the Future becomes ready.
            ///
            /// When @a func is called, it is guaranteed that get() will not block.
            ///
            /// @a func may be called in the context of this call or in the context of Promise::set_value()
            /// or Promise::SetError() or somewhere else.
            ///
            /// The return type of @a then depends on the return type of @a func (aka continuation).
            ///
            /// @param func  a callable to register to get the Future result or an exception
            /// @returns a new Future instance for the result of the continuation
            ///
            /// @traceid{SWS_CORE_00331}
            template <typename F, typename = CallableReturnsFuture<F>>
            auto then(F &&func) -> std::result_of_t<std::decay_t<F>(Future<T, E>)>
            {
                Lock lock(extra_state_->mutex_);

                using U = std::result_of_t<std::decay_t<F>(Future<T, E>)>;

                using T2 = typename U::value_type;
                using E2 = typename U::error_type;

                Promise<T2, E2> next_promise;
                auto next_future = next_promise.get_future();

                Future<T, E> &predecessor_future = *this;

                // creates a continuation which fulfills the promise
                auto continuation = [promise = std::move(next_promise),
                                     func(std::forward<F>(func))](Future<T, E> predecessor_future) mutable
                {
                    predecessor_future.fulfill_promise_future(func, promise, predecessor_future);
                };

                if (predecessor_future.is_ready())
                {
                    // the state is ready, invoke the continuation immediately
                    // unlock here so that Future passed to the continuation below doesn't block in .get()
                    lock.unlock();
                    continuation(std::move(predecessor_future));
                }
                else
                {
                    // the state is not yet ready, continuation is stored
                    // continuation is invoked in the context of Promise::set_value() or Promise::SetError()
                    extra_state_->SetContinuation(std::move(continuation), predecessor_future);
                }

                return next_future;
            }

            /// @brief Register a callable that gets called when the Future becomes ready.
            ///
            /// When @a func is called, it is guaranteed that get() will not block.
            ///
            /// @a func may be called in the context of this call or in the context of Promise::set_value()
            /// or Promise::SetError() or somewhere else.
            ///
            /// The return type of @a then depends on the return type of @a func (aka continuation).
            ///
            /// @param func  a callable to register to get the Future result or an exception
            /// @returns a new Future instance for the result of the continuation
            ///
            /// @traceid{SWS_CORE_00331}
            template <typename F, typename = CallableReturnsResult<F>>
            auto then(F &&func) -> Future<typename std::result_of_t<std::decay_t<F>(Future<T, E>)>::value_type,
                                          typename std::result_of_t<std::decay_t<F>(Future<T, E>)>::error_type>
            {
                Lock lock(extra_state_->mutex_);

                using U = std::result_of_t<std::decay_t<F>(Future<T, E>)>;

                using T2 = typename U::value_type;
                using E2 = typename U::error_type;

                Promise<T2, E2> next_promise;
                auto next_future = next_promise.get_future();

                Future<T, E> &predecessor_future = *this;

                // creates a continuation which fulfills the promise
                auto continuation = [promise = std::move(next_promise),
                                     func(std::forward<F>(func))](Future<T, E> predecessor_future) mutable
                {
                    predecessor_future.fulfill_promise_result(func, promise, predecessor_future);
                };

                if (predecessor_future.is_ready())
                {
                    // the state is ready, invoke the continuation immediately
                    // unlock here so that Future passed to the continuation below doesn't block in .get()
                    lock.unlock();
                    continuation(std::move(predecessor_future));
                }
                else
                {
                    // the state is not yet ready, continuation is stored
                    // continuation is invoked in the context of Promise::set_value() or Promise::SetError()
                    extra_state_->SetContinuation(std::move(continuation), predecessor_future);
                }

                return next_future;
            }

            /// @brief Register a callable that gets called when the Future becomes ready.
            ///
            /// When @a func is called, it is guaranteed that get() will not block.
            ///
            /// @a func may be called in the context of this call or in the context of Promise::set_value()
            /// or Promise::SetError() or somewhere else.
            ///
            /// The return type of @a then depends on the return type of @a func (aka continuation).
            ///
            /// @param func  a callable to register to get the Future result or an exception
            /// @returns a new Future instance for the result of the continuation
            ///
            /// @traceid{SWS_CORE_00331}
            template <typename F, typename = CallableReturnsValueType<F>>
            auto then(F &&func) -> Future<std::result_of_t<std::decay_t<F>(Future<T, E>)>, E>
            {
                Lock lock(extra_state_->mutex_);

                using U = std::result_of_t<std::decay_t<F>(Future<T, E>)>;

                Promise<U, E> next_promise;
                auto next_future = next_promise.get_future();

                Future<T, E> &predecessor_future = *this;

                // creates a continuation which fulfills the promise
                auto continuation = [promise = std::move(next_promise),
                                     func(std::forward<F>(func))](Future<T, E> predecessor_future) mutable
                {
                    predecessor_future.fulfill_promise_valuetype(func, promise, predecessor_future);
                };

                if (predecessor_future.is_ready())
                {
                    // the state is ready, invoke the continuation immediately
                    // unlock here so that Future passed to the continuation below doesn't block in .get()
                    lock.unlock();
                    continuation(std::move(predecessor_future));
                }
                else
                {
                    // the state is not yet ready, continuation is stored
                    // continuation is invoked in the context of Promise::set_value() or Promise::SetError()
                    extra_state_->SetContinuation(std::move(continuation), predecessor_future);
                }

                return next_future;
            }

            /// @brief Register a callable that gets called when the Future becomes ready.
            ///
            /// When @a func is called, it is guaranteed that get() will not block.
            ///
            /// @a func may be called in the context of this call or in the context of Promise::set_value()
            /// or Promise::SetError() or somewhere else.
            ///
            /// The return type of @a then depends on the return type of @a func (aka continuation).
            ///
            /// @param func  a callable to register to get the Future result or an exception
            /// @returns a new Future instance for the result of the continuation
            ///
            /// @traceid{SWS_CORE_00331}
            template <typename F, typename = CallableReturnsVoid<F>>
            auto then(F &&func) -> Future<void, E>
            {
                Lock lock(extra_state_->mutex_);

                Promise<void, E> next_promise;
                auto next_future = next_promise.get_future();

                Future<T, E> &predecessor_future = *this;

                // creates a continuation which fulfills the promise
                auto continuation = [promise = std::move(next_promise),
                                     func(std::forward<F>(func))](Future<T, E> predecessor_future) mutable
                {
                    predecessor_future.fulfill_promise_void(func, promise, predecessor_future);
                };

                if (predecessor_future.is_ready())
                {
                    // the state is ready, invoke the continuation immediately
                    // unlock here so that Future passed to the continuation below doesn't block in .get()
                    lock.unlock();
                    continuation(std::move(predecessor_future));
                }
                else
                {
                    // the state is not yet ready, continuation is stored
                    // continuation is invoked in the context of Promise::set_value() or Promise::SetError()
                    extra_state_->SetContinuation(std::move(continuation), predecessor_future);
                }

                return next_future;
            }

            /**
             * \brief 返回异步操作是否已完成。
             * 
             * 如果is_ready（）返回true，则保证get（）和wait调用不会阻塞。
             * 
             * \return 如果Future包含值或错误，则为true，否则为false
             * @traceid{SWS_CORE_00332}
             */
            bool is_ready() const
            {
                return std::future_status::ready == delegate_future_.wait_for(std::chrono::seconds::zero());
            }

        private:
            /**
             * \brief 从给定的std:：Future和指向额外状态的指针构造Future。
             * \param delegate_future std::future instance
             * \param extra_state state that is shared with the Promise
             * @traceid{SWS_CORE_00332}
             */
            Future(std::future<R> &&delegate_future, std::shared_ptr<internal::State<T, E>> extra_state)
                : delegate_future_(std::move(delegate_future)), extra_state_(extra_state) {}

            std::future<R> delegate_future_;
            std::shared_ptr<internal::State<T, E>> extra_state_;
            std::mutex mutex_;
            Lock lock_;
            template <typename, typename>
            friend class Promise;
            friend class internal::State<T, E>;
        };
        /**
         * \brief 类的专业化未来的“无效”价值观
         * \param E  the type of error
         * @traceid{SWS_CORE_06221}
         */
        template <typename E>
        class Future<void, E> final
        {
            using R = Result<void, E>;

            using Lock = std::unique_lock<std::mutex>;

        public:
            /// Alias type for T
            using ValueType = void;
            /// Alias type for the Promise type collaborating with this Future type
            using PromiseType = Promise<void, E>;
            /// Alias type for the future_status type
            using Status = future_status;

            template <typename, typename>
            friend class Future;

            /// @traceid{SWS_CORE_06222}
            /// @copydoc Future::Future
            Future() noexcept = default;

            /// @traceid{SWS_CORE_06233}
            /// @copydoc Future::~Future
            ~Future() noexcept = default;

            /// @traceid{SWS_CORE_06234}
            /// @copydoc Future::Future(const Future&)
            Future(Future const &other) = delete;

            /// @traceid{SWS_CORE_06235}
            /// @copydoc Future::operator=(const Future&)
            Future &operator=(Future const &) = delete;

            /// @traceid{SWS_CORE_06223}
            /// @copydoc Future::Future(Future&&)
            Future(Future &&other) noexcept
                : lock_(other.mutex_), delegate_future_(std::move(other.delegate_future_)), extra_state_(std::move(other.extra_state_))
            {
                lock_.unlock();
            }

            /// @traceid{SWS_CORE_06225}
            /// @copydoc Future::operator=(Future&&)
            Future &operator=(Future &&other) noexcept
            {
                if (this != &other)
                {
                    Lock lhsLock(mutex_, std::defer_lock);
                    Lock rhsLock(other.mutex_, std::defer_lock);
                    std::lock(lhsLock, rhsLock);

                    delegate_future_ = std::move(other.delegate_future_);
                    extra_state_ = std::move(other.extra_state_);
                }
                return *this;
            }

#ifndef ARA_NO_EXCEPTIONS
            /// @traceid{SWS_CORE_06226}
            /// @copydoc Future::get
            void get() { return GetResult().ValueOrThrow(); }
#endif

            /// @traceid{SWS_CORE_06236}
            /// @copydoc Future::GetResult
            Result<void, E> GetResult() noexcept
            {
#ifndef ARA_NO_EXCEPTIONS
                try
                {
                    return delegate_future_.get();
                }
                catch (std::future_error const &ex)
                {
                    std::error_code const &ec = ex.code();
                    future_errc err;
                    if (ec == std::future_errc::broken_promise)
                    {
                        err = future_errc::broken_promise;
                    }
                    else if (ec == std::future_errc::future_already_retrieved)
                    {
                        err = future_errc::future_already_retrieved;
                    }
                    else if (ec == std::future_errc::promise_already_satisfied)
                    {
                        err = future_errc::promise_already_satisfied;
                    }
                    else if (ec == std::future_errc::no_state)
                    {
                        err = future_errc::no_state;
                    }
                    else
                    {
                        // Should rather use a vendor/demonstrator-specific ErrorDomain here?
                        return R::FromError(CoreErrc::kInvalidArgument);
                    }
                    return R::FromError(err);
                }
                catch (std::exception &e)
                {
                    // catching exception from delegate_future_.get()
                    std::cerr << "Exception in ara::core::Future<void,E>::GetResult(): " << e.what() << '\n';
                    future_errc err = future_errc::broken_promise;
                    return R::FromError(err);
                }
#else
                return delegate_future_.get();
#endif
            }

            /// @traceid{SWS_CORE_06227}
            /// @copydoc Future::valid
            bool valid() const noexcept { return delegate_future_.valid(); }

            /// @traceid{SWS_CORE_06228}
            /// @copydoc Future::wait
            void wait() const { delegate_future_.wait(); }

            /// @traceid{SWS_CORE_06229}
            /// @copydoc Future::wait_for
            template <typename Rep, typename Period>
            future_status wait_for(std::chrono::duration<Rep, Period> const &timeoutDuration) const
            {
                switch (delegate_future_.wait_for(timeoutDuration))
                {
                case std::future_status::ready:
                    return future_status::kReady;
                case std::future_status::timeout:
                    return future_status::kTimeout;
                default:
                    assert(!"this std::future_status should not occur in our setup");
                    return static_cast<future_status>(0);
                }
            }

            /// @traceid{SWS_CORE_06230}
            /// @copydoc Future::wait_until
            template <typename Clock, typename Duration>
            future_status wait_until(std::chrono::time_point<Clock, Duration> const &deadline) const
            {
                switch (delegate_future_.wait_until(deadline))
                {
                case std::future_status::ready:
                    return future_status::kReady;
                case std::future_status::timeout:
                    return future_status::kTimeout;
                default:
                    assert(!"this std::future_status should not occur in our setup");
                    return static_cast<future_status>(0);
                }
            }

            /// @brief Trait that detects whether a type is a Future<...>
            template <typename U>
            struct is_future : std::false_type
            {
            };

            template <typename U, typename G>
            struct is_future<Future<U, G>> : std::true_type
            {
            };

            /// @brief Trait that detects whether a type is a Result<...>
            template <typename U>
            struct is_result : std::false_type
            {
            };

            template <typename U, typename G>
            struct is_result<Result<U, G>> : std::true_type
            {
            };

            template <typename F>
            using CallableReturnsResult = std::enable_if_t<is_result<std::result_of_t<std::decay_t<F>(Future<void, E>)>>::value>;

            template <typename F>
            using CallableReturnsFuture = std::enable_if_t<is_future<std::result_of_t<std::decay_t<F>(Future<void, E>)>>::value>;

            template <typename F>
            using CallableReturnsValueType = std::enable_if_t<!is_future<std::result_of_t<std::decay_t<F>(Future<void, E>)>>::value && !is_result<std::result_of_t<std::decay_t<F>(Future<void, E>)>>::value && !std::is_void<std::result_of_t<std::decay_t<F>(Future<void, E>)>>::value>;

            template <typename F>
            using CallableReturnsVoid = std::enable_if_t<std::is_void<std::result_of_t<std::decay_t<F>(Future<void, E>)>>::value>;

            /// Alias type for void
            using value_type = void;
            /// Alias type for E
            using error_type = E;

        private:
            template <class Successor>
            void handle_future_error(Successor &successor_promise_, const std::future_error &ex)
            {
                std::error_code const &ec = ex.code();
                future_errc err;
                switch (ec.value())
                {
                case static_cast<int>(std::future_errc::broken_promise):
                    err = future_errc::broken_promise;
                    break;
                case static_cast<int>(std::future_errc::future_already_retrieved):
                    err = future_errc::future_already_retrieved;
                    break;
                case static_cast<int>(std::future_errc::promise_already_satisfied):
                    err = future_errc::promise_already_satisfied;
                    break;
                case static_cast<int>(std::future_errc::no_state):
                    err = future_errc::no_state;
                    break;
                default:
                    successor_promise_.SetError(CoreErrc::kInvalidArgument);
                    return;
                }
                successor_promise_.SetError(err);
            }

            template <class Successor>
            void handle_exception(Successor &successor_promise_, const std::exception &ex)
            {
                std::cerr << "Exception in continuation attached to then(): " << ex.what() << '\n';
                future_errc err = future_errc::broken_promise;
                successor_promise_.SetError(err);
            }

            template <typename F,
                      class Successor,
                      class Predecessor,
                      typename std::enable_if_t<!std::is_void<
                          typename std::result_of_t<std::decay_t<F>(Future<void, E>)>::value_type>::value> * = nullptr>
            void fulfill_promise_future(F &&func_, Successor &successor_promise_, Future<Predecessor, E> &predecessor_future_)
            {
                try
                {
                    using U = std::result_of_t<std::decay_t<F>(Future<void, E>)>;

                    using T3 = typename U::value_type;
                    using E3 = typename U::error_type;

                    // callback returns another future: inner_future, when inner_future is done, next_future can be done
                    Future<T3, E3> inner_future;
                    inner_future = std::forward<F>(func_)(std::move(predecessor_future_));

                    if (!inner_future.valid())
                    {
                        successor_promise_.SetError(ara::core::future_errc::broken_promise);
                        return;
                    }

                    std::unique_lock<std::mutex> inner_lock(inner_future.extra_state_->mutex_);

                    auto inner_continuation = [outer_promise = std::move(successor_promise_)](Future<T3, E3> inner_future) mutable
                    {
                        auto result = inner_future.GetResult();
                        if (result.HasValue())
                        {
                            outer_promise.set_value(result.Value());
                        }
                        else
                        {
                            outer_promise.SetError(result.Error());
                        }
                    };

                    if (inner_future.is_ready())
                    {
                        inner_lock.unlock();
                        inner_continuation(std::move(inner_future));
                    }
                    else
                    {
                        inner_future.extra_state_->SetContinuation(std::move(inner_continuation), inner_future);
                    }
                }
                catch (std::future_error const &ex)
                {
                    handle_future_error(successor_promise_, ex);
                }
                catch (std::exception const &e)
                {
                    handle_exception(successor_promise_, e);
                }
            }

            template <typename F,
                      class Successor,
                      class Predecessor,
                      typename std::enable_if_t<std::is_void<
                          typename std::result_of_t<std::decay_t<F>(Future<void, E>)>::value_type>::value> * = nullptr>
            void fulfill_promise_future(F &&func_, Successor &successor_promise_, Future<Predecessor, E> &predecessor_future_)
            {
                try
                {
                    using U = std::result_of_t<std::decay_t<F>(Future<void, E>)>;

                    using T3 = typename U::value_type;
                    using E3 = typename U::error_type;

                    // callback returns another future: inner_future, when inner_future is done, next_future can be done
                    Future<T3, E3> inner_future;
                    inner_future = std::forward<F>(func_)(std::move(predecessor_future_));

                    if (!inner_future.valid())
                    {
                        successor_promise_.SetError(ara::core::future_errc::broken_promise);
                        return;
                    }

                    std::unique_lock<std::mutex> inner_lock(inner_future.extra_state_->mutex_);

                    auto inner_continuation = [outer_promise = std::move(successor_promise_)](Future<T3, E3> inner_future) mutable
                    {
                        auto result = inner_future.GetResult();
                        if (result.HasValue())
                        {
                            outer_promise.set_value();
                        }
                        else
                        {
                            outer_promise.SetError(result.Error());
                        }
                    };

                    if (inner_future.is_ready())
                    {
                        inner_lock.unlock();
                        inner_continuation(std::move(inner_future));
                    }
                    else
                    {
                        inner_future.extra_state_->SetContinuation(std::move(inner_continuation), inner_future);
                    }
                }
                catch (std::future_error const &ex)
                {
                    handle_future_error(successor_promise_, ex);
                }
                catch (std::exception const &e)
                {
                    handle_exception(successor_promise_, e);
                }
            }

            template <typename F,
                      class Successor,
                      class Predecessor,
                      typename std::enable_if_t<!std::is_void<
                          typename std::result_of_t<std::decay_t<F>(Future<void, E>)>::value_type>::value> * = nullptr>
            void fulfill_promise_result(F &&func_, Successor &successor_promise_, Future<Predecessor, E> &predecessor_future_)
            {
                try
                {
                    auto result = std::forward<F>(func_)(std::move(predecessor_future_));
                    if (result.HasValue())
                    {
                        successor_promise_.set_value(result.Value());
                    }
                    else
                    {
                        successor_promise_.SetError(result.Error());
                    }
                }
                catch (std::future_error const &ex)
                {
                    handle_future_error(successor_promise_, ex);
                }
                catch (std::exception const &e)
                {
                    handle_exception(successor_promise_, e);
                }
            }

            template <typename F,
                      class Successor,
                      class Predecessor,
                      typename std::enable_if_t<std::is_void<
                          typename std::result_of_t<std::decay_t<F>(Future<void, E>)>::value_type>::value> * = nullptr>
            void fulfill_promise_result(F &&func_, Successor &successor_promise_, Future<Predecessor, E> &predecessor_future_)
            {
                try
                {
                    auto result = std::forward<F>(func_)(std::move(predecessor_future_));
                    if (result.HasValue())
                    {
                        successor_promise_.set_value();
                    }
                    else
                    {
                        successor_promise_.SetError(result.Error());
                    }
                }
                catch (std::future_error const &ex)
                {
                    handle_future_error(successor_promise_, ex);
                }
                catch (std::exception const &e)
                {
                    handle_exception(successor_promise_, e);
                }
            }

            template <typename F, class Successor, class Predecessor>
            void
            fulfill_promise_valuetype(F &&func_, Successor &successor_promise_, Future<Predecessor, E> &predecessor_future_)
            {
                try
                {
                    auto result = std::forward<F>(func_)(std::move(predecessor_future_));
                    successor_promise_.set_value(result);
                }
                catch (std::future_error const &ex)
                {
                    handle_future_error(successor_promise_, ex);
                }
                catch (std::exception const &e)
                {
                    handle_exception(successor_promise_, e);
                }
            }

            template <typename F, class Successor, class Predecessor>
            void fulfill_promise_void(F &&func_, Successor &successor_promise_, Future<Predecessor, E> &predecessor_future_)
            {
                try
                {
                    std::forward<F>(func_)(std::move(predecessor_future_));
                    successor_promise_.set_value();
                }
                catch (std::future_error const &ex)
                {
                    handle_future_error(successor_promise_, ex);
                }
                catch (std::exception const &e)
                {
                    handle_exception(successor_promise_, e);
                }
            }

        public:
            /// @brief Register a callable that gets called when the Future becomes ready.
            ///
            /// When @a func is called, it is guaranteed that get() will not block.
            ///
            /// @a func may be called in the context of this call or in the context of Promise::set_value()
            /// or Promise::SetError() or somewhere else.
            ///
            /// The return type of @a then depends on the return type of @a func (aka continuation).
            ///
            /// @param func  a callable to register to get the Future result or an exception
            /// @returns a new Future instance for the result of the continuation
            ///
            /// @traceid{SWS_CORE_06231}
            template <typename F, typename = CallableReturnsFuture<F>>
            auto then(F &&func) -> std::result_of_t<std::decay_t<F>(Future<void, E>)>
            {
                Lock lock(extra_state_->mutex_);

                using U = std::result_of_t<std::decay_t<F>(Future<void, E>)>;

                using T2 = typename U::value_type;
                using E2 = typename U::error_type;

                Promise<T2, E2> next_promise;
                auto next_future = next_promise.get_future();

                Future<void, E> &predecessor_future = *this;

                // creates a continuation which fulfills the promise
                auto continuation = [promise = std::move(next_promise),
                                     func(std::forward<F>(func))](Future<void, E> predecessor_future) mutable
                {
                    predecessor_future.fulfill_promise_future(func, promise, predecessor_future);
                };

                if (predecessor_future.is_ready())
                {
                    // the state is ready, invoke the continuation immediately
                    // unlock here so that Future passed to the continuation below doesn't block in .get()
                    lock.unlock();
                    continuation(std::move(predecessor_future));
                }
                else
                {
                    // the state is not yet ready, continuation is stored
                    // continuation is invoked in the context of Promise::set_value() or Promise::SetError()
                    extra_state_->SetContinuation(std::move(continuation), predecessor_future);
                }

                return next_future;
            }

            /// @brief Register a callable that gets called when the Future becomes ready.
            ///
            /// When @a func is called, it is guaranteed that get() will not block.
            ///
            /// @a func may be called in the context of this call or in the context of Promise::set_value()
            /// or Promise::SetError() or somewhere else.
            ///
            /// The return type of @a then depends on the return type of @a func (aka continuation).
            ///
            /// @param func  a callable to register to get the Future result or an exception
            /// @returns a new Future instance for the result of the continuation
            ///
            /// @traceid{SWS_CORE_06231}
            template <typename F, typename = CallableReturnsResult<F>>
            auto then(F &&func) -> Future<typename std::result_of_t<std::decay_t<F>(Future<void, E>)>::value_type,
                                          typename std::result_of_t<std::decay_t<F>(Future<void, E>)>::error_type>
            {
                Lock lock(extra_state_->mutex_);

                using U = std::result_of_t<std::decay_t<F>(Future<void, E>)>;

                using T2 = typename U::value_type;
                using E2 = typename U::error_type;

                Promise<T2, E2> next_promise;
                auto next_future = next_promise.get_future();

                Future<void, E> &predecessor_future = *this;

                // creates a continuation which fulfills the promise
                auto continuation = [promise = std::move(next_promise),
                                     func(std::forward<F>(func))](Future<void, E> predecessor_future) mutable
                {
                    predecessor_future.fulfill_promise_result(func, promise, predecessor_future);
                };

                if (predecessor_future.is_ready())
                {
                    // the state is ready, invoke the continuation immediately
                    // unlock here so that Future passed to the continuation below doesn't block in .get()
                    lock.unlock();
                    continuation(std::move(predecessor_future));
                }
                else
                {
                    // the state is not yet ready, continuation is stored
                    // continuation is invoked in the context of Promise::set_value() or Promise::SetError()
                    extra_state_->SetContinuation(std::move(continuation), predecessor_future);
                }

                return next_future;
            }

            /// @brief Register a callable that gets called when the Future becomes ready.
            ///
            /// When @a func is called, it is guaranteed that get() will not block.
            ///
            /// @a func may be called in the context of this call or in the context of Promise::set_value()
            /// or Promise::SetError() or somewhere else.
            ///
            /// The return type of @a then depends on the return type of @a func (aka continuation).
            ///
            /// @param func  a callable to register to get the Future result or an exception
            /// @returns a new Future instance for the result of the continuation
            ///
            /// @traceid{SWS_CORE_06231}
            template <typename F, typename = CallableReturnsValueType<F>>
            auto then(F &&func) -> Future<std::result_of_t<std::decay_t<F>(Future<void, E>)>, E>
            {
                Lock lock(extra_state_->mutex_);

                using U = std::result_of_t<std::decay_t<F>(Future<void, E>)>;

                Promise<U, E> next_promise;
                auto next_future = next_promise.get_future();

                Future<void, E> &predecessor_future = *this;

                // creates a continuation which fulfills the promise
                auto continuation = [promise = std::move(next_promise),
                                     func(std::forward<F>(func))](Future<void, E> predecessor_future) mutable
                {
                    predecessor_future.fulfill_promise_valuetype(func, promise, predecessor_future);
                };

                if (predecessor_future.is_ready())
                {
                    // the state is ready, invoke the continuation immediately
                    // unlock here so that Future passed to the continuation below doesn't block in .get()
                    lock.unlock();
                    continuation(std::move(predecessor_future));
                }
                else
                {
                    // the state is not yet ready, continuation is stored
                    // continuation is invoked in the context of Promise::set_value() or Promise::SetError()
                    extra_state_->SetContinuation(std::move(continuation), predecessor_future);
                }

                return next_future;
            }

            /// @brief Register a callable that gets called when the Future becomes ready.
            ///
            /// When @a func is called, it is guaranteed that get() will not block.
            ///
            /// @a func may be called in the context of this call or in the context of Promise::set_value()
            /// or Promise::SetError() or somewhere else.
            ///
            /// The return type of @a then depends on the return type of @a func (aka continuation).
            ///
            /// @param func  a callable to register to get the Future result or an exception
            /// @returns a new Future instance for the result of the continuation
            ///
            /// @traceid{SWS_CORE_06231}
            template <typename F, typename = CallableReturnsVoid<F>>
            auto then(F &&func) -> Future<void, E>
            {
                Lock lock(extra_state_->mutex_);

                Promise<void, E> next_promise;
                auto next_future = next_promise.get_future();

                Future<void, E> &predecessor_future = *this;

                // creates a continuation which fulfills the promise
                auto continuation = [promise = std::move(next_promise),
                                     func(std::forward<F>(func))](Future<void, E> predecessor_future) mutable
                {
                    predecessor_future.fulfill_promise_void(func, promise, predecessor_future);
                };

                if (predecessor_future.is_ready())
                {
                    // the state is ready, invoke the continuation immediately
                    // unlock here so that Future passed to the continuation below doesn't block in .get()
                    lock.unlock();
                    continuation(std::move(predecessor_future));
                }
                else
                {
                    // the state is not yet ready, continuation is stored
                    // continuation is invoked in the context of Promise::set_value() or Promise::SetError()
                    extra_state_->SetContinuation(std::move(continuation), predecessor_future);
                }

                return next_future;
            }

            /// @traceid{SWS_CORE_06232}
            /// @copydoc Future::is_ready
            bool is_ready() const
            {
                return std::future_status::ready == delegate_future_.wait_for(std::chrono::seconds::zero());
            }

        private:
            Future(std::future<R> &&delegate_future, std::shared_ptr<internal::State<void, E>> extra_state)
                : delegate_future_(std::move(delegate_future)), extra_state_(extra_state) {}

            std::mutex mutex_;
            Lock lock_; // this Lock is only necessary for the move constructor
            std::future<R> delegate_future_;
            std::shared_ptr<internal::State<void, E>> extra_state_;
            template <typename, typename>
            friend class Promise;
            friend class internal::State<void, E>;
        };
    } // namespace core
} // namespace ara

#endif // _ARA_CORE_FUTURE_H_
