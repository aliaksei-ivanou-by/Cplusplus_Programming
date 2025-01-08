#include <thread>
#include <chrono>
#include <mutex>
#include <condition_variable>

class Semaphore
{
public:
    Semaphore(unsigned long init_count = 0) {
        count_ = init_count;
    }

    void acquire() { // decrement the internal counter
        std::unique_lock<std::mutex> lck(m_);
        while (!count_) {
            cv_.wait(lck);
        }
        count_--;
    }

    void release() { // increment the internal counter
        std::unique_lock<std::mutex> lck(m_);
        count_++;
        lck.unlock();
        cv_.notify_one();
    }
private:
    std::mutex m_;
    std::condition_variable cv_;
    unsigned long count_;
};

Semaphore sem(4);

void worker(int id) {
    sem.acquire();
    printf("Thread %d\n", id);
    srand(id); // sem for "random" amount between 1-3 seconds
    std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 2000 + 1000));
    printf("Thread %d done\n", id);
    sem.release();
}

int main()
{
    std::thread threads[10];
    for (int i = 0; i < 10; i++) {
        threads[i] = std::thread(worker, i);
    }
    for (auto& p : threads)
    {
        p.join();
    }
}