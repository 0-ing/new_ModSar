
#ifndef _ARA_CORE_UTILITY_H_
#define _ARA_CORE_UTILITY_H_

#include <initializer_list>
#include <iterator>
#include <cstddef>
#include <climits>

namespace ara {
namespace core {

namespace internal {

#ifdef __GNUC__
#ifndef ATTR_PACKED
#define APD_PACKED __attribute__((packed))
#else
// Do here what is necessary for achieving SWS_CORE_10101 for this compiler!
#define APD_PACKED
#endif
#endif

/**
 * \brief 函数说明
 * 这是一个基于类的实现。它满足所有要求，但也允许其他不符合要求的用途，特别是用括号初始化，
 * 例如“byte（42）”。这在基于std：：byte的实现中是不可能的，此不应该使用。
 * 实现定义了此类型是否可以用于类型别名而不会触发未定义的行为。
 * 
 * @traceid{SWS_CORE_10100}
 * @traceid{SWS_CORE_10101}
 * @traceid{SWS_CORE_10105}
 */

class Byte final {
    /// @traceid{SWS_CORE_10102}
    unsigned char mValue;

public:
    /**
     * \brief 函数说明
     * 此构造函数故意不初始化任何内容，因为默认初始化会使实例的值不确定。它还故意“在第一次声明
     * 时默认”，这样这个类就没有“用户提供的”默认构造函数，这对于实现std:：byte所需的等价性是
     * 必要的。
     * @traceid{SWS_CORE_10104}
     */
    Byte() noexcept = default;

    /**
     * \brief 从“unsigned char”转换构造函数
     * @traceid{SWS_CORE_10103}
     * @traceid{SWS_CORE_10106}
     */
    constexpr explicit Byte(unsigned char value) noexcept : mValue(value) {}
    /**
     * \brief 编译器生成的特殊成员函数很好，等效于以下函数：
     * 
     * constexpr byte(byte const& other) noexcept = default;
     * constexpr byte(byte&& other) noexcept = default;
     * constexpr byte& operator=(byte const& other) noexcept = default;
     * constexpr byte& operator=(byte&& other) noexcept = default;
     * ~byte() noexcept = default;
     */
    
    /**
     * \brief 将运算符转换为“unsigned char”
     * @traceid{SWS_CORE_10107}
     * @traceid{SWS_CORE_10108}
     */
    constexpr explicit operator unsigned char() const noexcept { return mValue; }
} APD_PACKED;

/// @traceid{SWS_CORE_10109}
constexpr bool operator==(Byte b1, Byte b2) noexcept {
    return (static_cast<unsigned char>(b1) == static_cast<unsigned char>(b2));
}

/// @traceid{SWS_CORE_10110}
constexpr bool operator!=(Byte b1, Byte b2) noexcept {
    return (static_cast<unsigned char>(b1) != static_cast<unsigned char>(b2));
}

}  // namespace internal

#if __cplusplus < 201703L
/// @traceid{SWS_CORE_04200}
using Byte = internal::Byte;
#else
/// @traceid{SWS_CORE_04200}
using Byte = std::byte;
#endif

// -------------------------------------------------------------------------

#if defined(__cpp_lib_nonmember_container_access) && (__cpp_lib_nonmember_container_access - 0) >= 201411
using std::data;
using std::size;
using std::empty;
#else
/**
 * \brief 返回指向包含容器元素的内存块的指针。
 * \tparam 用data（）方法include一个类型
 * \param c  Container的实例
 * \returns 指向容器的第一个元素的指针
 * 
 * @traceid{SWS_CORE_04110}
 */
template <typename Container>
constexpr auto data(Container& c) -> decltype(c.data()) {
    return c.data();
}

/**
 * \brief 向包含容器元素的内存块返回const_pointer
 * \tparam 用data（）方法include一个类型
 * \param c  Container的实例
 * \returns 指向容器的第一个元素的指针
 * 
 * @traceid{SWS_CORE_04111}
 */
template <typename Container>
constexpr auto data(Container const& c) -> decltype(c.data()) {
    return c.data();
}

/**
 * \brief 返回一个指针，指向包含原始数组元素的内存块。
 * \tparam T 数据类型
 * \tparam N 数据个数
 * \param array 对原始数组的引用
 * \returns 指向数组第一个元素的指针
 * 
 * @traceid{SWS_CORE_04112}
 */
template <typename T, std::size_t N>
constexpr T* data(T (&array)[N]) noexcept {
    return array;
}

/**
 * \brief 返回一个指向内存块的指针，该内存块包含std:：initializer_list的元素。
 * \tparam E std:：initializer_list中元素的类型
 * \param il  std:：initializer_list
 * \returns 指向容器的第一个元素的指针
 * 
 * @traceid{SWS_CORE_04113}
 */
template <typename E>
constexpr E const* data(std::initializer_list<E> il) noexcept {
    return il.begin();
}

/**
 * \brief 返回容器的大小。
 * \tparam 用data（）方法容器一个类型
 * \param c Container的实例
 * \returns array的大小
 * 
 * @traceid{SWS_CORE_04120}
 */
template <typename Container>
constexpr auto size(Container const& c) -> decltype(c.size()) {
    return c.size();
}

/**
 * \brief 返回给定的原始数组大小
 * \tparam T 数据类型
 * \tparam N 数据个数
 * \param array 对原始数组的引用
 * \returns array的大小，即N
 * 
 * @traceid{SWS_CORE_04121}
 */
template <typename T, std::size_t N>
constexpr std::size_t size(T const (&array)[N]) noexcept {
    return (void)array, N;  // The comma operator is used here to ignore unused variables inside a C++11 constexpr func
}

/**
 * \brief 返回给定容器是否为空。
 * \tparam 用empty（）方法容器一个类型
 * \param c Container的实例
 * \returns 如果容器为空，则为true，否则为false
 * 
 * @traceid{SWS_CORE_04130}
 */
template <typename Container>
constexpr auto empty(Container const& c) -> decltype(c.empty()) {
    return c.empty();
}

/**
 * \brief 返回给定的原始数组是否为空。
 * 
 * 由于原始数组在C++中不能有零个元素，因此此函数总是返回false。
 * 
 * \tparam T 数据类型
 * \tparam N 数据个数
 * \param array array
 * \returns false
 * 
 * @traceid{SWS_CORE_04131}
 */
template <typename T, std::size_t N>
constexpr bool empty(T const (&array)[N]) noexcept {
    return (void)array,
           false;  // The comma operator is used here to ignore unused variables inside a C++11 constexpr func
}

/**
 * \brief 返回给定的std:：initializer_list是否为空。
 * \tparam std:：initializer_list中元素的类型
 * \param il  the std::initializer_list
 * \returns 如果std:：initializer_list为空，则为true，否则为false
 * 
 * @traceid{SWS_CORE_04132}
 */
template <typename E>
constexpr bool empty(std::initializer_list<E> il) noexcept {
    return il.size() == 0;
}
#endif
// In-place construction
// @traceid{SWS_CORE_04011}
struct in_place_t {
    // @traceid{SWS_CORE_04012}
    explicit in_place_t() = default;  // NOLINT (runtime/explicit)
};
// @traceid{SWS_CORE_04013}
constexpr in_place_t in_place{ };

// @traceid{SWS_CORE_04021}
template <typename T>
struct in_place_type_t {
    using type = T;

    // @traceid{SWS_CORE_04022}
    explicit in_place_type_t() = default;  // NOLINT (runtime/explicit)
};

/// @traceid{SWS_CORE_04031}
template <std::size_t I>
struct in_place_index_t {
    static constexpr size_t value = I;

    // @traceid{SWS_CORE_04032}
    explicit in_place_index_t() = default;  // NOLINT (runtime/explicit)
};

template <std::size_t I>
constexpr in_place_index_t<I> in_place_index{ };

/**
 * variable templates are only available with -std=c++14
 *
 * template <typename T>
 * constexpr in_place_type_t<T> in_place_type;
 * template <typename T>
 * constexpr in_place_index_t<T> in_place_index;
 */


/** Downported and conforming C++11 implementation of the
 *  std::conjunction helper templates.
 */
template <typename...>
struct conjunction : std::true_type {};

template <typename B1>
struct conjunction<B1> : B1 {};

template <typename B1, typename B2>
struct conjunction<B1, B2> : std::conditional<B1::value, B2, B1>::type {};

template <typename B1, typename... Bs>
struct conjunction<B1, Bs...> : std::conditional<B1::value, conjunction<Bs...>, B1>::type {};

/** Downported and conforming C++11 implementation of the
 *  std::disjunction helper templates.
 */
template <typename...>
struct disjunction : std::false_type {};

template <typename B1>
struct disjunction<B1> : B1 {};

template <typename B1, typename B2>
struct disjunction<B1, B2> : std::conditional<B1::value, B1, B2>::type {};

template <typename B1, typename... Bs>
struct disjunction<B1, Bs...> : std::conditional<B1::value, B1, disjunction<Bs...>>::type {};

}  // namespace core
}  // namespace ara

#endif  // _ARA_CORE_UTILITY_H_
