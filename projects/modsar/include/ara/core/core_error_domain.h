/**
 * \copyright BCSC all rights resvered
 * \author JJL
 * \date 2023/7/22
 */

#ifndef ARA_CORE_CORE_ERROR_DOMAIN_H_
#define ARA_CORE_CORE_ERROR_DOMAIN_H_

#include <cerrno>

#include "ara/core/error_code.h"
#include "ara/core/error_domain.h"
#include "ara/core/exception.h"

namespace ara {
    namespace core {

    /**
     * \brief CoreError错误码枚举定义
     
    * @traceid{SWS_CORE_05200}
    * @traceid{SWS_CORE_10900}
    * @traceid{SWS_CORE_10901}
    * @traceid{SWS_CORE_10902}
    * @traceid{SWS_CORE_10903}
    * @traceid{SWS_CORE_10999}
    */
    enum class CoreErrc : ErrorDomain::CodeType {
        kInvalidArgument = 22,            ///< 函数收到无效入参
        kInvalidMetaModelShortname = 137, ///< string为无效模型元素的shortname
        kInvalidMetaModelPath = 138,      ///< 错误模型元素路径
    };

    /**
     * \brief Exception type thrown for CORE errors.
     *
     * @traceid{SWS_CORE_05211}
     * @traceid{SWS_CORE_10910}
     * @traceid{SWS_CORE_10911}
     * @traceid{SWS_CORE_10999}
     */
    class CoreException : public Exception {
    public:
        /**
         * \brief Construct a new CoreException from an ErrorCode.
         * \param err  the ErrorCode
         *
         * @traceid{SWS_CORE_05212}
         */
        explicit CoreException(ErrorCode err) noexcept : Exception(err) {}
    };

    /**
     * \brief An error domain for ara::core errors.
     * \domainid{0x8000'0000'0000'0014}
     
    * @traceid{SWS_CORE_00011}
    * @traceid{SWS_CORE_05221}
    * @traceid{SWS_CORE_10400}
    * @traceid{SWS_CORE_10930}
    * @traceid{SWS_CORE_10931}
    * @traceid{SWS_CORE_10932}
    * @traceid{SWS_CORE_10950}
    * @traceid{SWS_CORE_10999}
    */
    class CoreErrorDomain final : public ErrorDomain {
        /// @traceid{SWS_CORE_00010}
        constexpr static ErrorDomain::IdType kId = 0x8000000000000014;

    public:
        /**
         * \brief 错误码的枚举值
         *
         * @traceid{SWS_CORE_05231}
         * @traceid{SWS_CORE_10933}
         */
        using Errc = CoreErrc;

        /**
         * \brief 异常的类型
         *
         * @traceid{SWS_CORE_05232}
         * @traceid{SWS_CORE_10934}
         */
        using Exception = CoreException;

        /**
         * \brief 默认构造函数
         *
         * @traceid{SWS_CORE_05241}
         * @traceid{SWS_CORE_00014}
         */
        constexpr CoreErrorDomain() noexcept : ErrorDomain(kId) {}

        /**
         * \brief 返回CoreErrorDomain文字表示
         * \returns "Core"
         *
         * @traceid{SWS_CORE_05242}
         * @traceid{SWS_CORE_00014}
         * @traceid{SWS_CORE_10951}
         */
        char const* Name() const noexcept override { return "Core"; }

        /**
         * \brief 返回错误码对应文字信息
         * \param errorCode  Core类型错误码
         * \returns 错误码对应错误类型
         *
         * @traceid{SWS_CORE_05243}
         */
        char const* Message(ErrorDomain::CodeType errorCode) const noexcept override {
            Errc const code = static_cast<Errc>(errorCode);
            switch (code) {
                case Errc::kInvalidArgument:
                    return "Invalid argument";
                case Errc::kInvalidMetaModelShortname:
                    return "Invalid meta model shortname";
                case Errc::kInvalidMetaModelPath:
                    return "Invalid meta model path";
                default:
                    return "Unknown error";
            }
        }
        /**
         * \brief 抛出错误码对应CoreException
         * \param errorCode  Core错误码实例
         *
         * @traceid{SWS_CORE_05244}
         */
        void ThrowAsException(ErrorCode const& errorCode) const noexcept(false) override {
            /// @traceid{SWS_CORE_10953}
            ThrowOrTerminate<Exception>(errorCode);
        }
    };

    namespace internal {
    constexpr CoreErrorDomain g_coreErrorDomain;
    }  // namespace internal

    /**
     * \brief 返回全局CoreErrorDomain的引用
     * \returns 返回全局CoreErrorDomain的引用
     *
     * @traceid{SWS_CORE_05280}
     * @traceid{SWS_CORE_10980}
     * @traceid{SWS_CORE_10981}
     * @traceid{SWS_CORE_10982}
     * @traceid{SWS_CORE_10999}
     */
    constexpr ErrorDomain const& GetCoreErrorDomain() noexcept { return internal::g_coreErrorDomain; }

    /**
     * \brief 创建CoreErrorDomain内的ErrorCode
     *
     * 该函数通常为ErrorCode构造函数内部使用，用户不直接使用。
     *
     * \param code  CoreErrc错误码
     * \param data  补充数据类型
     * \returns 一个新的ErrorCode的实例
     *
     * @traceid{SWS_CORE_05290}
     * @traceid{SWS_CORE_10990}
     * @traceid{SWS_CORE_10991}
     * @traceid{SWS_CORE_10999}
     */
    constexpr ErrorCode MakeErrorCode(CoreErrc code, ErrorDomain::SupportDataType data) noexcept {
        return ErrorCode(static_cast<ErrorDomain::CodeType>(code), GetCoreErrorDomain(), data);
    }

    }  // namespace core
}  // namespace ara

#endif  // ARA_CORE_CORE_ERROR_DOMAIN_H_
