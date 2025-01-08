#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <cstdio>

class Line {
public:
    void serve(int i)
    {
        std::unique_lock<std::mutex> locker(mutex);
        queue.push(i);
        cond.notify_one();
    }

    int take()
    {
        std::unique_lock<std::mutex> locker(mutex);
        while (queue.empty())
        {
            cond.wait(locker);
        }
        int val = queue.front();
        queue.pop();
        return val;
    }
    friend void producer();
    friend void consumer();
private:
    std::queue<int> queue;
    std::mutex mutex;
    std::condition_variable cond;
};

Line line;

void producer()
{
    for (int i = 0; i < 10000; i++) // serve a 10,000 items
    {
        line.serve(i);  // Corrected to call line.serve
    }
    line.serve(-1); // indicate no more work
    printf("Producer done.\n");
}

void consumer()
{
    int temp = 0; // Initialize temp to 0
    while (true)
    {
        int val = line.take();
        if (val == -1) // Corrected to comparison
        {
            printf("Consumer is working.\n");
            line.serve(-1); // put back last item for the other consumers to take
            return;
        }
        else {
            temp += val;
        }
    }
}

int main()
{
    std::thread thread_first(producer);
    std::thread thread_second(consumer);
    std::thread thread_third(consumer);

    thread_first.join();
    thread_second.join();
    thread_third.join();
}