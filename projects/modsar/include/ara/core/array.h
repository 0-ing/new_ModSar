/**
 * \copyright BCSC all rights resvered
 * \author JJL
 * \date 2023/7/22
 */
#ifndef ARA_COM_CORE_H_
#define ARA_COM_CORE_H_
#include <array>
namespace ara
{
    namespace core
    {
        /**
         * SWS_CORE_01201
         * SWS_CORE_11200
         */
        template <class T, std::size_t N>
        using Array = std::array<T, N>;
    } // namespace core

} // namespace ara

#endif // ARA_COM_CORE_H_