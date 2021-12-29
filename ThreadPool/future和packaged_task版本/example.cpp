

#include <iostream>
#include <functional>
#include <thread>
#include <future>
#include <chrono>
using namespace std;

#include "threadpool.h"


int sum1(int a, int b)
{
    this_thread::sleep_for(chrono::seconds(2));
    return a + b;
}
int sum2(int a, int b, int c)
{
    this_thread::sleep_for(chrono::seconds(2));
    return a + b + c;
}

int main()
{
    ThreadPool pool;
    pool.start(2);

    future<int> result1 = pool.submitTask(sum1, 1, 2);
    future<int> result2 = pool.submitTask(sum2, 1, 2, 3);
    future<int> result3 = pool.submitTask([](int b, int e)->int {
        int sum = 0;
        for (int i = b; i <= e; i++)
            sum += i;
        return sum;
        }, 1, 100);

    cout << result1.get() << endl;
    cout << result2.get() << endl;
    cout << result3.get() << endl;

	getchar();

}