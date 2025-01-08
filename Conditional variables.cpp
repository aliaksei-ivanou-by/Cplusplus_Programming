#include <thread>
#include <mutex>
#include <condition_variable>

int items = 10;
std::mutex mutex;
std::condition_variable cond;

void person(int id) {
    int put_back = 0;
    while (items > 0) {
        std::unique_lock<std::mutex> lock(mutex); // pick up the mutex
        while ((id != items % 5) && (items > 0)) { // is it your turn to take item?
            put_back++; // it's not your turn; put back...
            cond.wait(lock); // ...and wait...
        }
        if (items > 0) {
            items--; // it's your turn; take some items!
            lock.unlock(); // put back
            cond.notify_all(); // notify another thread to take their turn
        }
    }
    printf("Person %d put back %u times.\n", id, put_back);
}

int main() {
    std::thread threads[5];
    for (int i = 0; i < 5; i++) {
        threads[i] = std::thread(person, i);
    }
    for (auto& ht : threads) {
        ht.join();
    }
}