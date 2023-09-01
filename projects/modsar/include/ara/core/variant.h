/**
 * \copyright BCSC all rights resvered
 * \author JJL
 * \date 2023/7/22
 */

#ifndef _ARA_CORE_VARIANTH
#define _ARA_CORE_VARIANTH

#include <string>
#include <stdio.h>
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <utility>
#include <memory>
#include <cassert>
#include <type_traits>
#include <tuple>

#include "ara/core/utility.h"

namespace ara
{
    namespace core
    {
        namespace internal
        {

            using byte_t = uint8_t;
            template <typename TypeToCheck, typename T, typename... Targs>
            struct does_contain_type
            {
                static constexpr bool value = std::is_same<TypeToCheck, T>::value || does_contain_type<TypeToCheck, Targs...>::value;
            };

            template <typename TypeToCheck, typename T>
            struct does_contain_type<TypeToCheck, T>
            {
                static constexpr bool value = std::is_same<TypeToCheck, T>::value;
            };

            template <std::size_t N, typename Type, typename T, typename... Targs>
            struct get_index_of_type
            {
                static constexpr std::size_t index = get_index_of_type<N + 1, Type, Targs...>::index;
            };

            template <std::size_t N, typename Type, typename... Targs>
            struct get_index_of_type<N, Type, Type, Targs...>
            {
                static constexpr std::size_t index = N;
            };

            template <std::size_t N, std::size_t Index, typename T, typename... Targs>
            struct get_type_at_index
            {
                using type = typename get_type_at_index<N + 1, Index, Targs...>::type;
            };

            template <std::size_t N, typename T, typename... Targs>
            struct get_type_at_index<N, N, T, Targs...>
            {
                using type = T;
            };

            template <std::size_t N, typename T, typename... Targs>
            struct call_at_index
            {
                static void destructor(const std::size_t index, byte_t *ptr)
                {
                    if (N == index)
                    {
                        reinterpret_cast<T *>(ptr)->~T();
                    }
                    else
                    {
                        call_at_index<N + 1, Targs...>::destructor(index, ptr);
                    }
                }

                static void move(const std::size_t index, byte_t *source, byte_t *destination)
                {
                    if (N == index)
                    {
                        *reinterpret_cast<T *>(destination) = std::move(*reinterpret_cast<T *>(source));
                    }
                    else
                    {
                        call_at_index<N + 1, Targs...>::move(index, source, destination);
                    }
                }

                static void moveConstructor(const std::size_t index, byte_t *source, byte_t *destination)
                {
                    if (N == index)
                    {
                        new (destination) T(std::move(*reinterpret_cast<T *>(source)));
                    }
                    else
                    {
                        call_at_index<N + 1, Targs...>::moveConstructor(index, source, destination);
                    }
                }

                static void copy(const std::size_t index, byte_t *source, byte_t *destination)
                {
                    if (N == index)
                    {
                        *reinterpret_cast<T *>(destination) = *reinterpret_cast<T *>(source);
                    }
                    else
                    {
                        call_at_index<N + 1, Targs...>::copy(index, source, destination);
                    }
                }

                static void copyConstructor(const std::size_t index, byte_t *source, byte_t *destination)
                {
                    if (N == index)
                    {
                        new (destination) T(*reinterpret_cast<T *>(source));
                    }
                    else
                    {
                        call_at_index<N + 1, Targs...>::copyConstructor(index, source, destination);
                    }
                }
            };

            template <std::size_t N, typename T>
            struct call_at_index<N, T>
            {
                static void destructor(const std::size_t index, byte_t *ptr)
                {
                    if (N == index)
                    {
                        reinterpret_cast<T *>(ptr)->~T();
                    }
                    else
                    {
                        assert(false && "Could not call destructor for variant element");
                    }
                }

                static void move(const std::size_t index, byte_t *source, byte_t *destination)
                {
                    if (N == index)
                    {
                        *reinterpret_cast<T *>(destination) = std::move(*reinterpret_cast<T *>(source));
                    }
                    else
                    {
                        assert(false && "Could not call move assignment for variant element");
                    }
                }

                static void moveConstructor(const std::size_t index, byte_t *source, byte_t *destination)
                {
                    if (N == index)
                    {
                        new (destination) T(std::move(*reinterpret_cast<T *>(source)));
                    }
                    else
                    {
                        assert(false && "Could not call move constructor for variant element");
                    }
                }

                static void copy(const std::size_t index, byte_t *source, byte_t *destination)
                {
                    if (N == index)
                    {
                        *reinterpret_cast<T *>(destination) = *reinterpret_cast<T *>(source);
                    }
                    else
                    {
                        assert(false && "Could not call copy assignment for variant element");
                    }
                }

                static void copyConstructor(const std::size_t index, byte_t *source, byte_t *destination)
                {
                    if (N == index)
                    {
                        new (destination) T(*reinterpret_cast<T *>(source));
                    }
                    else
                    {
                        assert(false && "Could not call copy constructor for variant element");
                    }
                }
            };

            template <typename TypeToCheck, typename... Targs>
            struct are_move_constructible
            {
                static constexpr bool value = std::is_move_constructible<TypeToCheck>::value && are_move_constructible<Targs...>::value;
            };

            template <typename TypeToCheck>
            struct are_move_constructible<TypeToCheck>
            {
                static constexpr bool value = std::is_move_constructible<TypeToCheck>::value;
            };

            /** 
             * Variant Converting Constructor实现需要以下辅助结构和类型。
             */

            using index_type = std::size_t;
            static constexpr index_type invalid_index = index_type(-1);

            /** 
             * 递归模板结构，用于检索要复制/移动到变量中的T类型的潜在索引。
             */
            template <typename T, typename... Types>
            struct get_direct_type_index;

            template <typename T, typename N, typename... Types>
            struct get_direct_type_index<T, N, Types...>
            {
                static constexpr index_type index = std::is_same<T, N>::value ? sizeof...(Types) : get_direct_type_index<T, Types...>::index;
            };

            template <typename T>
            struct get_direct_type_index<T>
            {
                static constexpr index_type index = invalid_index;
            };

            /** 
             * Recursive template structs, to check the type convertibility of the the potential type T
             * to be copy or move assigned into the Variant against the types held in the Variant.
             * 递归模板结构，用于检查要复制或移动到变量中的潜在类型T的类型可转换性，以及变量中的类型。
             */
            template <typename T, typename... Types>
            struct get_convertible_type_index;

            template <typename T, typename N, typename... Types>
            struct get_convertible_type_index<T, N, Types...>
            {
                static constexpr index_type index = std::is_convertible<T, N>::value
                                                        ? disjunction<std::is_convertible<T, Types>...>::value ? invalid_index : sizeof...(Types)
                                                        : get_convertible_type_index<T, Types...>::index;
            };

            template <typename T>
            struct get_convertible_type_index<T>
            {
                static constexpr index_type index = invalid_index;
            };

            /** 
             * The following structure contains the relevant details regarding the type T passed to the Variant
             * converting constructor - potential index of T in the Variant retrieved either by directly checking
             * against the contained types, or looking up potential conversion of T into one of the Variant types
             * and retrieving the matched index. Finally, the type inside the Variant best fit for T is deduced
             * from a tuple constructed of void (if supported in future) and Types... from the Variant.
             * 以下结构包含有关传递给Variant转换构造函数的类型T的相关详细信息-通过直接检查包含的类型，或查找T到其中一个Variant类型的潜在转换
             * 并检索匹配的索引，检索Variant中T的潜在索引。最后，Variant中最适合T的类型是从一个由void（如果将来支持的话）和Types…构造的元组
             * 中推导出来的。。。来自变体。
             */
            template <typename T, typename... Types>
            struct T_detail
            {
                using T_val = typename std::remove_const<typename std::remove_reference<T>::type>::type;
                static constexpr index_type T_direct = get_direct_type_index<T_val, Types...>::index;
                static constexpr bool T_present = T_direct != invalid_index;
                static constexpr index_type T_index = T_present ? T_direct : get_convertible_type_index<T_val, Types...>::index;
                static constexpr bool T_valid = T_index != invalid_index;
                static constexpr index_type T_final = T_valid ? sizeof...(Types) - T_index : 0;
                static constexpr index_type T_varIndex = T_final - 1;
                using T_varType = typename std::tuple_element<T_final, std::tuple<void, Types...>>::type;
            };

            /**
             * std::max is not constexpr in c++11, c++14 could be used instead
             * std:：max在c++11中不是constexpr，可以使用c++14
             */
            template <typename T>
            constexpr T max(const T &a)
            {
                return a;
            }
            template <typename T>
            constexpr T max(const T &a, const T &b)
            {
                return (b < a) ? a : b;
            }
            template <typename T, typename... Targs>
            constexpr T max(const T &a, const T &b, const Targs &...args)
            {
                return max(max(a, b), args...);
            }

        } // namespace internal

        /**
         * \brief value which an invalid variant index occupies 无效变量索引占用的值
         * \code  
         *     ara::core::Variant<int, float> someVariant;
         *
         *     // every unset variant has an invalid variant in the beginning
         *     if ( someVariant.index() == INVALID_VARIANT_INDEX ) ...
         *
         *     ara::core::Variant<int, float> someVariant2(ara::core::in_place_type<int>, 12);
         *
         *     // variant with setted value therefore the index is not invalid
         *     if ( someVariant.index() != INVALID_VARIANT_INDEX ) ...
         */
       
        static constexpr std::size_t INVALID_VARIANT_INDEX = std::size_t(-1);
        
        /**
         * \brief C++17标准与C++11的变体实现。该接口的灵感来自C++17标准，但由于不允许抛出异常，
         *        因此在get和template方面有所更改。
         * \param Types... 变体应该能够存储的类型的可变列表
         * \code  
         * 
         *      #include "ara/core/variant.h"
         *      #include <iostream>
         *      ara::core::Variant<int, float, double> someVariant;
         *      // ... do stuff
         *      if ( someVariant.index() == INVALID_VARIANT_INDEX )
         *      {
         *          someVariant.emplace<float>(123.456f);
         *      }
         *      else if ( someVariant.index() == 1)
         *      {
         *          auto blubb = someVariant.template get_at_index<1>();
         *          std::cout << *blubb << std::endl;
         *          std::cout << *blubb << std::endl;\
         *          auto sameAsBlubb = someVariant.get<float>();
         *          std::cout << *sameAsBlubb << std::endl;
         *      }
         *      // .. do stuff
         *      int defaultValue = 123;
         *      int * fuu = someVariant.get_if<int>(&defaultValue);
         *      std::cout << *fuu << std::endl;
         */

        /**
        * ara::core::Variant代表一种类型安全的union。
        * 1、与union相同点：可以包含多种类型的数据，但任何时刻最多只能存放其中一种类型的数据。
        * 2、与union不同点：Variant无须借助外力只需要通过查询自身就可辨别实际所存放数据的类型。
        * 本质上是对union的抽象和封装
        */


        /// @traceid{SWS_CORE_01601}

        template <typename... Types>
        class Variant
        {
        private:
            /**
            * \brief 函数说明 包含最大元素的大小
            */
            static constexpr std::size_t TYPE_SIZE = internal::max(sizeof(Types)...);

        public:
            /**
            * \brief 默认构造函数构造不包含元素的变体，并在调用.INDEX（）时返回INVALID_variant_INDEX
            */
       
            Variant() = default;
        
            /**
            * \brief   inplace构造。原地构造一个初始化为第I个类型的Variant，如果I不小于Variant可选类型的数量， 
            *          第I个类型数次出现不止一次或由args不能构造出第I个类型，则该构造函数不可用。
            * \param[in] index index of the type which should be constructed 应该构造的类型的索引
            * \param[in] args variadic list of arguments which will be forwarded to the constructor to the type at index
            *            将转发给inde处类型的构造函数的变参数列表                    
            */
            template <std::size_t N, typename... CTorArguments>
            Variant(const in_place_index_t<N> &index, CTorArguments &&...args) noexcept; // NOLINT (runtime/explicit)

            /**
            * \brief   inplace构造。由il原地构造一个初始化为第I个类型T类型的Variant，如果I不小于Variant中可选类型的数量或T类型
            *          在声明的Variant中出现多次或由args和il不能构造出T，则该构造函数不可用。 
            * \param[in] index index of the type which should be constructed 应该构造的类型的索引
            * \param[in] f_inir initialization list to construct the class at index  在索引处构造类的初始化列表  
            * \param[in] f_args variadic list of arguments which will be forwarded to the constructor to the class    
            *          将转发给类的构造函数的参数的可变列表         
            */
            template <std::size_t ClassIndex, class U, class... CTorArguments>
            explicit Variant(in_place_index_t<ClassIndex> index, std::initializer_list<U> f_inir, CTorArguments &&...f_args);

            /**
            * \brief 创建一个变量并执行类型T的就地构造。如果T不是变量的一部分，则会出现编译器错误。
            *        inplace构造。原地构造一个初始化为T类型的Variant，如果T类型在声明的Variant中出现多次或由args不能构造出T，
            *        则该构造函数不可用。
            * \param[in] type type which should be created inside the variant 应该在变体内部创建的类型
            * \param[in] args variadic list of arguments which will be forwarded to the constructor to the type
            *                 将转发给该类型的构造函数的参数的可变列表                   
            */
            template <typename T, typename... CTorArguments>
            Variant(const in_place_type_t<T> &type, CTorArguments &&...args) noexcept; // NOLINT (runtime/explicit)
            
            /**
            * \brief inplace构造。由il原地构造一个初始化为T类型的Variant，如果T类型在声明的Variant中出现多次或由args和il
            *        不能构造出T，则该构造函数不可用。
            * \param[in] T 应该构造的类
            * \param[in] f_inir 将转发给该类型的构造函数的参数的可变列表
            * \param[in] f_args 将转发给类的构造函数的参数的可变列表                
            */
            template <class T, class U, class... CTorArguments>
            explicit Variant(in_place_type_t<T>, std::initializer_list<U> f_inir, CTorArguments &&...f_args);

            /**
            * \brief 拷贝构造 如果变量包含元素，则调用元素复制构造函数，否则复制空变量
            * \param[in] rhs 源副本
            */
            Variant(const Variant &rhs) noexcept;

            /**
            * \brief 如果变量包含元素，则调用元素复制赋值运算符，否则将复制空变量
            * \param[in] rhs 副本分配的来源
            * \return  对变体本身的引用
            */
            Variant &operator=(const Variant &rhs) noexcept;

            /**
            * \brief 移动构造 如果变量包含元素，则调用元素move构造函数，否则将移动空变量
            * @param[in] rhs 移动源副本
            */
            Variant(Variant &&rhs) noexcept;

            /**
            * \brief 如果变量包含元素，则调用元素移动赋值运算符，否则移动空变量
            * \param[in] rhs 源副本
            * \return  对变体本身的引用
            */
            Variant &operator=(Variant &&rhs) noexcept;

            /**
            * \brief 如果变量包含元素，则调用元素析构函数，否则不会发生任何事情
            */
            ~Variant() noexcept;
            
            /**
            * \brief 转换构造函数使用通用引用模板化的类型T&&，然后解析该类型以在<Types…>中找到直接匹配，
            *        或者使用迭代函子构造每个T_ j的可能重载，然后解析为在＜Types…＞中找到T的最佳匹配。未能解决将引发编译时错误，
            *        这在运行时没有任何保证。
            * \param[in] rhs 源对象
            */
            template <typename T,
                      typename U = internal::T_detail<T, Types...>,
                      typename Enable = typename std::enable_if<U::T_valid && !std::is_same<Variant<Types...>, typename U::T_val>::value>::type>
            Variant(T &&rhs) noexcept(std::is_nothrow_constructible<typename U::T_varType, T &&>::value); // NOLINT (runtime/explicit)
            
            /**
            * \brief 如果变量包含元素，则调用元素赋值运算符，否则我们将有未定义的行为。重要的是，你要确保变体真的包含T类型。
            * \param[in] rhs 基础移动分配的源对象
            * \return 本身的引用
            */
            template <typename T>
            typename std::enable_if<!std::is_same<T, Variant<Types...> &>::value, Variant<Types...>>::type &
            operator=(T &&rhs) noexcept;

            /**
            * \brief 将参数转发到template_at_index（）
            * \param TypeIndex 将创建的类型的索引
            * \param[in] args 将转发到TypeIndex处类型的构造函数的参数
            */
            template <std::size_t TypeIndex, typename... CTorArguments>
            void emplace(CTorArguments &&...args) noexcept;
            
            /**
            * \brief 调用索引TypeIndex处类型的构造函数，并将参数完美地转发到此构造函数。（不符合stl）
            * \param TypeIndex 将创建的类型的索引
            * \param[in] args 将转发到TypeIndex处类型的构造函数的参数
            */
            template <std::size_t TypeIndex, typename... CTorArguments>
            void emplace_at_index(CTorArguments &&...args) noexcept;
            
            /**
            * \brief 调用类型T的构造函数，并将参数完美地转发给T的构造函数。
            */
            template <typename T, typename... CTorArguments>
            void emplace(CTorArguments &&...args) noexcept;
            
            /**
            * \brief 在ClassIndex调用类的构造函数，并将类U和参数的初始值设定项列表完美地转发给构造函数。
            */
            template <std::size_t ClassIndex, class U, class... CTorArguments>
            void emplace(std::initializer_list<U> f_inir, CTorArguments &&...f_args);
            
            /**
            * \brief 调用类T的构造函数，并将类U和参数的初始值设定项列表完美地转发给构造函数。
            */
            template <class T, class U, class... CTorArguments>
            void emplace(std::initializer_list<U> f_inir, CTorArguments &&...f_args);
            
            /**
            * \brief 返回指向存储在索引TypeIndex中的类型的指针。（不符合stl）
            * \return 如果变量确实包含索引TypeIndex处的类型，则返回一个有效指针；如果变量根本不包含类型或包含不同的类型，那么返回nullptr。
            * \code 
            *       cxx::variant<int, float> someVariant(cxx::in_place_type<int>(), 12);
            *       int * someNumber = someVariant.template get_at_index<0>();
            */
         
            template <std::size_t TypeIndex>
            typename internal::get_type_at_index<0, TypeIndex, Types...>::type *get_at_index() noexcept;
            
            /**
            * \brief 返回指向存储在索引TypeIndex中的类型的指针。（不符合stl）
            * \return 如果变量确实包含索引TypeIndex处的类型，则返回一个有效指针；如果变量根本不包含类型或包含不同的类型，那么返回nullptr。
            * \code 
            *       cxx::variant<int, float> someVariant(cxx::in_place_type<int>(), 12);
            *       int * someNumber = someVariant.template get_at_index<0>();
            */
            template <std::size_t TypeIndex>
            const typename internal::get_type_at_index<0, TypeIndex, Types...>::type *get_at_index() const noexcept;

            
            /**
            * \brief 返回一个指向变量中存储的类型T的指针。（不符合stl）
            * \return  如果变体确实包含类型T，则返回有效指针；否则，如果变体根本不包含类型或不包含其他类型，则返回nullptr
            */
            template <typename T>
            const T *get() const noexcept;
            
            /**
            * \brief 返回一个指向变量中存储的类型T的指针。（不符合stl）
            * \return  如果变体确实包含类型T，则返回有效指针；否则，如果变体根本不包含类型或不包含其他类型，则返回nullptr
            */
            template <typename T>
            T *get() noexcept;
            
            /**
            * \brief 如果类型T存储在变体中，则返回指向该类型的指针；否则返回提供的defaultValue
            * \return  如果存储值的类型为T，则指向该值的指针，否则为defaultValue
            */
            template <typename T>
            T *get_if(T *defaultValue) noexcept;

            /**
            * \brief 如果类型T存储在变体中，则返回指向该类型的指针，否则返回提供的默认值
            * \return  如果存储值的类型为T，则指向该值的指针，否则为defaultValue
            */
            template <typename T>
            const T *get_if(const T *defaultValue) const noexcept;
            
            /**
            * \brief 检查变量是否有值
            * \return  当且仅当变量包含值时返回false
            */
            constexpr bool valueless_by_exception() const noexcept;

            /**
            * \brief 返回变量中存储类型的索引。如果变体不包含任何类型，则返回INVALID_variant_INDEX
            * \return  存储类型的索引
            */
            constexpr size_t index() const noexcept;

            /**
            * \brief Lvalue和rhs的互换价值
            */
            void swap(Variant &rhs) noexcept;

        private:
            alignas(internal::max(alignof(Types)...)) internal::byte_t m_storage[TYPE_SIZE]{0u};
            std::size_t m_type_index = INVALID_VARIANT_INDEX;

        private:
            template <typename T>
            bool has_bad_variant_element_access() const noexcept;
            static void error_message(const char *f_source, const char *f_msg) noexcept;

            void call_element_destructor() noexcept;
        };

        /// @brief returns true if the variant holds a given type T, otherwise false
        template <typename T, typename... Types>
        constexpr bool holds_alternative(const Variant<Types...> &f_variant) noexcept;

        template <class T, class... Types>
        constexpr T *get_if(Variant<Types...> *f_variant_ptr) noexcept;

        template <class T, class... Types>
        constexpr const T *get_if(const Variant<Types...> *f_variant_ptr) noexcept;

        // implemetation
        template <typename... Types>
        inline Variant<Types...>::Variant(const Variant &f_rhs) noexcept : m_type_index(f_rhs.m_type_index)
        {
            if (m_type_index != INVALID_VARIANT_INDEX)
            {
                internal::call_at_index<0, Types...>::copyConstructor(
                    m_type_index, const_cast<internal::byte_t *>(f_rhs.m_storage), m_storage);
            }
        }

        template <typename... Types>
        template <std::size_t N, typename... CTorArguments>
        inline Variant<Types...>::Variant(const in_place_index_t<N> &, CTorArguments &&...f_args) noexcept
        {
            emplace_at_index<N>(std::forward<CTorArguments>(f_args)...);
        }

        template <typename... Types>
        template <std::size_t ClassIndex, class U, class... CTorArguments>
        Variant<Types...>::Variant(in_place_index_t<ClassIndex>, std::initializer_list<U> f_inir, CTorArguments &&...f_args)
        {
            emplace<ClassIndex>(f_inir, std::forward<CTorArguments>(f_args)...);
        }

        template <typename... Types>
        template <typename T, typename... CTorArguments>
        inline Variant<Types...>::Variant(const in_place_type_t<T> &, CTorArguments &&...f_args) noexcept
        {
            emplace<T>(std::forward<CTorArguments>(f_args)...);
        }

        template <typename... Types>
        template <class T, class U, class... CTorArguments>
        Variant<Types...>::Variant(in_place_type_t<T>, std::initializer_list<U> f_inir, CTorArguments &&...f_args)
        {
            emplace<T>(f_inir, std::forward<CTorArguments>(f_args)...);
        }

        template <typename... Types>
        Variant<Types...> &Variant<Types...>::operator=(const Variant &f_rhs) noexcept
        {
            if (this != &f_rhs)
            {
                if (m_type_index != f_rhs.m_type_index)
                {
                    call_element_destructor();
                    m_type_index = f_rhs.m_type_index;

                    if (m_type_index != INVALID_VARIANT_INDEX)
                    {
                        internal::call_at_index<0, Types...>::copyConstructor(
                            m_type_index, const_cast<internal::byte_t *>(f_rhs.m_storage), m_storage);
                    }
                }
                else
                {
                    if (m_type_index != INVALID_VARIANT_INDEX)
                    {
                        internal::call_at_index<0, Types...>::copy(
                            m_type_index, const_cast<internal::byte_t *>(f_rhs.m_storage), m_storage);
                    }
                }
            }
            return *this;
        }

        template <typename... Types>
        Variant<Types...>::Variant(Variant &&f_rhs) noexcept : m_type_index{std::move(f_rhs.m_type_index)}
        {
            if (m_type_index != INVALID_VARIANT_INDEX)
            {
                internal::call_at_index<0, Types...>::moveConstructor(m_type_index, f_rhs.m_storage, m_storage);
            }
            f_rhs.m_type_index = INVALID_VARIANT_INDEX;
        }

        template <typename... Types>
        Variant<Types...> &Variant<Types...>::operator=(Variant &&f_rhs) noexcept
        {
            if (this != &f_rhs)
            {
                if (m_type_index != f_rhs.m_type_index)
                {
                    call_element_destructor();
                    m_type_index = std::move(f_rhs.m_type_index);
                    if (m_type_index != INVALID_VARIANT_INDEX)
                    {
                        internal::call_at_index<0, Types...>::moveConstructor(m_type_index, f_rhs.m_storage, m_storage);
                    }
                    f_rhs.m_type_index = INVALID_VARIANT_INDEX;
                }
                else
                {
                    if (m_type_index != INVALID_VARIANT_INDEX)
                    {
                        internal::call_at_index<0, Types...>::move(m_type_index, f_rhs.m_storage, m_storage);
                    }
                }

                f_rhs.m_type_index = INVALID_VARIANT_INDEX;
            }
            return *this;
        }

        template <typename... Types>
        inline Variant<Types...>::~Variant() noexcept
        {
            call_element_destructor();
        }

        template <typename... Types>
        void Variant<Types...>::call_element_destructor() noexcept
        {
            if (m_type_index != INVALID_VARIANT_INDEX)
            {
                internal::call_at_index<0, Types...>::destructor(m_type_index, m_storage);
            }
        }

        template <typename... Types>
        template <typename T, typename U, typename Enable>
        Variant<Types...>::Variant(T &&f_rhs) noexcept(std::is_nothrow_constructible<typename U::T_varType, T &&>::value)
            : m_type_index(U::T_varIndex)
        {
            new (m_storage) typename U::T_varType(std::forward<T>(f_rhs));
        }

        template <typename... Types>
        template <typename T>
        inline typename std::enable_if<!std::is_same<T, Variant<Types...> &>::value, Variant<Types...>>::type &
        Variant<Types...>::operator=(T &&f_rhs) noexcept
        {
            Variant<Types...> temp{std::forward<T>(f_rhs)};

            if (m_type_index != INVALID_VARIANT_INDEX)
            {
                call_element_destructor();
            }

            internal::call_at_index<0, Types...>::moveConstructor(temp.m_type_index, temp.m_storage, m_storage);
            m_type_index = std::move(temp.m_type_index);

            return *this;
        }

        template <typename... Types>
        template <std::size_t TypeIndex, typename... CTorArguments>
        inline void Variant<Types...>::emplace(CTorArguments &&...f_args) noexcept
        {
            Variant<Types...>::emplace_at_index<TypeIndex, CTorArguments...>(std::forward<CTorArguments>(f_args)...);
        }

        template <typename... Types>
        template <std::size_t TypeIndex, typename... CTorArguments>
        inline void Variant<Types...>::emplace_at_index(CTorArguments &&...f_args) noexcept
        {
            static_assert(TypeIndex <= sizeof...(Types), "TypeIndex is out of bounds");
            using T = typename internal::get_type_at_index<0, TypeIndex, Types...>::type;
            call_element_destructor();
            new (m_storage) T(std::forward<CTorArguments>(f_args)...);
            m_type_index = TypeIndex;
        }

        template <typename... Types>
        template <typename T, typename... CTorArguments>
        inline void Variant<Types...>::emplace(CTorArguments &&...f_args) noexcept
        {
            static_assert(internal::does_contain_type<T, Types...>::value, "variant does not contain given type");
            if (m_type_index != INVALID_VARIANT_INDEX)
            {
                call_element_destructor();
            }
            new (m_storage) T(std::forward<CTorArguments>(f_args)...);
            m_type_index = internal::get_index_of_type<0, T, Types...>::index;
        }

        template <typename... Types>
        template <std::size_t ClassIndex, class U, class... CTorArguments>
        void Variant<Types...>::emplace(std::initializer_list<U> f_inir, CTorArguments &&...f_args)
        {

            static_assert(ClassIndex <= sizeof...(Types), "ClassIndex is out of bounds");
            using T = typename internal::get_type_at_index<0, ClassIndex, Types...>::type;
            call_element_destructor();
            new (m_storage) T(f_inir, std::forward<CTorArguments>(f_args)...);
            m_type_index = ClassIndex;
        }

        template <typename... Types>
        template <class T, class U, class... CTorArguments>
        void Variant<Types...>::emplace(std::initializer_list<U> f_inir, CTorArguments &&...f_args)
        {

            static_assert(internal::does_contain_type<T, Types...>::value, "variant does not contain given type");
            call_element_destructor();
            new (m_storage) T(f_inir, std::forward<CTorArguments>(f_args)...);
            m_type_index = internal::get_index_of_type<0, T, Types...>::index;
        }

        template <typename... Types>
        template <std::size_t TypeIndex>
        inline typename internal::get_type_at_index<0, TypeIndex, Types...>::type *Variant<Types...>::get_at_index() noexcept
        {
            if (TypeIndex != m_type_index)
            {
                return nullptr;
            }

            using T = typename internal::get_type_at_index<0, TypeIndex, Types...>::type;

            return static_cast<T *>(static_cast<void *>(m_storage));
        }

        template <typename... Types>
        template <std::size_t TypeIndex>
        inline const typename internal::get_type_at_index<0, TypeIndex, Types...>::type *Variant<Types...>::get_at_index() const
            noexcept
        {
            using T = typename internal::get_type_at_index<0, TypeIndex, Types...>::type;
            return const_cast<const T *>(const_cast<Variant *>(this)->get_at_index<TypeIndex>());
        }

        template <typename... Types>
        template <typename T>
        inline const T *Variant<Types...>::get() const noexcept
        {
            if (has_bad_variant_element_access<T>())
            {
                return nullptr;
            }
            else
            {
                return static_cast<const T *>(static_cast<const void *>(m_storage));
            }
        }

        template <typename... Types>
        template <typename T>
        inline T *Variant<Types...>::get() noexcept
        {
            return const_cast<T *>(const_cast<const Variant *>(this)->get<T>());
        }

        template <typename... Types>
        template <typename T>
        inline T *Variant<Types...>::get_if(T *f_default_value) noexcept
        {
            return const_cast<T *>(const_cast<const Variant *>(this)->get_if<T>(const_cast<const T *>(f_default_value)));
        }

        template <typename... Types>
        template <typename T>
        inline const T *Variant<Types...>::get_if(const T *f_default_value) const noexcept
        {
            if (has_bad_variant_element_access<T>())
            {
                return f_default_value;
            }
            return this->get<T>();
        }

        template <typename... Types>
        constexpr bool Variant<Types...>::valueless_by_exception() const noexcept
        {
            return (m_type_index == INVALID_VARIANT_INDEX);
        }

        template <typename... Types>
        constexpr size_t Variant<Types...>::index() const noexcept
        {
            return m_type_index;
        }

        template <typename... Types>
        void Variant<Types...>::swap(Variant &rhs) noexcept
        {
            static_assert(internal::are_move_constructible<Types...>::value, "Not all types is move constructible");
            if (valueless_by_exception() && rhs.valueless_by_exception())
                return;
            std::swap(*this, rhs);
        }

        template <typename... Types>
        template <typename T>
        inline bool Variant<Types...>::has_bad_variant_element_access() const noexcept
        {
            static_assert(internal::does_contain_type<T, Types...>::value, "variant does not contain given type");
            return (m_type_index != internal::get_index_of_type<0, T, Types...>::index);
        }

        template <typename T, typename... Types>
        inline constexpr bool holds_alternative(const Variant<Types...> &f_variant) noexcept
        {
            return f_variant.template get<T>() != nullptr;
        }

        template <class T, class... Types>
        constexpr T *get_if(Variant<Types...> *f_variant_ptr) noexcept
        {
            return f_variant_ptr->template get<T>();
        }

        template <class T, class... Types>
        constexpr const T *get_if(const Variant<Types...> *f_variant_ptr) noexcept
        {
            return f_variant_ptr->template get<T>();
        }

    } // namespace core

} // namespace ara

#endif // _ARA_CORE_VARIANTH