#include <type_traits>

/**
 * 主要检查类型的条件满足要求
 * std::enable_if<CONIDTION,TYPE> 如果Condition满足 则使用 TYPE ,否则是空类型
 * 1. 可用在条件特例化 等价为 switch 语义
 * 2. 可以用来检查符合条件的类型 否则编译失败
 */


template<typename T>
typename std::enable_if<(sizeof(T) > 2),T>::type funceb()
{
    T myt = {};
    return myt;
}
#if 0
template<typename T>
typename std::enable_if<(sizeof(T) == 1),T>::type funceb()
{
    T myt = {};
    return myt;
}
#endif
template<typename T,typename = typename std::enable_if<(sizeof(T)>2),void>::type >
class TCLASS{
public:
    T say(){}
};

template <bool Condition, typename U = void>
using enable_if_t = typename std::enable_if<Condition,U>::type;

template<typename T, typename U = void>
class TCLASS1;
template<typename T >
class TCLASS1<T,enable_if_t<(sizeof(T)>2)> > {

};
template<typename T >
class TCLASS1<T,enable_if_t<(sizeof(T)==1)> > {
public:
    void a(){}
};

int main()
{
    funceb<int>();
    funceb<char>();
    TCLASS<int> a();
    TCLASS1<char> b;
    b.a();
}
