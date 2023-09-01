/**
 * \copyright bcsc all rights reseverd
 * \brief 简单说明
 * \author JJL
 * \date 2023/7/8
 */
#ifndef _ARA_COM_TYPES_H
#define _ARA_COM_TYPES_H

#include <string>
#include <functional>

namespace ara
{
    namespace com
    {
        /**
         * @brief Identifier of a certain instance of a service.
         *
         * This is needed to distinguish between different instances of exactly the same
         * service in the system.
         * Only contains instance information. Does NOT contain a fully qualified
         * name, which also contains service type information.
         *
         * @note The implementation might be changed by product vendor.
         * Only the Autosar Adaptive API must be preserved.
         *
         * @remark
         * @ID{[SWS_CM_00302]} // 需要标注具体AP文档的code 具体参考 vrte or AOS
         *
         * @todo
         * As per specification of COM & TPS, eventually we will need ALL information about a Service Instance
         * included in here:
         *
         * - instanceId (some integer)
         * - deployment info (Ipc|SomeIp|Dds...)
         *
         * Currently the constructor string is not used in that way, values look like "42" or "myInstance".
         * As the instanceId does not include the deployment info yet, we need to insert this information from the
         * generated Proxy/Skeleton code. Only the generated code knows whether Ipc or Some/Ip deployments have been generated.
         */
        class InstanceIdentifier
        {
        public:
        private:
            /**
             * @brief The (legacy) string value.
             *
             * @remark
             * We have to keep this as long as users don't directly deliver the right serialized string view in all cases.
             */
            std::string idString_;

            /** @brief The instance id. */
            uint32_t id_;
        };
        /**
         * Container for a list of service handles.
         *
         * \see ara::com::FindService
         *
         * \note This implementation might be changed by product vendor.
         * At least the container implementation must be compliant to C++11
         * containers according to: http://en.cppreference.com/w/cpp/concept/Container
         *
         * * \remark
         * @ID{[SWS_CM_00383]}
         */
        template <typename HandleType>
        using ServiceHandleContainer = ara::core::Vector<HandleType>;

        /**
         * \brief Function wrapper for handler, that gets called in case service availability
         * for services, which have been searched for via FindService() has changed.
         *
         * \remark
         * @ID{[SWS_CM_00383]}
         */
        template <typename HandleType>
        using FindServiceHandler = std::function<void(ServiceHandleContainer<HandleType>, FindServiceHandle)>;

        /**
         * \brief Receive handler method, which is semantically a void(void) function.
         *
         * \note This implementation might be changed by product vendor.
         *
         * \remark
         * @ID{[SWS_CM_00309]}
         */
        using EventReceiveHandler = std::function<void(void)>;

    } // namespace com

} // namespace ara

#endif // _ARA_COM_TYPES_H