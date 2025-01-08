#include <thread>
#include <mutex>
#include <barrier>
#include <cstdio>

unsigned int bags_of_chips = 1; // start with one on the list
std::mutex pencil;
std::barrier<> fist_bump(10); // 10 threads should synchronize at the barrier

void cpu_work(unsigned long workUnits) {
    unsigned long x = 0;
    for (unsigned long i = 0; i < workUnits * 1000000; i++) { // Initialized 'i'
        x++;
    }
}

void barron_shopper() {
    cpu_work(1); // do a bit of work first
    fist_bump.arrive_and_wait(); // Use 'arrive_and_wait' to synchronize
    std::scoped_lock<std::mutex> lock(pencil); // Lock to safely modify 'bags_of_chips'
    bags_of_chips *= 2; // Double the bags of chips
    printf("Barron DOUBLED the bags of chips.\n");
}

void olivia_shopper() {
    cpu_work(1); // do a bit of work first
    {
        std::scoped_lock<std::mutex> lock(pencil); // Lock to safely modify 'bags_of_chips'
        bags_of_chips += 3; // Add 3 bags of chips
    }
    printf("Olivia ADDED 3 bags of chips.\n");
    fist_bump.arrive_and_wait(); // Use 'arrive_and_wait' to synchronize
}

int main() {
    std::thread shoppers[10]; // Array of 10 threads (5 pairs of shoppers)
    for (int i = 0; i < 10; i += 2) {
        shoppers[i] = std::thread(barron_shopper); // Barron shopper thread
        shoppers[i + 1] = std::thread(olivia_shopper); // Olivia shopper thread
    }
    for (auto& s : shoppers) {
        s.join(); // Wait for all threads to finish
    }
    printf("We need to buy %u bags of chips.\n", bags_of_chips); // Print the total number of bags
}
