#pragma once

#include "sw_fwd.h"  // Forward declaration

// https://en.cppreference.com/w/cpp/memory/weak_ptr
template <typename T>
class WeakPtr {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    WeakPtr() = default;

    WeakPtr(const WeakPtr& other) : ptr_(other.ptr_), block_(other.block_) {
        if (block_ != nullptr) {
            block_->IncrementWeak();
        }
    };

    template <typename Y>
    WeakPtr(const WeakPtr<Y>& other) : ptr_(other.ptr_), block_(other.block_) {
        if (block_ != nullptr) {
            block_->IncrementWeak();
        }
    };

    template <typename Y>
    WeakPtr(WeakPtr<Y>&& other) : ptr_(other.ptr_), block_(other.block_) {
        other.ptr_ = nullptr;
        other.block_ = nullptr;
    };

    WeakPtr(WeakPtr&& other) : ptr_(other.ptr_), block_(other.block_) {
        other.ptr_ = nullptr;
        other.block_ = nullptr;
    };

    // Demote `SharedPtr`
    // #2 from https://en.cppreference.com/w/cpp/memory/weak_ptr/weak_ptr
    WeakPtr(const SharedPtr<T>& other) : ptr_(other.ptr_), block_(other.block_) {
        if (block_ != nullptr) {
            block_->IncrementWeak();
        }
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    WeakPtr& operator=(const WeakPtr& other) {
        if (this->ptr_ == (&other)->ptr_) {
            return *this;
        }
        if (block_ != nullptr) {
            block_->DecrementWeak();
        }
        ptr_ = other.ptr_;
        block_ = other.block_;
        if (block_ != nullptr) {
            block_->IncrementWeak();
        }
        return *this;
    };
    WeakPtr& operator=(WeakPtr&& other) {
        if (this->ptr_ == (&other)->ptr_) {
            return *this;
        }
        if (block_ != nullptr) {
            block_->DecrementWeak();
        }
        ptr_ = other.ptr_;
        block_ = other.block_;
        other.ptr_ = nullptr;
        other.block_ = nullptr;
        return *this;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~WeakPtr() {
        if (block_ != nullptr) {
            block_->DecrementWeak();
        }
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    void Reset() {
        if (block_ != nullptr) {
            block_->DecrementWeak();
        }
        ptr_ = nullptr;
        block_ = nullptr;
    };
    void Swap(WeakPtr& other) {
        std::swap(ptr_, other.ptr_);
        std::swap(block_, other.block_);
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    size_t UseCount() const {
        if (block_ == nullptr) {
            return 0;
        }
        return block_->strong;
    };
    bool Expired() const {
        return UseCount() == 0;
    };
    SharedPtr<T> Lock() const {
        if (UseCount() == 0) {
            return SharedPtr<T>();
        }
        block_->IncrementStrong();
        return SharedPtr<T>(ptr_, block_);
    };

private:
    T* ptr_ = nullptr;
    ControlBlock* block_ = nullptr;

    template <typename Y>
    friend class EnableSharedFromThis;

    template <typename Y>
    friend class WeakPtr;
};
