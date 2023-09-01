/**
 * \copyright BCSC all rights resvered
 * \author JJL
 * \date 2023/7/22
 */
#ifndef _ARA_CORE_OPTIONAL_ERROR_H_
#define _ARA_CORE_OPTIONAL_ERROR_H_

#include <utility>

#include "ara/core/error_domain.h"
#include "ara/core/error_code.h"
#include "ara/core/exception.h"

namespace ara
{
    namespace core
    {

        class ErrorCode;

        enum class optional_errc : ErrorDomain::CodeType
        {
            bad_access = 1
        };

        /**
         * \brief Optional类引发的异常类型
         */
        class OptionalException : public Exception
        {
        public:
            /**
             * \brief 构造一个包含Error对象的新的可选异常对象。
             */
            explicit OptionalException(ErrorCode &&err) noexcept : Exception(std::move(err)) {}
        };

        class OptionalErrorDomain : public ErrorDomain
        {
            constexpr static ErrorDomain::IdType kId = 0x6F7074696F6E616C;

        public:
            using Errc = optional_errc;
            using Exception = OptionalException;

            constexpr OptionalErrorDomain() noexcept : ErrorDomain(kId) {}

            char const *Name() const noexcept override { return "Optional"; }

            char const *Message(ErrorDomain::CodeType errorCode) const noexcept override
            {
                optional_errc const code = static_cast<optional_errc>(errorCode);
                switch (code)
                {
                case optional_errc::bad_access:
                    return "Accessing an optional object that does not contain a value";
                default:
                    return "Unknown error";
                }
            }

            void ThrowAsException(ErrorCode const &errorCode) const noexcept(false) override
            {
                ThrowOrTerminate<Exception>(errorCode);
            }
        };

        namespace internal
        {
            constexpr OptionalErrorDomain g_optionalErrorDomain;
        } // namespace internal

        inline constexpr ErrorDomain const &GetOptionalDomain() { return internal::g_optionalErrorDomain; }

        inline constexpr ErrorCode MakeErrorCode(optional_errc code, ErrorDomain::SupportDataType data, char const * = "")
        {
            return ErrorCode(static_cast<ErrorDomain::CodeType>(code), GetOptionalDomain(), data);
        }

    } // namespace core
} // namespace ara

#endif // _ARA_CORE_OPTIONAL_ERROR_H_
