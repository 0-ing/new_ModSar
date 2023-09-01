/**
 * \copyright BCSC all rights resvered
 * \author JJL
 * \date 2023/7/22
 */
#ifndef ARA_CORE_ERROR_CODE_H_
#define ARA_CORE_ERROR_CODE_H_

#include "ara/core/error_domain.h"
#include "ara/core/string_view.h"

namespace ara {
    namespace core {

    /**
     * \brief ErrorCode包含原始错误码值和错误域。原始错误码值为特定于此错误域。
     *
     * @traceid{SWS_CORE_00501}
     * @traceid{SWS_CORE_10300}
     */
    class ErrorCode final {
        friend std::ostream& operator<<(std::ostream& out, ErrorCode const& e) {
            return (out << e.mDomain->Name() << ":" << e.mValue << ":" << e.mSupportData);
        }

        using CodeType = ErrorDomain::CodeType;
        using SupportDataType = ErrorDomain::SupportDataType;

    public:
        /**
         * \brief ErrorCode有参构造函数
         * 除非EnumT是枚举类型，否则此构造函数不参与重载解析。
         *
         * \tparam EnumT  枚举类
         * \param e   错误码值对应枚举类型
         * \param data  补充数据
         *
         * @traceid{SWS_CORE_00512}
         */
        template <typename EnumT, typename = typename std::enable_if<std::is_enum<EnumT>::value>::type>
        constexpr ErrorCode(EnumT e, SupportDataType data = 0) noexcept
            // 调用MakeErrorCode()不合格，这样就可以通过ADL找到正确的重载。
            : ErrorCode(MakeErrorCode(e, data)) {}
        /**    
         * \brief ErrorCode有参构造函数
         *
         * \param value  域内指定错误码对应数值
         * \param domain  与value对应的的错误域              
         * \param data  补充数据
         *
         * @traceid{SWS_CORE_00513}
         */
        
        constexpr ErrorCode(CodeType value, ErrorDomain const& domain, SupportDataType data = 0) noexcept
            : mValue(value), mSupportData(data), mDomain(&domain) {}
        /**
         * \brief 返回原始错误码值
         * \returns 原始错误码值
         *
         * @traceid{SWS_CORE_00514}
         */
        constexpr CodeType Value() const noexcept { return mValue; }

        /**
         * \brief 获取该ErrorCode实例所包含的用户自定义补充数据
         *
         * 底层类型和返回值的含义由实现定义。
         *
         * \returns 用户自定义补充数据
         *
         * @traceid{SWS_CORE_00516}
         */
        constexpr SupportDataType SupportData() const noexcept { return mSupportData; }

        /**
         * \brief 获取该ErrorCode所关联的错误域
         * \returns 该ErrorCode所关联的错误域
         *
         * @traceid{SWS_CORE_00515}
         */
        constexpr ErrorDomain const& Domain() const noexcept { return *mDomain; }

        /**
         * \brief 根据ErrorCode实例所包含的错误码，返回对应的字符串文本说明
         * \returns 错误码对应文字说明
         *
         * @traceid{SWS_CORE_00518}
         */
        StringView Message() const noexcept { return Domain().Message(Value()); }

        /**
         * \brief 抛出错误码对应异常
         *
         * 此函数将决定该错误码对应的异常类型，并抛出它，抛出的异常类型包含该错误码
         *
         * @traceid{SWS_CORE_00519}
         */
        [[noreturn]] void ThrowAsException() const noexcept(false) {
            Domain().ThrowAsException(*this);

            // Never reached, but apparently needed to avoid warnings from certain compilers (such as 5.4.0).
            std::terminate();
        }

    private : 
        CodeType mValue;
        SupportDataType mSupportData;
        ErrorDomain const* mDomain;  // non-owning pointer to the associated ErrorDomain
    };

    /**
     * \brief Global operator== for ErrorCode
     * 如果两个ErrorCode的Value()及Domain()结果相等，则认为两个
     * ErrorCode相等，SupportData()结果是否相等，不予考虑。
     * 
     * \param lhs  要比较的ErrorCode的实例1
     * \param rhs  要比较的ErrorCode的实例2
     * \returns 比较的结果，如果两个实例相等，返回true；否则，返回false。

    * @traceid{SWS_CORE_00571}
    */
    constexpr bool operator==(ErrorCode const& lhs, ErrorCode const& rhs) noexcept {
        return lhs.Domain() == rhs.Domain() && lhs.Value() == rhs.Value();
    }

    /**
     * \brief Global operator!= for ErrorCode
     *
     * 如果两个ErrorCode的Value()及Domain()结果相等，则认为两个
     * ErrorCode相等，SupportData()结果是否相等，不予考虑。

    * \param lhs  要比较的ErrorCode的实例1
    * \param rhs  要比较的ErrorCode的实例2
    * \returns 比较的结果，如果两个实例不相等，返回true；否则，返回false。
    *
    * @traceid{SWS_CORE_00572}
    */
    constexpr bool operator!=(ErrorCode const& lhs, ErrorCode const& rhs) noexcept {
        return lhs.Domain() != rhs.Domain() || lhs.Value() != rhs.Value();
    }

    template <typename ExceptionType>
    [[noreturn]] void ThrowOrTerminate(ErrorCode errorCode) {
    #ifndef ARA_NO_EXCEPTIONS
        throw ExceptionType(std::move(errorCode));
    #else
        (void) errorCode;
        std::terminate();
    #endif
    }


    }  // namespace core
}  // namespace ara

#endif // ARA_CORE_ERROR_CODE_H_