#ifndef ARA_CORE_RESULT_H_
#define ARA_CORE_RESULT_H_

#include <system_error>
#include <type_traits>
#include <memory>
#include <utility>
#include <iostream>

#include "ara/core/variant.h"

#include "ara/core/error_code.h"

namespace ara {
    namespace core {
    /**
     * \brief This class is a type that contains either a value or an error.
     *        本章节描述ara::core::Result<T,E>数据类型，该数据类型要么包含一个T类型的值，要么包含一个E类型的错误。
     *
     * \tparam T  the type of value
     * \tparam E  the type of error
     *
     * traceid SWS_CORE_00701
     */
    template <typename T, typename E = ErrorCode>
    class Result final {
        Variant<T, E> mData;

    public:
    
        /**
         * \brief  Type alias for the type T of values 
         *         该Result包含的数值类型，支持 T 为void类型
         * traceid SWS_CORE_00711
         */
        using value_type = T;
        /**
         * \brief Type alias for the type E of errors
         *        该Result包含的错误类型
         * traceid SWS_CORE_00712
         */
        using error_type = E;
        
        /**
         * \brief 根据指定的值（给定为左值）生成一个新结果。
         * \param t 要放入结果中的值
         * \returns a包含值\a t的结果
         * @traceid{SWS_CORE_00731}
         */
        static Result FromValue(T const& t) { return Result(t); }

        /**
         * \brief 根据指定的值（给定为右值）生成一个新结果。
         * \param t 要放入结果中的值
         * \returns a包含值\a t的结果
         * @traceid{SWS_CORE_00732}
         */
        static Result FromValue(T&& t) { return Result(std::move(t)); }
        
        /**
         * \brief 使用T的构造函数入参构造含T类型数值的Result实例
         * This function shall not participate in overload resolution unless:
         * std::is_constructible<T, Args&&...>::value is true, and
         * - the first type of the expanded parameter pack is not T, and
         *   - the first type of the expanded parameter pack is not a specialization of Result
         * \tparam Args。。。给这个函数的参数类型
         * \param args 用于构造值的参数
         * \returns 一个包含结果的值
         * @traceid{SWS_CORE_00733}
         */
        template <typename... Args>
        static Result FromValue(Args&&... args) {
            return Result(T { std::forward<Args>(args)... });
        }
        
        /**
         * \brief 根据传入的E类型左值返回一个含有E类型实例的Result实例
         * \param e 要放入结果中的error值
         * \returns 包含值e的结果
         * @traceid{SWS_CORE_00734}
         */
        static Result FromError(E const& e) { return Result(e); }
        
        /**
         * \brief 根据传入的E类型右值返回一个含有E类型实例的Result实例
         * \param e 要放入结果中的error值
         * \returns 包含值e的结果
         * @traceid{SWS_CORE_00735}
         */
        static Result FromError(E&& e) { return Result(std::move(e)); }
        
        /**
         * \brief 根据根据给定参数就地构造的错误生成新的Result。
         * This function shall not participate in overload resolution unless:
         * std::is_constructible<E, Args&&...>::value is true, and
         * - the first type of the expanded parameter pack is not E, and
         * - the first type of the expanded parameter pack is not a specialization of Result
         * \tparam Args。。。给这个函数的参数类型
         * \param args 用于构造error值的参数
         * \returns 一个包含结果的值
         * @traceid{SWS_CORE_00736}
         */
        template <typename... Args>
        static Result FromError(Args&&... args) {
            return Result(E { std::forward<Args>(args)... });
        }

        // 默认构造函数被 delete 了
        Result() = delete;
    
        /**
         * \brief 根据指定的值（给定为左值）构造一个新的Result。
         * \param t 要放入结果中的值
         * @traceid{SWS_CORE_00721}
         */
        Result(T const& t) : mData(t) { }

        /**
         * \brief 根据指定的值（给定为右值）构造一个新的Result。
         * \param t 要放入结果中的值
         * @traceid{SWS_CORE_00722}
         */
        Result(T&& t) : mData(std::move(t)) { }

        /**
         * \brief 根据指定的错误（给定为左值）构造一个新的Result。
         * \param e 要放入结果中的错误
         * @traceid{SWS_CORE_00723}
         */
        explicit Result(E const& e) : mData(e) { }
        
        /**
         * \brief 根据指定的错误（给定为右值）构造一个新的Result。
         * \param e 要放入结果中的错误
         * @traceid{SWS_CORE_00724}
         */
        explicit Result(E&& e) : mData(std::move(e)) { }

        /**
         * \brief 拷贝构造函数
         * \param e 左值实例
         * @traceid{SWS_CORE_00725}
         */
        Result(Result const& oth   er) = default;
        
        /**
         * \brief 移动构造函数
         * \param e 右值实例
         * @traceid{SWS_CORE_00726}
         */
        Result(Result&& other) noexcept(
            std::is_nothrow_move_constructible<T>::value&& std::is_nothrow_move_constructible<E>::value)
            = default;
        
        /**
         * \brief 析构函数
         * This destructor is trivial if std::is_trivially_destructible<T>::value
         * && std::is_trivially_destructible<E>::value is true.
         * @traceid{SWS_CORE_00727}
         */
        ~Result() = default;

        /**
         * \brief 复制并将另一个结果分配给此实例。
         * \param other  另一个实例
         * \return *这个，包含\aother的内容
         */
        Result& operator=(Result const& other) = default;

        /**
         * \brief 移动并将另一个结果分配给此实例。
         * \param other  另一个实例
         * \return *这个，包含\aother的内容
         * @traceid{SWS_CORE_00742}
         */
        Result& operator=(Result&& other) noexcept(
            std::is_nothrow_move_constructible<T>::value&& std::is_nothrow_move_assignable<T>::value&&
                std::is_nothrow_move_constructible<E>::value&& std::is_nothrow_move_assignable<E>::value)
            = default;
        
        /**
         * \brief 在这个实例中放入一个新值，该值是根据给定的参数就地构造的。
         * \tparam Args...  给这个函数的参数类型
         * \param args...  用于构造值的参数=
         * @traceid{SWS_CORE_00743}
         */
        template <typename... Args>
        void EmplaceValue(Args&&... args) {
            mData.template emplace<T>(std::forward<Args>(args)...);
        }
        
        /**
         * \brief 在这个实例中放入一个新的错误，这个错误是根据给定的参数就地构造的。
         * \tparam Args...  给这个函数的参数类型
         * \param args...  用于构造error值的参数
         * @traceid{SWS_CORE_00744}
         */
        template <typename... Args>
        void EmplaceError(Args&&... args) {
            mData.template emplace<E>(std::forward<Args>(args)...);
        }
        /**
         * \brief 将此实例的内容与其他实例的内容交换。
         * \param [in,out] other 其他实例
         * @traceid{SWS_CORE_00745}
         */
        void Swap(Result& other) noexcept(
            std::is_nothrow_move_constructible<T>::value&& std::is_nothrow_move_assignable<T>::value&&
                std::is_nothrow_move_constructible<E>::value&& std::is_nothrow_move_assignable<E>::value) {
            using std::swap;
            swap(mData, other.mData);
        }
        
        /**
         * \brief 检查*this是否包含值。
         * \return 如果*this包含值，则为true，否则为false
         * @traceid{SWS_CORE_00752}
         */
        explicit operator bool() const noexcept { return HasValue(); }
    
        /**
         * \brief 判断Result实例是否含有值类型。如果包含则返回true，否则返回false。
         * \return 如果*this包含值，则为true，否则为false
         * @traceid{SWS_CORE_00751}
         */
        bool HasValue() const noexcept { return mData.index() == 0; }

        /**
         * \brief 访问包含的值。
         *        如果*This不包含值，则该函数的行为是未定义的。
         * \return 对包含值的const_reference 
         * @traceid{SWS_CORE_00753}
         */
        T const& operator*() const& { return Value(); }
        
        /**
         * \brief 访问包含的值。
         *        如果*This不包含值，则该函数的行为是未定义的。
         * \return 对包含值的rreference 
         * @traceid{SWS_CORE_00759}
         */
        T&& operator*() && { return std::move(*this).Value(); }

        /**
         * \brief 重载，返回指向该Result内T值的指针
         *        如果*This不包含值，则该函数的行为是未定义的。
         * \return 指向包含值的指针
         * @traceid{SWS_CORE_00754}
         */
        T const* operator->() const { return std::addressof(Value()); }
        
        /**
         * \brief 获取该Result实例所包含的T类型的值
         *        如果*this不包含值，则此函数的行为是未定义的。
         * \return 对包含值的常量引用
         * @traceid{SWS_CORE_00755}
         */
        T const& Value() const& {
            const T * ptr = get_if<T>(&mData);
            if (ptr)
                return *ptr;
            std::cout << "__ value() called but NOT a value!\n";
            std::terminate();
        }
    
        /**
         * \brief 访问包含的值。
         *        如果*this不包含值，则此函数的行为是未定义的。
         * \return 对包含值的right引用
         * @traceid{SWS_CORE_00755}
         */
        T&& Value() && {
            T* ptr = get_if<T>(&mData);
            if (ptr)
                return std::move(*ptr);
            std::cout << "__ value() called but NOT a value!\n";
            std::terminate();
        }

        /**
         * \brief 获取该Result实例所包含的T类型的值
         *        如果*this不包含值，则此函数的行为是未定义的。
         * \return 对包含值的const引用
         * @traceid{SWS_CORE_007557}
         */
        E const& Error() const& {
            E const* ptr = get_if<E>(&mData);
            if (ptr)
                return *ptr;
            std::cout << "__ error() called but NOT an error!\n";
            std::terminate();
        }

        /**
         * \brief 访问包含的错误。
         *        如果*this不包含值，则此函数的行为是未定义的。
         * \return 对包含值的r引用
         * @traceid{SWS_CORE_007558}
         */
        E&& Error() && {
            E* ptr = get_if<E>(&mData);
            if (ptr)
                return std::move(*ptr);
            std::cout << "__ error() called but NOT an error!\n";
            std::terminate();
        }
        
        /**
         * \brief 获取该Result所含T值
         *
         * 如果该Result实例存在值，则返回该Result实例所包含的值，否则返回传入的U类型的值（被强制转换为T类型返回）。
         *
         * \tparam U  the type of \a defaultValue
         * \param defaultValue  如果*this不包含value，则使用这个value
         * \returns the value
         *
         * @traceid{SWS_CORE_00761}
         */
        template <typename U>
        T ValueOr(U&& defaultValue) const& {
            return HasValue() ? Value() : static_cast<T>(std::forward<U>(defaultValue));
        }

        /**
         * \brief 获取该Result所含T值
         *
         * 如果该Result实例存在值，则返回该Result实例所包含的值，否则返回传入的U类型的值（被强制转换为T类型返回）。
         *
         * \tparam U  the type of \a defaultValue
         * \param defaultValue  如果*this不包含value，则使用这个value
         * \returns the value
         *
         * @traceid{SWS_CORE_00762}
         */
        template <typename U>
        T ValueOr(U&& defaultValue) && {
            return HasValue() ? std::move(Value()) : static_cast<T>(std::forward<U>(defaultValue));
        }

        /**
         * \brief 获取该Result所含E值，如果该Result实例存在E值，则返回该
         * Result实例所包含的E值，否则返回传入的G类型的值（被强制转换为E类型返回）
         *
         * \tparam G  the type of \a defaultError
         * \param defaultError  the error to use if *this does not contain an error
         * \returns the error
         * 
         * @traceid{SWS_CORE_00763}
         */
        template <typename G>
        E ErrorOr(G&& defaultError) const {
            return HasValue() ? static_cast<E>(std::forward<G>(defaultError)) : Error();
        }

        /**
         * \brief 检查Result所含错误类型
         * 
         * 如果该Result实例包含值则返回false；
         * 如果该Result实例所包含的错误实例和传入的G类型的错误实例一致，则返回true，否则返回false。
         * 
         * \tparam G  the type of the error argument \a error
         * \param error  the error to check
         * \returns true if *this contains an error that is equivalent to the given error, false otherwise
         * 
         * @traceid{SWS_CORE_00765}
         */
        template <typename G>
        bool CheckError(G&& e) const {
            return HasValue() ? false : (Error() == static_cast<E>(std::forward<G>(e)));
        }

    #ifndef ARA_NO_EXCEPTIONS

        /**
         * \brief 返回该Result的T值，若Result不含T，则根据E类型的错误抛出
         * ara::core::Exception类型的异常
         * 
         * 当编译器工具链不支持c++异常时，此函数不参与重载解析。
         * 
         * \returns a const reference to the contained value
         * \throws <TYPE>  the exception type associated with the contained error
         * 
         * @traceid{SWS_CORE_00766}
         */
        T const& ValueOrThrow() const& noexcept(false) {
            T const* ptr = get_if<T>(&mData);
            if (ptr)
                return *ptr;

            // TODO: check if mData is uninitialized (-> monostate)
            E const* e = get_if<E>(&mData);

            e->ThrowAsException();
        }

        /**
         *  \brief 返回该Result的T值，若Result不含T，则根据E类型的错误抛出ara::core::Exception类型的异常
         * 
         *  当编译器工具链不支持c++异常时，此函数不参与重载解析。
         * 
         *  \returns an rvalue reference to the contained value
         *  \throws <TYPE>  the exception type associated with the contained error
         * 
         *  @traceid{SWS_CORE_00769}
         */
        T&& ValueOrThrow() && noexcept(false) {
            T* ptr = get_if<T>(&mData);
            if (ptr)
                return std::move(*ptr);

            E* e = get_if<E>(&mData);
            e->ThrowAsException();
        }

    #endif

        /**
         * \brief 如果该Result实例包含值，则返回该值，否则返回传入函数的返回值。
         * 传入的函数的形参为E类型的错误。
         * 
         * Callable应该与这个接口兼容:
         * <code>
         * T f(const E&);
         * </code>
         * 
         * \tparam F  the type of the Callable \a f
         * \param f  the Callable
         * \returns the value
         * 
         * @traceid{SWS_CORE_00767}
         */ 
        template <typename F>
        T Resolve(F&& f) const {
            return HasValue() ? Value() : std::forward<F>(f)(Error());
        }

    private:
        // Re-implementation of C++14's std::enable_if_t
        template <bool Condition, typename U = void>
        using enable_if_t = typename std::enable_if<Condition, U>::type;

        // Re-implementation of C++14's std::result_of_t
        template <typename U>
        using result_of_t = typename std::result_of<U>::type;

        *  \brief Trait that detects whether a type is a Result<...>
        template <typename U>
        struct is_result : std::false_type { };

        template <typename U, typename G>
        struct is_result<Result<U, G>> : std::true_type { };

        template <typename F>
        using CallableReturnsResult = enable_if_t<is_result<result_of_t<F(T const&)>>::value>;

        template <typename F>
        using CallableReturnsNoResult = enable_if_t<!is_result<result_of_t<F(T const&)>>::value>;

    public:
        /**
         *  \brief 对Result内T值，执行f操作，并使用f的返回值构造新的Result返回
         * 
         *  Callable应该兼容以下两个接口之一:
         *  - <code>Result<XXX, E> f(const T&);</code>
         *  - <code>XXX f(const T&);</code>
         *  这意味着Callable要么直接返回Result<XXX>，要么直接返回XXX，其中XXX可以是适合于Result类使用的任何类型。
         * 
         *  对于返回Result类型的模板实参f，该函数的返回类型为<code>decltype(f(Value()))</code>;
         *  对于不返回Result类型的模板实参f，该函数的返回类型为<code>Result<decltype(f(Value()))， E></code>。
         * 
         *  如果该实例不包含值，则仍然会创建并返回一个新的Result<XXX, E>，并将该实例的原始错误内容复制到新实例中。
         * 
         *  \tparam F  the type of the Callable \a f
         *  \param f  可调用的F类型的函数
         *  \returns 可被转换的数据类型的新的Result实例
         * 
         *  @traceid{SWS_CORE_00768}
         */ 
        template <typename F, typename = CallableReturnsResult<F>>
        auto Bind(F&& f) const -> decltype(f(Value())) {
            using U = decltype(f(Value()));
            return HasValue() ? std::forward<F>(f)(Value()) : U(Error());
        }

        // @traceid{SWS_CORE_00768}
        template <typename F, typename = CallableReturnsNoResult<F>>
        auto Bind(F&& f) const -> Result<decltype(f(Value())), E> {
            using U = decltype(f(Value()));
            using R = Result<U, E>;
            return HasValue() ? std::forward<F>(f)(Value()) : R(Error());
        }

        template <typename, typename>
        friend class Result;
    };

    /**
     *  \brief 交换两个给定参数的内容。
     *  \param lhs  一个实例
     *  \param rhs  另一个实例
     * 
     *  @traceid{SWS_CORE_00796}
     */ 
    template <typename T, typename E>
    void swap(Result<T, E>& lhs, Result<T, E>& rhs) noexcept(noexcept(lhs.Swap(rhs))) {
        lhs.Swap(rhs);
    }

    /**
     *  \brief 比较两个Result实例是否相等。
     * 
     *  比较两个Result实例是否相等。如果一个Result包含了Value，它不
     *  等于任何包含Error的Result。当且仅当两个Result的实例都包含同
     *  样的数据类型，比如都是ErrorCode或者都是Value，并且包含的该
     *  类型的值相等，这两个Result才相等。
     * 
     *  \param lhs  用于比较的Result实例1
     *  \param rhs  用于比较的Result实例2
     *  \returns true if the two instances compare equal, false otherwise
     * 
     *  @traceid{SWS_CORE_00780}
     */ 
    template <typename T, typename E>
    bool operator==(Result<T, E> const& lhs, Result<T, E> const& rhs) {
        if (static_cast<bool>(lhs) != static_cast<bool>(rhs))
            return false;
        if (lhs)
            return lhs.Value() == rhs.Value();
        return lhs.Error() == rhs.Error();
    }

    /**
     *  \brief 比较两个Result实例是否不相等。如果一个Result包含了Value，它
     *  不等于任何包含Error的Result。当且仅当两个Result的实例都包含
     *  同样的数据类型，比如都是ErrorCode或者都是Value，并且包含的
     *  该类型的值相等，这两个Result才相等，其他场景均为不相等。
     * 
     *  \param lhs  用于比较的Result实例1
     *  \param rhs  用于比较的Result实例2
     *  \returns true if the two instances compare unequal, false otherwise
     *  \see operator==()
     * 
     *  @traceid{SWS_CORE_00781}
     */ 
    template <typename T, typename E>
    bool operator!=(Result<T, E> const& lhs, Result<T, E> const& rhs) {
        if (static_cast<bool>(lhs) != static_cast<bool>(rhs))
            return true;
        if (lhs)
            return lhs.Value() != rhs.Value();
        return lhs.Error() != rhs.Error();
    }

    /**
     *  \brief 比较Result和Value是否相等。一个不包含Value的Result不等于任
     *  何Value。当且仅当一个包含Value的Result与待比较的Value有相
     *  同的数据类型，并且Result包含的Value等于待比较的Value，它们
     *  才相等。
     * 
     *  \param lhs  用于比较的Result实例
     *  \param rhs  用于比较的Value
     *  \returns true if the Result's value compares equal to the rhs value, false otherwise
     * 
     *  @traceid{SWS_CORE_00782}
     */ 
    template <typename T, typename E>
    bool operator==(Result<T, E> const& lhs, T const& value) {
        return lhs ? *lhs == value : false;
    }

    /**
     *  \brief 比较Result和Value是否相等。一个不包含Value的Result不等于任
     *  何Value。当且仅当一个包含Value的Result与待比较的Value有相
     *  同的数据类型，并且Result包含的Value等于待比较的Value，它们
     *  才相等。
     * 
     *  \param lhs  用于比较的Value
     *  \param rhs  用于比较的Result实例
     *  \returns true if the Result's value compares equal to the lhs value, false otherwise
     * 
     *  @traceid{SWS_CORE_00783}
     */ 
    template <typename T, typename E>
    bool operator==(T const& value, Result<T, E> const& rhs) {
        return rhs ? value == *rhs : false;
    }

    /**
     *  \brief 比较Result和Value是否不相等。一个不包含Value的Result不等于
     *  任何Value。当且仅当一个包含Value的Result与待比较的Value有
     *  相同的数据类型，并且Result包含的Value等于待比较的Value，它
     *  们才相等，其他场景均为不相等。
     * 
     *  \param lhs  用于比较的Result实例
     *  \param rhs  用于比较的Value
     *  \returns true if the Result's value compares unequal to the rhs value, false otherwise
     * 
     *  @traceid{SWS_CORE_00784}
     */ 
    template <typename T, typename E>
    bool operator!=(Result<T, E> const& lhs, T const& value) {
        return lhs ? *lhs != value : true;
    }

    /**
     *  \brief 比较Result和Value是否不相等。一个不包含Value的Result不等于
     *  任何Value。当且仅当一个包含Value的Result与待比较的Value有
     *  相同的数据类型，并且Result包含的Value等于待比较的Value，它
     *  们才相等，其他场景均为不相等。
     * 
     *  \param lhs  用于比较的Value
     *  \param rhs  用于比较的Result实例
     *  \returns true if the Result's value compares unequal to the lhs value, false otherwise
     * 
     *  @traceid{SWS_CORE_00785}
     */ 
    template <typename T, typename E>
    bool operator!=(T const& value, Result<T, E> const& rhs) {
        return rhs ? value != *rhs : true;
    }

    /**
     *  \brief 比较Result和Error是否相等。一个不包含Error的Result不等于任何
     *  Error。当且仅当一个包含Error的Result与待比较的Error有相同的
     *  数据类型，并且Result包含的Error的值等于待比较的Error的值，
     *  它们才相等。
     * 
     *  \param lhs  用于比较的Result实例
     *  \param rhs  用于比较的Error
     *  \returns true if the Result's error compares equal to the rhs error, false otherwise
     * 
     *  @traceid{SWS_CORE_00786}
     */ 
    template <typename T, typename E>
    bool operator==(Result<T, E> const& lhs, E const& error) {
        return lhs ? false : lhs.Error() == error;
    }

    /**
     *  \brief 比较Result和Error是否相等。一个不包含Error的Result不等于任何
     *  Error。当且仅当一个包含Error的Result与待比较的Error有相同的
     *  数据类型，并且Result包含的Error的值等于待比较的Error的值，
     *  它们才相等。
     * 
     *  \param lhs  用于比较的Error
     *  \param rhs  用于比较的Result实例
     *  \returns true if the Result's error compares equal to the lhs error, false otherwise
     * 
     *  @traceid{SWS_CORE_00787}
     */ 
    template <typename T, typename E>
    bool operator==(E const& error, Result<T, E> const& rhs) {
        return rhs ? false : error == rhs.Error();
    }

    /**
     *  \brief 比较Result和Error是否不相等。一个不包含Error的Result不等于任
     *  何Error。当且仅当一个包含Error的Result与待比较的Error有相同
     *  的数据类型，并且Result包含的Error的值等于待比较的Error的
     *  值，它们才相等，其他场景均为不相等。
     * 
     *  \param lhs  用于比较的Result实例
     *  \param rhs  用于比较的Error
     *  \returns true if the Result's error compares unequal to the rhs error, false otherwise
     * 
     *  @traceid{SWS_CORE_00788}
     */ 
    template <typename T, typename E>
    bool operator!=(Result<T, E> const& lhs, E const& error) {
        return lhs ? true : lhs.Error() != error;
    }

    /**
     *  \brief 比较Result和Error是否不相等。一个不包含Error的Result不等于任
     *  何Error。当且仅当一个包含Error的Result与待比较的Error有相同
     *  的数据类型，并且Result包含的Error的值等于待比较的Error的
     *  值，它们才相等，其他场景均为不相等。
     * 
     *  \param lhs  用于比较的Error
     *  \param rhs  用于比较的Result实例
     *  \returns true if the Result's error compares unequal to the lhs error, false otherwise
     * 
     *  @traceid{SWS_CORE_00789}
     */ 
    template <typename T, typename E>
    bool operator!=(E const& error, Result<T, E> const& rhs) {
        return rhs ? true : error != rhs.Error();
    }

    /**
     *  \brief Result的void值的特化
     * 
     *  \tparam T  void型
     *  \tparam E  错误类型
     * 
     *  @traceid{SWS_CORE_00801}
     */ 
    template <typename E>
    class Result<void, E> final {
        // Cannot use 'void' for variant, so define our own (default-constructible) dummy type instead.
        struct dummy { };
        using T = dummy;
        Variant<T, E> mData;

    public:
        /**
         *  \brief 值类型T的类型别名，对于此专门化始终为"void"
         *  @traceid{SWS_CORE_00811}
         */ 
        using value_type = void;
        
        /**
         *  \brief 错误类型E的类型别名
         *  @traceid{SWS_CORE_00812}
         */ 
        using error_type = E;

        /**
         *  \brief 用"void"作为值构建一个新的Result
         *  \returns 包含“void”值的Result
         * 
         *  @traceid{SWS_CORE_00831}
         */ 
        static Result FromValue() { return Result(); }

        /**
         *  \copydoc Result<T,E>::FromError(const E&)
         *  @traceid{SWS_CORE_00834}
         */ 
        static Result FromError(E const& e) { return Result(e); }

        /**
         *  \copydoc Result<T,E>::FromError(E&&)
         *  @traceid{SWS_CORE_00835}
         */ 
        static Result FromError(E&& e) { return Result(std::move(e)); }

        /**
         *  \brief 根据根据给定参数就地构造的错误生成新的Result。
         * 
         *  此函数不参与重载解析，除非:
         *  std::is_constructible<E, Args&&…>::value为true，且
         *      -扩展参数包的第一个类型不是E，并且
         *      -扩展参数包的第一种类型不是Result的专门化
         * 
         *  \tparam Args... 给这个函数的参数类型
         *  \param args     用于构造错误的参数包
         *  \returns        包含错误的Result
         * 
         *  @traceid{SWS_CORE_00836}
         */ 
        template <typename... Args>
        static Result FromError(Args&&... args) {
            return Result(E { std::forward<Args>(args)... });
        }
        
        /**
         *  \brief 用“void”值构造一个新的Result
         * 
         *  @traceid{SWS_CORE_00821}
         */ 
        Result() noexcept : mData(T {}) { }
        
        /**
         *  \copydoc Result<T,E>::Result(const E&)
         *  @traceid{SWS_CORE_00823}
         */ 
        explicit Result(E const& e) : mData(e) { }

        /**
         *  \copydoc Result<T,E>::Result(E&&)
         *  @traceid{SWS_CORE_00824}
         */ 
        explicit Result(E&& e) : mData(std::move(e)) { }

        /**
         *  \copydoc Result<T,E>::Result(const Result&)
         *  @traceid{SWS_CORE_00825}
         */ 
        Result(Result const& other) = default;

        /**
         *  \copydoc Result<T,E>::Result(Result&&)
         *  @traceid{SWS_CORE_00826}
         */ 
        Result(Result&& other) noexcept(std::is_nothrow_move_constructible<E>::value) = default;

        /**
         *  \brief Destructor
         * 
         *  This destructor is trivial if std::is_trivially_destructible<E>::value is true.
         * 
         *  @traceid{SWS_CORE_00827}
         */
        ~Result() { }

        /**
         *  \copydoc Result<T,E>::operator=(const Result&)
         *  @traceid{SWS_CORE_00841}
         */ 
        Result& operator=(Result const& other) = default;

        /**
         *  \copydoc Result<T,E>::operator=(Result&&)
         *  @traceid{SWS_CORE_00842}
         */ 
        Result& operator=(Result&& other) noexcept(
            std::is_nothrow_move_constructible<E>::value&& std::is_nothrow_move_assignable<E>::value)
            = default;

        /**
         *  \copydoc Result<T,E>::EmplaceValue()
         *  @traceid{SWS_CORE_00843}
         */ 
        template <typename... Args>
        void EmplaceValue(Args&&... args) noexcept {
            mData.template emplace<T>(std::forward<Args>(args)...);
        }

        /**
         *  \copydoc Result<T,E>::EmplaceError()
         *  @traceid{SWS_CORE_00844}
         */ 
        template <typename... Args>
        void EmplaceError(Args&&... args) {
            mData.template emplace<E>(std::forward<Args>(args)...);
        }

        /**
         *  \copydoc Result<T,E>::Swap()
         *  @traceid{SWS_CORE_00845}
         */ 
        void Swap(Result& other) noexcept(
            std::is_nothrow_move_constructible<E>::value&& std::is_nothrow_move_assignable<E>::value) {
            using std::swap;
            swap(mData, other.mData);
        }
        /**
         *  \copydoc Result<T,E>::bool()
         *  @traceid{SWS_CORE_00852}
         */ 
        explicit operator bool() const noexcept { return HasValue(); }

        /**
         *  \copydoc Result<T,E>::HasValue()
         *  @traceid{SWS_CORE_00851}
         */ 
        bool HasValue() const noexcept { return mData.index() == 0; }

        /**
         *  \copydoc Result<void,E>::Value()
         *  @traceid{SWS_CORE_00853}
         */ 
        void operator*() const { return Value(); }

        /**
         *  \brief Do nothing
         * 
         *  此函数仅用于帮助进行泛型编程
         * 
         *  如果*this不包含值，此函数的行为是未定义的。
         * 
         *  @traceid{SWS_CORE_00855}
         */ 
        void Value() const {
            T const* ptr = get_if<T>(&mData);
            if (ptr)
                return;
            std::cout << "__ value() called but NOT a (void) value!\n";
            std::terminate();
        }

        /**
         *  \copydoc Result<T,E>::Error()const&
         *  @traceid{SWS_CORE_00857}
         */ 
        E const& Error() const& {
            E const* ptr = get_if<E>(&mData);
            if (ptr)
                return *ptr;
            std::cout << "__ error() called but NOT an error!\n";
            std::terminate();
        }

        /**
         *  \copydoc Result<T,E>::Error()&&
         *  @traceid{SWS_CORE_00858}
         */ 
        E&& Error() && {
            E* ptr = get_if<E>(&mData);
            if (ptr)
                return std::move(*ptr);
            std::cout << "__ error() called but NOT an error!\n";
            std::terminate();
        }
        /**
         *  \brief Do nothing.
         * 
         *  \note 此函数仅用于帮助进行泛型编程。
         * 
         *  \tparam U  defaultValue的类型
         *  \param defaultValue  如果*this不包含值，则使用的值
         * 
         *  @traceid{SWS_CORE_00861}
         */ 
        template <typename U>
        void ValueOr(U&& defaultValue) const {
            return HasValue() ? Value() : static_cast<void>(std::forward<U>(defaultValue));
        }

        /**
         *  \copydoc Result<T,E>::ErrorOr(G&&)const&
         *  @traceid{SWS_CORE_00863}
         */ 
        template <typename G>
        E ErrorOr(G&& defaultError) const {
            return HasValue() ? static_cast<E>(std::forward<G>(defaultError)) : Error();
        }
        /**
         *  \copydoc Result<T,E>::CheckError()
         *  @traceid{SWS_CORE_00865}
         */ 
        template <typename G>
        bool CheckError(G&& e) const {
            return HasValue() ? false : (Error() == static_cast<E>(std::forward<G>(e)));
        }

    #ifndef ARA_NO_EXCEPTIONS
        /**
         *  \copydoc Result<T,E>::ValueOrThrow()
         *  @traceid{SWS_CORE_00866}
         */ 
        void ValueOrThrow() const noexcept(false) {
            T const* ptr = get_if<T>(&mData);
            if (ptr)
                return;

            E const* e = get_if<E>(&mData);
            e->ThrowAsException();
        }
    #endif
        /**
         *  \brief 什么都不做 或者 调用函数。
         *  如果*this包含一个值，这个函数什么也不做。否则，将调用指定的可调用对象
         * 
         *  Callable应该与这个接口兼容:
         *  <code>
         *  void f(const E&);
         *  </code>
         * 
         *  此函数仅用于帮助进行泛型编程。
         * 
         *  \tparam F  the type of the Callable \a f
         *  \param f  the Callable
         * 
         *  @traceid{SWS_CORE_00867}
         */ 
        template <typename F>
        void Resolve(F&& f) const {
            return HasValue() ? Value() : std::forward<F>(f)(Error());
        }

    private:
        // Re-implementation of C++14's std::enable_if_t
        template <bool Condition, typename U = void>
        using enable_if_t = typename std::enable_if<Condition, U>::type;

        // Re-implementation of C++14's std::result_of_t
        template <typename U>
        using result_of_t = typename std::result_of<U>::type;

        /// \brief Trait that detects whether a type is a Result<...>
        template <typename U>
        struct is_result : std::false_type { };

        template <typename U, typename G>
        struct is_result<Result<U, G>> : std::true_type { };

        template <typename F>
        using CallableReturnsResult = enable_if_t<is_result<result_of_t<F()>>::value>;

        template <typename F>
        using CallableReturnsNoResult = enable_if_t<!is_result<result_of_t<F()>>::value>;

    public:
        /**
         *  \brief 将给定的Callable应用于此实例的值，并返回一个包含调用结果的新Result。
         * 
         *  Callable应该兼容以下两个接口之一:
         *  - <code>Result<XXX, E> f();</code>
         *  - <code>XXX f();</code>
         *  这意味着Callable要么直接返回Result<XXX>，要么直接返回XXX，其中XXX可以是适合于Result类使用的任何类型。
         * 
         *  对于返回Result类型的模板实参f，该函数的返回类型是<code>decltype(f())</code>;
         *  对于不返回Result类型的模板实参f，该函数的返回类型是<code>Result<decltype(f())， E></code>。
         * 
         *  如果该实例不包含值，则仍然会创建并返回一个新的Result<XXX, E>，并将该实例的原始错误内容复制到新实例中。
         * 
         *  \tparam F  函数f的类型
         *  \param f   传入的函数指针
         *  \returns   可能已转换类型的新Result实例
         * 
         *  @traceid{SWS_CORE_00870}
         */ 
        template <typename F, typename = CallableReturnsResult<F>>
        auto Bind(F&& f) const -> decltype(f()) {
            using U = decltype(f());
            return HasValue() ? std::forward<F>(f)() : U(Error());
        }

        /// @traceid{SWS_CORE_00870}
        template <typename F, typename = CallableReturnsNoResult<F>>
        auto Bind(F&& f) const -> Result<decltype(f()), E> {
            using U = decltype(f());
            using R = Result<U, E>;
            return HasValue() ? std::forward<F>(f)() : R(Error());
        }

        template <typename, typename>
        friend class Result;
    };

    } // namespace core
} // namespace ara

#endif // ARA_CORE_RESULT_H_
