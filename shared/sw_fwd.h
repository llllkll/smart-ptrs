#pragma once

#include <exception>

class BadWeakPtr : public std::exception {};

template <typename T>
class SharedPtr;

template <typename T>
class WeakPtr;

struct ControlBlock {
    int counter = 1;

    virtual void Deleter() = 0;

    virtual ~ControlBlock() = default;

    void Increment() {
        ++counter;
    }

    void Decrement() {
        --counter;
        if (counter == 0) {
            Deleter();
            delete this;
        }
    }
};

template <typename T>
struct ControlBlockPtr : ControlBlock {
    T* ptr;

    ControlBlockPtr(T* pointer) : ptr(pointer) {
    }

    ~ControlBlockPtr() override = default;

    void Deleter() override {
        delete ptr;
    }
};

template <typename T>
struct ControlBlockObj : ControlBlock {
    alignas(T) char holder[sizeof(T)];

    template <typename... Args>
    ControlBlockObj(Args&&... args) {
        new (&holder) T(std::forward<Args>(args)...);
    }

    ~ControlBlockObj() override = default;

    void Deleter() override {
        reinterpret_cast<T*>(&holder)->~T();
    }

    T* Get() {
        return reinterpret_cast<T*>(&holder);
    }
};