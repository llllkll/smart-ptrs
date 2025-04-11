#pragma once

#include <exception>

class BadWeakPtr : public std::exception {};

template <typename T>
class SharedPtr;

template <typename T>
class WeakPtr;

struct ControlBlock {
    int strong = 1;
    int weak = 0;

    virtual void StrongDeleter() = 0;

    virtual ~ControlBlock() = default;

    void IncrementStrong() {
        ++strong;
    }

    void IncrementWeak() {
        ++weak;
    }

    void DecrementStrong() {
        --strong;
        if (strong == 0 && weak == 0) {
            StrongDeleter();
            delete this;
        } else if (strong == 0) {
            StrongDeleter();
        }
    }

    void DecrementWeak() {
        --weak;
        if (strong == 0 && weak == 0) {
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

    void StrongDeleter() override {
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

    void StrongDeleter() override {
        reinterpret_cast<T*>(&holder)->~T();
    }

    T* Get() {
        return reinterpret_cast<T*>(&holder);
    }
};
