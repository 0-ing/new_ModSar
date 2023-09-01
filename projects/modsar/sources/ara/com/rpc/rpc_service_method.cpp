#include "include/ara/com/rpc/rpc_service_method.hpp"
namespace ara
{
    namespace com
    {
        namespace rpc
        {
            void RpcServiceMethod::RunHandler(const Method &param)
            {
                // move to cpp file to implemate this
                ResponseType result;
                std::shared_ptr<vsomeip::message> resp = Getinstance()->create_message(param);
                std::shared_ptr<vsomeip::payload> pl = param->get_payload();
                Status status = CatchingFunctionHandler([this, param, &result]
                                                        { return func_(static_cast<RequestType *>(pl->get_data()), &result); });

                RunHandlerHelper<vsomeip::message, ResponseType>(resp, static_cast<ResponseType *>(&result), status);
                host_->SendResponse(resp);
            }

        } // namespace rpc

    } // namespace com

} // namespace ara
