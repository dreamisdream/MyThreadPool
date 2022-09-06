// MyThreaPool.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "ThreadPool.h"
#include <chrono>

using namespace std;


int fun1(int a, int b) {
    cout << "fun1 a+b  " << a + b << endl;
    return a + b;
}

int main()
{
    // 创建线程池 hardware_concurrency
    int cnt = thread::hardware_concurrency();
    MyThreadPool pool(cnt);

    vector<future<int>> results;
    for (int i = 0; i < 8; ++i) {
        pool.enqueue([i] {
            cout << "begin  " << i << endl;
            this_thread::sleep_for(std::chrono::seconds(1));
            cout << "end  " << i << endl;
            return pow(i, 2.0);
            });
    }
    pool.enqueue(fun1, 1, 2);

    for (auto&& result : results) {
        cout << result.get() << endl;
    }
    return 0;
}