#include <type_traits>
#include <stdio.h>

/**
 * 检查类是否有指定参数的构建方式
 * std::is_constructible<T,Args..> 检查T 是否可以用 Args... 指定的参数列表来初始化
 */

struct A{

};

struct A1 {
    A1(A1&&){}
};

struct A2 {
    A2(int){}
    A2(A2&&) = delete;
    A2& operator=(A2&) = delete;
};


int main()
{
    printf("%d\n",std::is_constructible<int>::value);
    printf("%d\n",std::is_constructible<A1,A1&&>::value);
    printf("%d\n",std::is_move_constructible<A1>::value);
    printf("%d\n",std::is_move_constructible<A2>::value);
}
