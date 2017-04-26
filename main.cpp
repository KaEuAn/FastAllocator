#include <iostream>
#include <algorithm>
#include <ctime>
#include <list>
#include <memory>
#include <tuple>

template <typename T>
class Stack {
    static const uint32_t MIN_SIZE = 128;
    static const uint16_t EXP_CONST = 2;
    T* container;
    uint32_t real_size;
    uint32_t _size;

    void resize(uint32_t new_size) {
        T *new_container = new T[new_size];
        uint32_t j = 0;
        for (uint32_t i = 0; i < real_size; ++i, ++j) {
            new_container[j] = container[i];
        }
        T *per = container;
        real_size = new_size;
        container = new_container;
        delete[] per;
    }

    void tryExtend() {
        if (_size == real_size - 1) {
            resize(real_size * EXP_CONST);
        }
    }
    void tryMinimize() {
        if (_size * EXP_CONST * EXP_CONST < real_size && real_size < MIN_SIZE) {
            resize(real_size / EXP_CONST);
        }
    }

public:
    explicit Stack() : real_size(MIN_SIZE), _size(0) {
            container = new T[real_size];
    };
    Stack(Stack&& other) : container(std::move(other.container)), real_size(std::move(other.real_size)),
                           _size(std::move(other._size)) {
        other.container = nullptr;
        other.real_size = 0;
        other._size = 0;
    }
    ~Stack() {
        delete[] container;
    }
    Stack(const Stack& a) {
        real_size = a.real_size;
        container = new T[real_size];
        for (int i = 0; i < a._size; ++i) {
            container[i] = a.container[i];
        }
    }

    void push(T a) {
        tryExtend();
        container[_size] = a;
        ++_size;
    }
    void pop() {
        tryMinimize();
        --_size;
    }
    uint32_t getSize() const {
        return static_cast<const uint32_t>(_size);
    }
    const T top() const {
        return container[_size - 1];
    }
    const uint32_t size() const {
        return _size;
    }
};

template <size_t chunkSize>
class FixedAllocator {
    static const uint32_t START_SIZE = 64;
    static const uint32_t EXP = 2;
    typedef uint8_t byte;
    Stack<byte*> memory;
    Stack<byte*> chunks;
    inline void make_chunk(byte* mem, uint32_t mem_size) {
        for (int i = 0; i < mem_size; ++i) {
            chunks.push(mem + i * chunkSize);
        }
    }
    inline void addMemory() {
        memory.push(static_cast<byte*>(operator new(START_SIZE * chunkSize)));
        make_chunk(memory.top(), START_SIZE);
    }
    inline bool isEmpty() {
        return chunks.size() == 0;
    }
    inline void tryExtend() {
        if (isEmpty()) {
            addMemory();
        }
    }

public:
    inline explicit FixedAllocator() {
        memory.push(static_cast<byte*>(::operator new(START_SIZE * chunkSize)));
        make_chunk(memory.top(), START_SIZE);
    };
    inline FixedAllocator(FixedAllocator&& other) : memory(std::move(other.memory)), chunks(std::move(other.chunks)) {}
    inline ~FixedAllocator() {
        for (int i = 0; i < memory.getSize(); ++i) {
            void* del_memory = memory.top();
            operator delete(del_memory);
            memory.pop();
        }
    };

    inline void* allocate() {
        tryExtend();
        void* answer = chunks.top();
        chunks.pop();
        return answer;
    }
    inline void deallocate(void* p) {
        chunks.push(static_cast<byte*>(p));
    }

};


template <typename T>
class FastAllocator{
    FixedAllocator<24> myAlloc24;
    FixedAllocator<16> myAlloc16;
    FixedAllocator<12> myAlloc12;
    FixedAllocator<8> myAlloc8;
    FixedAllocator<4> myAlloc4;

public:

    typedef T value_type;
    typedef T* pointer;
    typedef const T* const_pointer;
    typedef T& reference;
    typedef const T& const_reference;
    typedef std::size_t size_type;
    typedef std::ptrdiff_t difference_type;

    template <typename _T>
    struct rebind {
        typedef FastAllocator<_T> other;
    };
    inline explicit FastAllocator() = default;
    ~FastAllocator() = default;
    inline FastAllocator(FastAllocator&& other) : myAlloc4(std::move(other.myAlloc4)), myAlloc8(std::move(other.myAlloc8)),
                                                  myAlloc12(std::move(other.myAlloc12)), myAlloc16(std::move(other.myAlloc16)),
                                                  myAlloc24(std::move(other.myAlloc24)) {
        other.myAlloc4 = FixedAllocator<4>();
        other.myAlloc8 = FixedAllocator<8>();
        other.myAlloc12 = FixedAllocator<12>();
        other.myAlloc16 = FixedAllocator<16>();
        other.myAlloc24 = FixedAllocator<24>();
    }

    inline pointer allocate(uint32_t size) {
        uint32_t sizeToAlloc = size * sizeof(T);
        switch (sizeToAlloc) {
            case 24: {
                return static_cast<pointer>(myAlloc24.allocate());
                break;
            }
            case 16: {
                return static_cast<pointer>(myAlloc16.allocate());
                break;
            }
            case 12: {
                return static_cast<pointer>(myAlloc12.allocate());
                break;
            }
            case 8: {
                return static_cast<pointer>(myAlloc8.allocate());
                break;
            }
            case 4: {
                return static_cast<pointer>(myAlloc4.allocate());
                break;
            }
            default:
                return static_cast<pointer>(::operator new(size * sizeof(T)));
        }
    }
    inline void deallocate(pointer p, uint32_t size) {
        uint32_t sizeToAlloc = size * sizeof(T);
        switch (sizeToAlloc) {
            case 24: {
                myAlloc24.deallocate(static_cast<void *>(p));
                break;
            }
            case 16: {
                myAlloc16.deallocate(static_cast<void *>(p));
                break;
            }
            case 12: {
                myAlloc12.deallocate(static_cast<void *>(p));
                break;
            }
            case 8: {
                myAlloc8.deallocate(static_cast<void *>(p));
                break;
            }
            case 4: {
                myAlloc4.deallocate(static_cast<void *>(p));
                break;
            }
            default:
                ::operator delete(p);
        }
    }
    inline void construct(pointer p, const T& t) {
        new(p) T(t);
    }
    inline void destroy(pointer p) {
        p->~T();
    }
};

template<typename T, typename Allocator = std::allocator<T> >
class List{

    struct Node {
        T value;
        Node* prev;
        Node* next;
        Node(const T& newValue = T(), Node* first = nullptr, Node* second = nullptr) :
                value(newValue), prev(first), next(second) {}
    };

    uint32_t _size;
    Node* startNode;
    Node* endNode;
    typedef typename Allocator::template rebind<Node>::other typeNodeAlloc;
    typeNodeAlloc nodeAlloc;
    
    Node* getNode() {
        return nodeAlloc.allocate(1);
    }

    void forwardErase(Node* eraseNode) {
        nodeAlloc.destroy(eraseNode);
        nodeAlloc.deallocate(eraseNode, 1);
    }

public:
    explicit List(const Allocator& alloc = typeNodeAlloc()) : _size(0), startNode(nullptr), endNode(nullptr), nodeAlloc(alloc) {}
    List(uint32_t count, const T& value = T(), const Allocator& alloc = typeNodeAlloc()) : _size(count), nodeAlloc(alloc){
        startNode = nodeAlloc.allocate(_size);
        std::uninitialized_fill_n(startNode, _size, value);
        endNode = startNode + (_size - 1);
    }
    List(const List& other) : _size(other._size), startNode(nullptr), endNode(nullptr), nodeAlloc(typeNodeAlloc()) {
        if (_size == 0)
            return;
        startNode = nodeAlloc.allocate(1);
        Node* otherCurrentNode = other.startNode;
        nodeAlloc.construct(startNode, otherCurrentNode->value);
        startNode->prev = nullptr;
        Node* currentNode = startNode;
        otherCurrentNode = otherCurrentNode->next;
        while(otherCurrentNode) {
            Node* newNode = nodeAlloc.allocate(1);
            nodeAlloc.construct(newNode, otherCurrentNode->value, otherCurrentNode->prev, otherCurrentNode->next);
            currentNode->next = newNode;
            newNode->prev = currentNode;
            currentNode = newNode;
            otherCurrentNode = otherCurrentNode->next;
        }
        endNode = currentNode;
        endNode->next = nullptr;
    }
    List(List&& other) : _size(std::move(other._size)),
                         startNode(other.startNode), endNode(other.endNode), nodeAlloc(std::move(other.nodeAlloc)) {
        other._size = 0;
        other.startNode = nullptr;
        other.endNode = nullptr;
    }
    template<typename _T>
    List& operator= (_T&& other) {
        this->~List();
        this->List(std::forward(other));
        return *this;
    }
    ~List() {
        if (_size == 0)
            return;
        for (int i = 0; i < _size; ++i) {
            forwardErase(startNode + i);
        }
        nodeAlloc.deallocate(startNode, _size);
    }

    uint32_t size() const {
        return _size;
    }
    template<typename _T>
    void insert_before(Node* insertNode, _T&& newValue) {
        Node* prevNode = insertNode->prev;
        Node* newNode = getNode();
        nodeAlloc.construct(newNode, std::forward(newValue), prevNode, insertNode);
        insertNode->prev = newNode;
        if (prevNode) {
            prevNode->next = newNode;
        }
        ++_size;
    }
    template<typename _T>
    void insert_after(Node* insertNode, _T&& newValue) {
        Node* nextNode = insertNode->next;
        Node* newNode = getNode();
        nodeAlloc.construct(newNode, std::forward(newValue), insertNode, nextNode);
        insertNode->next = newNode;
        if (nextNode) {
            nextNode->prev = newNode;
        }
        ++_size;
    }
    void erase(Node* eraseNode) {
        Node* nextNode = eraseNode->next;
        Node* prevNode = eraseNode->prev;
        if (nextNode)
            nextNode->prev = prevNode;
        if (prevNode)
            prevNode->next = nextNode;
        forwardErase(eraseNode);
        --_size;
    }

    template<typename _T>
    void push_back(_T&& newValue) {
        if (endNode) {
            insert_after(endNode, std::forward(newValue));
        } else {
            Node* newNode = getNode();
            nodeAlloc.construct(newNode, std::forward(newValue), nullptr, nullptr);
            startNode = newNode;
            endNode = newNode;
        }

    }
    template<typename _T>
    void push_front(_T&& newValue) {
        if (startNode) {
            insert_before(startNode, std::forward(newValue));
        } else {
            Node* newNode = getNode();
            nodeAlloc.construct(newNode, std::forward(newValue), nullptr, nullptr);
            startNode = newNode;
            endNode = newNode;
        }
    }
    void pop_back() {
        Node* prevNode = endNode->prev;
        if (endNode == startNode)
            startNode = nullptr;
        erase(endNode);
        endNode = prevNode;
    }
    void pop_front() {
        Node* nextNode = startNode->next;
        if (endNode == startNode)
            endNode = nullptr;
        erase(startNode);
        startNode = nextNode;
    }
};

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

void testAll() {
    int testAmount = 1000000;
    std::cout << "std::list, std::allocator  time: ";
    test<std::list<int>>(testAmount);
    std::cout << "std::list, myAllocator  time: ";
    test<std::list<int, FastAllocator<int>>>(testAmount);
}

int main() {
    testAll();
}
