
void ICall:: invoke()
{

    std::promise<Status> promise;
    auto futher = promise.get_future(status_);
    request_ = runtime::CreateMessage(method);
    request_->set_payload(request);

    auto handler_ = [](std::shared_ptr<BaseMessage> &recv_msg)
    {
        /**
         * payload 里一般是 二进制流
         * OutputMessage 一般是个有序列化能力的类 这里不能强转
         */
        OutputMessage message;
        auto payload = recv_msg->get_payload();
        int result = message.decode(payload->get_data(), payload->get_length());
        promise.set_value(result);
    } host_->RegisterMessageHandler(method->serviceId(), method->instanceId(), method->methodId(), handler_);
    host_->SendRequest(request_);
    auto status = futher.wait(status_);
    // 解释 为啥不能这里做状态反馈
    // 构造函数这里wait 会阻塞等待，关于wait 本身的返回值和状态 上层没法感知
    // 所以仅用status_ 无法去让上层感知这里的初始化行为 构造函数里没有返回值 所以需要在函数里做这件事
    core::Result<void> ret;
    ret.set_status(status) return ret;
}