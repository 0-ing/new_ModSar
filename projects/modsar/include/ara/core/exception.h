/**
 * \copyright BCSC all rights resvered
 * \author JJL
 * \date 2023/7/22
 */

#ifndef _ARA_CORE_EXCEPTION_H_
#define _ARA_CORE_EXCEPTION_H_

#include <exception>
#include <system_error>
#include <utility>
#include <ostream>

#include "ara/core/error_code.h"
#include "ara/core/string.h"

namespace ara {
    namespace core {

    /**
     * \defgroup ARA_CORE_EXCEPTION Exception data type
     * \brief Base type for all AUTOSAR exception types
     *
     * @traceid{SWS_CORE_00601}
     */
    class Exception : public std::exception {
        ErrorCode error_code_;

    public:
        /**
         * \brief 根据指定的ErrorCode构造一个新的异常实例
         * \param err  错误码
         *
         * @traceid{SWS_CORE_00611}
         */
        explicit Exception(ErrorCode err) noexcept : error_code_(std::move(err)) {}

        /**
         * \brief 获取异常对应ErrorCode实例
         * \returns 异常对应ErrorCode
         *
         * @traceid{SWS_CORE_00613}
         */
        ErrorCode const& Error() const noexcept { return error_code_; }

        /**
         * \brief 根据实例化时输入的ErrorCode实例，以字符串的形式返回ErrorCode所包含的错误码的含义。
         *
         * 该函数重写std::exception的what函数，详细用法参考std::exception::what
         *
         * \returns 异常文字说明
         *
         * @traceid{SWS_CORE_00612}
         */
        char const* what() const noexcept override {
            return error_code_.Domain().Message(error_code_.Value());
        }

    protected:
        /**
         * \brief 拷贝赋值
         * \param other  其他实例
         * \returns 返回自身
         *
         * @traceid{SWS_CORE_00614}
         */
        Exception& operator=(Exception const& other) = default;
    };

    }  // namespace core
}  // namespace ara

#endif  // _ARA_CORE_EXCEPTION_H_
