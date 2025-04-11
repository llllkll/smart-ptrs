#pragma once

#include "sw_fwd.h"  // Forward declaration

#include <cstddef>  // std::nullptr_t

// https://en.cppreference.com/w/cpp/memory/shared_ptr
template <typename T>
class SharedPtr {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    SharedPtr() = default;
    SharedPtr(std::nullptr_t) : ptr_(nullptr), block_(nullptr){};

    explicit SharedPtr(T* ptr) : ptr_(ptr), block_(new ControlBlockPtr<T>(ptr)){};

    template <typename Y>
    explicit SharedPtr(Y* ptr) : ptr_(ptr), block_(new ControlBlockPtr<Y>(ptr)){};

    SharedPtr(const SharedPtr& other) : ptr_(other.ptr_), block_(other.block_) {
        if (block_ != nullptr) {
            block_->IncrementStrong();
        }
    };

    SharedPtr(SharedPtr&& other) : ptr_(other.ptr_), block_(other.block_) {
        other.ptr_ = nullptr;
        other.block_ = nullptr;
    };

    template <typename Y>
    SharedPtr(const SharedPtr<Y>& other) : ptr_(other.ptr_), block_(other.block_) {
        if (block_ != nullptr) {
            block_->IncrementStrong();
        }
    };

    template <typename Y>
    SharedPtr(SharedPtr<Y>&& other) : ptr_(other.ptr_), block_(other.block_) {
        other.ptr_ = nullptr;
        other.block_ = nullptr;
    };

    SharedPtr(ControlBlockObj<T>* block) : block_(block) {
        ptr_ = block->Get();
    }

    SharedPtr(T* ptr, ControlBlock* block) : ptr_(ptr), block_(block) {
    }

    // Aliasing constructor
    // #8 from https://en.cppreference.com/w/cpp/memory/shared_ptr/shared_ptr
    template <typename Y>
    SharedPtr(const SharedPtr<Y>& other, T* ptr) : ptr_(ptr), block_(other.block_) {
        if (block_ != nullptr) {
            block_->IncrementStrong();
        }
    };

    // Promote `WeakPtr`
    // #11 from https://en.cppreference.com/w/cpp/memory/shared_ptr/shared_ptr
    explicit SharedPtr(const WeakPtr<T>& other) {
        if (other.Expired()) {
            throw BadWeakPtr();
        }
        *this = other.Lock();
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    SharedPtr& operator=(const SharedPtr& other) {
        if (this->Get() == (&other)->Get()) {
            return *this;
        }
        if (block_ != nullptr) {
            block_->DecrementStrong();
        }
        ptr_ = other.ptr_;
        block_ = other.block_;
        if (block_ != nullptr) {
            block_->IncrementStrong();
        }
        return *this;
    };

    template <typename Y>
    SharedPtr& operator=(const SharedPtr<Y>& other) {
        if (this->Get() == (&other)->Get()) {
            return *this;
        }
        if (block_ != nullptr) {
            block_->DecrementStrong();
        }
        ptr_ = other.ptr_;
        block_ = other.block_;
        if (block_ != nullptr) {
            block_->IncrementStrong();
        }
        return *this;
    };

    SharedPtr& operator=(SharedPtr&& other) {
        if (this->Get() == (&other)->Get()) {
            return *this;
        }
        if (block_ != nullptr) {
            block_->DecrementStrong();
        }
        ptr_ = std::move(other.ptr_);
        block_ = std::move(other.block_);
        other.ptr_ = nullptr;
        other.block_ = nullptr;
        return *this;
    };

    template <typename Y>
    SharedPtr& operator=(SharedPtr<Y>&& other) {
        if (this->Get() == (&other)->Get()) {
            return *this;
        }
        if (block_ != nullptr) {
            block_->DecrementStrong();
        }
        ptr_ = other.ptr_;
        block_ = other.block_;
        other.ptr_ = nullptr;
        other.block_ = nullptr;
        return *this;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~SharedPtr() {
        if (block_ != nullptr) {
            block_->DecrementStrong();
        }
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    void Reset() {
        if (block_ != nullptr) {
            block_->DecrementStrong();
        }
        ptr_ = nullptr;
        block_ = nullptr;
    };

    template <typename Y>
    void Reset(Y* ptr) {
        if (block_ != nullptr) {
            block_->DecrementStrong();
        }
        ptr_ = ptr;
        block_ = new ControlBlockPtr<Y>(ptr);
    };
    void Swap(SharedPtr& other) {
        std::swap(ptr_, other.ptr_);
        std::swap(block_, other.block_);
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T* Get() const {
        return ptr_;
    };
    T& operator*() const {
        return *ptr_;
    };
    T* operator->() const {
        return ptr_;
    };
    size_t UseCount() const {
        if (block_ == nullptr) {
            return 0;
        }
        return block_->strong;
    };
    explicit operator bool() const {
        return ptr_ != nullptr;
    };

private:
    T* ptr_ = nullptr;
    ControlBlock* block_ = nullptr;

    template <typename Y>
    friend class SharedPtr;

    template <typename Y>
    friend class WeakPtr;
};

template <typename T, typename U>
inline bool operator==(const SharedPtr<T>& left, const SharedPtr<U>& right) {
    return left.Get() == right.Get();
};

// Allocate memory only once
template <typename T, typename... Args>
SharedPtr<T> MakeShared(Args&&... args) {
    auto block = new ControlBlockObj<T>(std::forward<Args>(args)...);
    return SharedPtr<T>(block);
};

// Look for usage examples in tests
template <typename T>
class EnableSharedFromThis {
public:
    SharedPtr<T> SharedFromThis();
    SharedPtr<const T> SharedFromThis() const;

    WeakPtr<T> WeakFromThis() noexcept;
    WeakPtr<const T> WeakFromThis() const noexcept;
};
