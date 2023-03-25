#include <chrono>
#include <thread>
#include <future>
#include <numeric>
#include <vector>
#include <iostream>

using namespace std;

namespace std_async_demo {
    using vec_itr_t = vector<int>::iterator;

    /**
     * 通过promise和future封装好了线程同步和数据的线程安全，省心了很多这方面的工作。
     */
    void promise_future() {
        std::vector nums = { 1, 2, 3, 4, 5 };

        auto f = [](auto first, auto last, auto p) { // (1) define async function
            p.set_value(std::accumulate(first, last, 0)); // (2) set return value to PROMISE
        };
        // promise_future() 展示的是C++异步基本机制：引入promise用于线程函数里存结果(set)；
        std::promise<int> p;
        //  引入跟promise关联的future用于在调用线程取结果(get)；引入thread执行线程函数(run)；
        std::future<int> r = p.get_future();
        // 主线程启动线程，并等待通过future获取结果(call & wait)。
        // 具有异步功能的新线程
        std::thread t(f, nums.begin(), nums.end(), std::move(p));
        // 调用线程取结果(get)
        std::cout << "wait & get result = " << r.get() << endl;
        t.join();
    }

    /**
     * 不需要自建promise和thread实例及执行相关的调用，又省心了一些。
     */
    void package_task() {

        std::vector nums = { 1, 2, 3, 4 };

        auto f = [](vec_itr_t first, vec_itr_t last) { // (1) define async function
            return std::accumulate(first, last, 0);
        };

        // 引入task包装了promise和thread，把普通函数或函数式交给task(package)
        // package function call into a task
        std::packaged_task<int(vec_itr_t, vec_itr_t)> task(f);

        // task则执行时自动起线程执行，并自动把结果存好在promise里，在task关联的fucture里直接获取结果即可(call & wait)
        auto r = task.get_future();
        // 开启任务 给任务传参数
        task(nums.begin(), nums.end());
        // 获取结果
        std::cout << "wait & get result = " << r.get() << endl;
    }

    /**
     * 连task都干掉了，定义好普通函数或函数式，直接通过std::async() 异步调用，然后通过所关联的future取结果(call & wait)
     */
    void async_get() {

        /// 到这一步已经是直接call就好了，这已经是省心至极了吧？
        /// 是，但也不是，因为其实里面还隐藏了一个wait动作，即线程的同步与等待，主线程必须等在那里取结果，
        /// 而且，是永久地等，干不了其他；另外，取结果也是手动显式地get一下。

        cout << "async call & wait infinitely..." << endl;

        std::vector nums = { 1, 2, 3 };
        auto f = std::accumulate<vec_itr_t, int>; // (1) define async function
        auto r = std::async(std::launch::async, f, nums.begin(), nums.end(), 0); // (2) async call

        std::cout << "wait infinitely & get result = " << r.get() << endl; // (3)
    }

    /**
     * 让主线程干等着、死等着子线程结束和返回结果在大多生产环境下恐怕是不能接受的，
     * 在还没有Coroutine机制帮忙时，在future里加了个wait_for函数，用来每等一会就看看结果，还没好就干点别的再回来等等看看。
     */
    void async_wait() {

        cout << "async call & wait & check periodically for a certain time --" << endl;

        std::vector nums = { 1, 2 };
        // 定义异步函数
        auto f = [&](vec_itr_t first, vec_itr_t last) {
            std::this_thread::sleep_for(std::chrono::seconds(5));
            return std::accumulate(first, last, 0);
        };
        // 异步调用
        auto r = std::async(std::launch::async, f, nums.begin(), nums.end());

        std::future_status status;
        do
        {
            // 等待并定期检查状态
            status = r.wait_for(std::chrono::seconds(1));
            if (status == std::future_status::timeout || status == std::future_status::deferred) {
                cout << "not ready, do something else ..." << endl;
            }
        } while (status != std::future_status::ready); // 查看等待的状态是否读取到结果
        // 打印结果
        std::cout << "result = " << r.get() << endl;
    }
}

int main() {
    using namespace std_async_demo;
    promise_future();
//    package_task();
//    async_get();
//    async_wait();
    return 0;
}