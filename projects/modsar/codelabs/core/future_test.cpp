#include <iostream>
#include <future>
#include <exception>
#include <thread>
#include <unistd.h>

void setval(std::promise<int>& p ,int a) {
  try {
    if (a == 0) {
        throw std::runtime_error("set value 0 not perimted");
    } else {
        sleep(5);
        p.set_value(a);
    }
  } catch(...) {
    p.set_exception(std::current_exception());
  } 
}
 
int CallFuture1() {
  std::promise<int> prom;
  auto fut = prom.get_future();
  std::thread t1(setval, std::ref(prom), 2);
  try {
    std::cout << "fut get value: " << fut.get() << "\n";
  } catch(std::exception& ptr) {
    std::cout << ptr.what() << "\n";
  }
  t1.join();
  return 0;
}
int CallFuture2() {
    auto && task = std::packaged_task<int(int)>([](int a){
        sleep(2);
        return (42+a);
    });
    auto &&future = task.get_future();
    auto t1 = std::thread(std::move(task),2);
    std::cout << future.get() << std::endl;
    t1.join();
    return 0;
}


int CallFuture2_1() {
    auto sleep = []() {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        return 1;
    };
    std::packaged_task<int()> task(sleep);
	auto f = task.get_future();
	task(); 
	std::cout << "You can see this after 1 second"<<std::endl;
	std::cout << f.get() << std::endl;
    return 0;
}
int CallFuture3() {
    auto future = std::async([]{
        sleep(2);
        return 42;
    });
    std::cout << future.get() << std::endl;
    return 0;
}

int thread_task(std::promise<int>& pro, int i)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    std::cout << "thread_task" << i << std::endl;
    pro.set_value(i);
    return 0;
}
 
int CallFuture4() {
  std::promise<int> pro;
    std::thread mythread(thread_task, std::ref(pro), 5);
    mythread.join();
    std::future<int> fut = pro.get_future();
    std::future_status status =  fut.wait_for(std::chrono::milliseconds(1000));
    if (std::future_status::ready == status){     //线程已成功返回
       
        std::cout<< "ready"<<std::endl;
    }
    else if (std::future_status::timeout == status){ //wait_for的时间已结束，线程仍未成功返回
        std::cout<< "timeout"<<std::endl;
    }
    else if (std::future_status::deferred == status){ //如果std::async的第一个参数设置为std::launch::deferred,则该线程会直到std::future对象调用wait()或get()时才会执行,这种情况就会满足
        std::cout<< "deferred"<<std::endl; 
    }
    std::cout << fut.get() << std::endl;
    return 0;
}

using namespace std::chrono; 
std::string getData(std::string recvData) {
  
    std::cout << "getData start" << std::this_thread::get_id() << std::endl;
    std::this_thread::sleep_for(seconds(5));
    return "DB_" + recvData;
}
int CallFuture4_1() {
    system_clock::time_point start = system_clock::now();
    std::future<std::string> fut = std::async(std::launch::async, getData, "Data");
  
    std::future_status status;
    std::string dbData;
    do
    {
        status = fut.wait_for(std::chrono::seconds(1));
        switch (status){
        case std::future_status::ready:
            std::cout << "Ready..." << std::endl;
            dbData = fut.get();
            std::cout << dbData << std::endl;
            break;
        case std::future_status::timeout:
            std::cout << "timeout..." << std::endl;
            break;
        case std::future_status::deferred:
            std::cout << "deferred..." << std::endl;
            break;
        default:
            break;
        }
    } while (status != std::future_status::ready);
  
    auto end = system_clock::now();
    auto diff = duration_cast<std::chrono::seconds>(end - start).count();
    std::cout << "Total Time taken= " << diff << "Seconds" << std::endl;
    return 0;
}
int CallFuture5()
{
    auto promise = std::promise<int>();
    auto t1 = std::thread([&promise]{
        sleep(2);
        promise.set_value(42);
    });

    auto shared_future = std::shared_future<int>(promise.get_future());
    auto t2 = std::thread([shared_future]{
        std::printf("thread 2: %d\n", shared_future.get());
    });
    std::printf("main: %d\n", shared_future.get());

    t1.join();
    t2.join();
    return 0;
}

int CallFuture5_1 ()
{
    std::future<int> fut = std::async([](){return 10;});
    std::shared_future<int> shared_fut = fut.share();

    // 共享的 future 对象可以被多次访问.
    std::cout << "value: " << shared_fut.get() << '\n';
    std::cout << "its double: " << shared_fut.get()*2 << '\n';

    return 0;
}



int main(){
    CallFuture2();
    return 0;
}