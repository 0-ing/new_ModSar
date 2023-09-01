/**
 * \copyright BCSC all rights resvered
 * \author JJL
 * \date 2023/7/22
 */
#ifndef ARA_CORE_ERROR_DOMAIN_H_
#define ARA_CORE_ERROR_DOMAIN_H_
#include <cstdint>

namespace ara {
    namespace core {

    // forward declaration
    class ErrorCode;

    /**
     * \brief Encapsulation of an error domain

     * 封装了一个错误域（ErrorDomain）。
     * 错误域是错误码值的控制实体，它定义了错误码值到文本表示的映射。
     * 具体的错误域类应从此虚基类继承实现。
     *
     * @traceid{SWS_CORE_00110}
     * @traceid{SWS_CORE_10400}
     */
    class ErrorDomain {
    public:
        /**
         * \brief ErrorDomain标识符类型别名，不同ErrorDomain具有唯一标识符
         * 
         * @traceid{SWS_CORE_00121}
         */
        using IdType = std::uint64_t;

        /** 
         * \brief ErrorDomain内错误码数据类型别名
         *
         * @traceid{SWS_CORE_00122}
        */
        using CodeType = std::int32_t;

        /**
         * \brief 补充数据类型别名
         * 
         * @traceid{SWS_CORE_00123}
        */
        using SupportDataType = std::int32_t;

        /**
         * \brief Copy construction is disabled.
         * rrorDomain禁用拷贝构造函数。
         * 
         * @traceid{SWS_CORE_00131}
         */
        ErrorDomain(ErrorDomain const&) = delete;

        /**
         * \brief Move construction is disabled.
         * ErrorDomain禁用移动构造函数。
         * 
         *  @traceid{SWS_CORE_00132}
         */
        ErrorDomain(ErrorDomain&&) = delete;

        /**
         * \brief Copy assignment is disabled.
         * ErrorDomain禁用拷贝赋值。
         * 
         * @traceid{SWS_CORE_00133}
         */
        ErrorDomain& operator=(ErrorDomain const&) = delete;

        /**
         * \brief Move assignment is disabled.
         * ErrorDomain禁用移动赋值。
         * 
         * @traceid{SWS_CORE_00134}
         */
        ErrorDomain& operator=(ErrorDomain&&) = delete;

        /**
         * \brief Returns the name of this error domain.
         *
         * The returned pointer remains owned by class ErrorDomain and
         * shall not be freed by clients.
         * 返回错误域的名称，返回值不会是nullptr。
         * 
         * \returns the name as a null-terminated string, never nullptr
         * @traceid{SWS_CORE_00152}
         */
        virtual char const* Name() const noexcept = 0;

        /**
         * \brief Returns a textual representation of the given error code.
         * It is a Violation if the errorCode did not originate from this error domain,
         * and thus be subject to SWS_CORE_00003.
         * 返回给定错误码对应的文本描述
         * 
         * The returned pointer remains owned by the ErrorDomain subclass and
         * shall not be freed by clients.
         * 
         * \param errorCode  the domain-specific error code
         * \returns the text as a null-terminated string, never nullptr
         *
         * @traceid{SWS_CORE_00153}
         */
        virtual char const* Message(CodeType errorCode) const noexcept = 0;

        /**
         * \brief Throws the given errorCode as Exception
         * 根据指定的ErrorCode抛出异常
         * \param errorCode error code to be thrown
         * \remark if ARA_NO_EXCEPTIONS is defined, this function call will terminate.
         * 
         * @traceid{SWS_CORE_00154}
         */
        [[noreturn]] virtual void ThrowAsException(ErrorCode const& errorCode) const noexcept(false) = 0;

        /**
         * \brief Returns the unique domain identifier.
         * 
         * 返回唯一的ErrorDomain识别码。
         * 
         * \returns the identifier
         * 
         * @traceid{SWS_CORE_00151}
         * @traceid{SWS_CORE_10952}
         */
        constexpr IdType Id() const noexcept { return mId; }

        /** 
         * \brief Compares for equality with another ErrorDomain instance.
         * 操作符重载，同另一个ErrorDomain实例进行比较。如果相同返回true，否则返回false。
         *
         * Two ErrorDomain instances compare equal when their identifiers
         * (returned by Id()) are equal.
         *
         * \param other  the other instance
         * \returns true if @a other is equal to *this, false otherwise
         *
         * @traceid{SWS_CORE_00137}
         */
        constexpr bool operator==(ErrorDomain const& other) const noexcept { return mId == other.mId; }

        /**
         * \brief Compares for non-equality with another ErrorDomain instance.
         * 操作符重载，同另一个ErrorDomain实例进行比较。如果不相同返回true，否则返回false。
         * 
         * \param other  the other instance
         * \returns true if @a other is not equal to *this, false otherwise
         *
         * @traceid{SWS_CORE_00138}
         */
        constexpr bool operator!=(ErrorDomain const& other) const noexcept { return mId != other.mId; }

    protected:
        /** \brief Constructs a new instance with the given identifier.
         * 使用给定的标识符构造一个ErrorDomain实例。
         *
         * Identifiers are expected to be system-wide unique.
         *
         * \param id  the unique identifier
         *
         * @traceid{SWS_CORE_00135}
         */
        constexpr explicit ErrorDomain(IdType id) noexcept : mId(id) {}

        /**
         * \brief Destructor
         * 析构函数
        
         * This dtor is non-virtual (and trivial) so that this class is a literal type.
         * While we do have virtual functions, we never need polymorphic destruction.
         *
         * @traceid{SWS_CORE_00136}
         */
        ~ErrorDomain() = default;

    private:
        IdType const mId;
    };


    }  // namespace core
}  // namespace ara

#endif // ARA_CORE_ERROR_DOMAIN_H_
