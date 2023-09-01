/**
 * \copyright BCSC all rights resvered
 * \author JJL
 * \date 2023/7/22
 */
#ifndef ARA_COM_CORE_H_
#define ARA_COM_CORE_H_
#include <set>
namespace ara
{
    namespace core
    {
        template <typename VALUE>
        using Set = std::set<VALUE>;
    } // namespace core

} // namespace ara

#endif // ARA_COM_CORE_H_