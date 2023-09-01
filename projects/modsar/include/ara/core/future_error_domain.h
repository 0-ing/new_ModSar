/**
 * \copyright BCSC all rights resvered
 * \author JJL
 * \date 2023/7/22
 */

#ifndef _ARA_CORE_FUTURE_ERROR_DOMAIN_H_
#define _ARA_CORE_FUTURE_ERROR_DOMAIN_H_

#include <cstdint>
#include <utility>

#include "ara/core/error_domain.h"
#include "ara/core/error_code.h"
#include "ara/core/exception.h"

namespace ara {
namespace core {

/**
 * \brief 指定调用Future:：get或Future::GetResult时可能发生的内部错误的类型。
 * 这些定义等效于std:：Future_erc中的定义。
 *
 * @traceid{SWS_CORE_00400}
 * @traceid{SWS_CORE_10900}
 * @traceid{SWS_CORE_10902}
 * @traceid{SWS_CORE_10903}
 * @traceid{SWS_CORE_10999}
 */
enum class future_errc : ara::core::ErrorDomain::CodeType {
    broken_promise = 101,            ///< the asynchronous task abandoned its shared state
    future_already_retrieved = 102,  ///< the contents of the shared state were already accessed
    promise_already_satisfied = 103, ///< attempt to store a value into the shared state twice
    no_state = 104,                  ///< attempt to access Promise or Future without an associated state
};

/**
 * \brief Exception type thrown by Future/Promise classes
 *
 * @traceid{SWS_CORE_00411}
 * @traceid{SWS_CORE_10910}
 * @traceid{SWS_CORE_10911}
 * @traceid{SWS_CORE_10999}
 */
class FutureException : public Exception {
public:
    /**
     * \brief Construct a new future exception object containing an Error object.
     * \param err  the ErrorCode
     *
     * @traceid{SWS_CORE_00412}
     */
    explicit FutureException(ErrorCode err) noexcept : Exception(std::move(err)) {}
};

/**
 * @brief Error domain for errors originating from classes Future and Promise.
 * @domainid{0x8000'0000'0000'0013}
 *
 * @traceid{SWS_CORE_00011}
 * @traceid{SWS_CORE_00421}
 * @traceid{SWS_CORE_10400}
 * @traceid{SWS_CORE_10930}
 * @traceid{SWS_CORE_10931}
 * @traceid{SWS_CORE_10932}
 * @traceid{SWS_CORE_10950}
 * @traceid{SWS_CORE_10999}
 */
class FutureErrorDomain final : public ErrorDomain {
    /// @traceid{SWS_CORE_00010}
    constexpr static ErrorDomain::IdType kId = 0x8000000000000013;

public:
    /**
     * \brief 错误代码值枚举的别名
     * 
     * @traceid{SWS_CORE_00431}
     * @traceid{SWS_CORE_10933}
     */
    using Errc = future_errc;

    /**
     * \brief 异常基类的别名
     * 
     * @traceid{SWS_CORE_00432}
     * @traceid{SWS_CORE_10934}
     */
    using Exception = FutureException;
    
    /**
     * \brief 默认构造函数
     * 
     * @traceid{SWS_CORE_00441}
     * @traceid{SWS_CORE_00013}
     */
    constexpr FutureErrorDomain() noexcept : ErrorDomain(kId) {}

    /**
     * \brief 返回此错误域的“shortname”ApApplicationErrorDomain.SN。
     * \returns "Future"
     * 
     * @traceid{SWS_CORE_00442}
     * @traceid{SWS_CORE_00013}
     * @traceid{SWS_CORE_10951}
     */
    char const* Name() const noexcept override { return "Future"; }
    
    /**
     * \brief 将错误代码值转换为文本消息。
     * \param errorcode 错误代码值 
     * \returns 文本消息，从不为nullptr
     * 
     * @traceid{SWS_CORE_00443}
     */
    char const* Message(ErrorDomain::CodeType errorCode) const noexcept override {
        Errc const code = static_cast<Errc>(errorCode);
        switch (code) {
            case Errc::broken_promise:
                return "broken promise";
            case Errc::future_already_retrieved:
                return "future already retrieved";
            case Errc::promise_already_satisfied:
                return "promise already satisfied";
            case Errc::no_state:
                return "no state associated with this future";
            default:
                return "unknown future error";
        }
    }
    /**
     * \brief 引发与给定ErrorCode对应的异常类型
     * \param errorcode ErrorCode实例
     *
     * 
     * @traceid{SWS_CORE_00444}
     */
    void ThrowAsException(ErrorCode const& errorCode) const noexcept(false) override {
        /// @traceid{SWS_CORE_10953}
        ThrowOrTerminate<Exception>(errorCode);
    }
};

namespace internal {
constexpr FutureErrorDomain g_futureErrorDomain;
}  // namespace internal

/**
 * \brief 获取对单个全局FutureErrorDomain实例的引用。
 * \returns 引用FutureErrorDomain实例
 *
 * @traceid{SWS_CORE_00480}
 * @traceid{SWS_CORE_10980}
 * @traceid{SWS_CORE_10981}
 * @traceid{SWS_CORE_10982}
 * @traceid{SWS_CORE_10999}
 */
constexpr ErrorDomain const& GetFutureErrorDomain() noexcept { return internal::g_futureErrorDomain; }

/**
 * \brief 使用给定的支持数据类型为FutureErrorDomain创建一个新的ErrorCode。
 * \param code future-erc中的枚举值
 * \param data 供应商定义的补充价值
 * \returns 新的ErrorCode实例
 *
 * @traceid{SWS_CORE_00490}
 * @traceid{SWS_CORE_10990}
 * @traceid{SWS_CORE_10991}
 * @traceid{SWS_CORE_10999}
 */
constexpr ErrorCode MakeErrorCode(future_errc code, ErrorDomain::SupportDataType data) noexcept {
    return ErrorCode(static_cast<ErrorDomain::CodeType>(code), GetFutureErrorDomain(), data);
}
}  // namespace core
}  // namespace ara

#endif  // _ARA_CORE_FUTURE_ERROR_DOMAIN_H_
