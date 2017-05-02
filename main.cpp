#include "fastallocator.h"

template <typename listType>
void test(int testAmount) {
    listType list;
    int randMax = testAmount / 10;
    double time = clock();
    while (testAmount > 0) {
        int amount = std::rand() % randMax;
        if (amount > testAmount)
            amount = testAmount;
        testAmount -= amount;
        for (int i = 0; i < amount; ++i) {
            list.push_front(i);
        }
        for (int i = 0; i < amount; ++i) {
            list.pop_front();
        }
    }
    time = clock() - time;
    std::cout << time / CLOCKS_PER_SEC << '\n';
}

template<typename T>
void testAll() {
    int testAmount = 1000000;
    std::cout << "std::list, std::allocator  time: ";
    test<std::list<T>>(testAmount);
    std::cout << "std::list, myAllocator  time: ";
    test<std::list<T, FastAllocator<T>>>(testAmount);
    std::cout << "myList, std::allocator  time: ";
    test<List<T>>(testAmount);
    std::cout << "myList, myAllocator  time: ";
    test<List<T, FastAllocator<T>>>(testAmount);
}
int main() {
    testAll<int>();
    List<int> x;
    x.push_back(7);
    List<int> y;
    y = x;
    y.push_back(8);
    List<int>z(std::move(y));
}
