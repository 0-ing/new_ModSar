#ifndef _COM_SD_RUNTIME_H
#define _COM_SD_RUNTIME_H
namespace com
{
    namespace sd
    {
        /**
         * @brief Interface for a Runtime supporting AUTOSAR Service Discovery.
         *
         * The IRuntime interface provides callbacks for Proxies and Skeletons,
         * which are required to support Service Discovery.
         */
        class IRuntime
        {
        public:
            /** @brief Virtual destructor for proper destruction via base pointers. */
            virtual ~IRuntime() = default;

            /**
             * @brief Register a Skeleton at the Runtime.
             */
            virtual Result<void> registerSkeleton(ServiceId serviceId, const InstanceIdentifier &instanceIdentifier) = 0;

            /**
             * @brief Unregister a Skeleton at the Runtime.
             */
            virtual void unregisterSkeleton(ServiceId serviceId, const InstanceIdentifier &instanceIdentifier) = 0;

            /**
             * @brief Register a Proxy at the Runtime.
             */
            virtual Result<void> registerProxy(ServiceId serviceId, const InstanceIdentifier &instanceIdentifier) = 0;

            /**
             * @brief Start offering a Service.
             */
            virtual void offerService(ServiceId serviceId, const InstanceIdentifier &instanceIdentifier) = 0;

            /**
             * @brief Stop offering a Service.
             */
            virtual void stopOfferService(ServiceId serviceId, const InstanceIdentifier &instanceIdentifier) = 0;

            /**
             * @brief Request the Runtime to get available Service Instances.
             */
            virtual Result<ServiceHandleContainer> findService(ServiceId serviceId, const InstanceIdentifier &instanceIdentifier) = 0;

            /**
             * @brief Request the Runtime to get available Service Instances.
             */
            virtual Result<ServiceHandleContainer> findService(ServiceId serviceId,
                                                               const ara::core::InstanceSpecifier &instanceSpecifier) = 0;

            /**
             * @brief Request the Runtime to get available Service Instances and availability updates.
             */
            virtual Result<FindServiceHandle> startFindService(const FindServiceHandler &findServiceHandler,
                                                               ServiceId serviceId,
                                                               const InstanceIdentifier &instanceIdentifier) = 0;

            /**
             * @brief Request the Runtime to get available Service Instances and availability updates.
             */
            virtual Result<FindServiceHandle> startFindService(const FindServiceHandler &findServiceHandler,
                                                               ServiceId serviceId,
                                                               const ara::core::InstanceSpecifier &instanceSpecifier) = 0;

            /**
             * @brief Stop availability updates for the given findServiceHandle.
             */
            virtual void stopFindService(FindServiceHandle findServiceHandle) = 0;
        };

    } // namespace sd
} // namespace com

#endif // _COM_SD_RUNTIME_H