/**
 * \copyright BCSC all rights resvered
 * \author JJL
 * \date 2023/7/22
 */

#ifndef ARA_CORE_STRING_VIEW_H_
#define ARA_CORE_STRING_VIEW_H_
#include <string>
#include <iterator>
namespace ara
{
    namespace core
    {
        /**
         * SWS_CORE_02001
         * 此stringview类型构建一个字符序列的只读视图，该字符序列的生命周期由使用的对象负责保证。
         * if有成员和支持的结构（如全局关系运算符）应遵循C++17的std::string_view，但不使用constexpr声明非const成员函数。
         */
        class StringView
        {
        private:
            char const *value_ = 0; // 字符串
            int size_ = 0;          // 字符串长度

            using value_type = char;
            using size_type = int;
            using pointer = char *;
            using const_pointer = char const *;
            using reference = char &;
            using const_reference = char const &;
            using const_iterator = char const *;
            using iterator = const_iterator;
            using const_reverse_iterator = std::reverse_iterator<const_iterator>;
            using reverse_iterator = const_reverse_iterator;

        public:
            constexpr StringView(){};
            constexpr StringView(const char *value, int size) : value_(value), size_(size)
            {
            }

            constexpr StringView(const char *p) : StringView(p, p == nullptr ? 0 : std::char_traits<char>::length(p)) {} // NOLINT (runtime/explicit)

            // constexpr StringView(std::string const &s) : StringView(s.c_str(), s.size()) {} // NOLINT (runtime/explicit)

            constexpr StringView(StringView const &other) noexcept = default;

            // Not "constexpr" because that would make it also "const" on C++11 compilers.
            StringView &operator=(StringView const &other) noexcept = default;

            // 24.4.2.2, iterator support
            constexpr const_iterator begin() const noexcept { return value_; }

            constexpr const_iterator end() const noexcept { return value_ + size_; }

            const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(end()); }

            const_reverse_iterator rend() const noexcept { return const_reverse_iterator(begin()); }

            // 24.4.2.3, capacity
            constexpr size_type size() const noexcept { return size_; }
            constexpr size_type length() const noexcept { return size_; }

            constexpr bool empty() const noexcept { return size_ == 0; }

            // 24.4.2.4, element access
            constexpr const_reference operator[](size_type pos) const { return value_[pos]; }
            inline const_reference at(size_type pos) const
            {
                // compound statements are not allowed in C++11
                if (pos >= size_)
                {
                    std::terminate();
                }
                return value_[pos];
            }
            constexpr const_reference front() const { return value_[0]; }
            constexpr const_reference back() const { return value_[size_ - 1]; }
            constexpr const_pointer data() const noexcept { return value_; }
            ~StringView() {}
        };

    } // namespace core

} // namespace ara

#endif // ARA_CORE_STRING_VIEW_H_
