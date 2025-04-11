#pragma once

#include <cstddef>  // for std::nullptr_t
#include <utility>  // for std::exchange / std::swap

class SimpleCounter {
public:
    SimpleCounter() = default;

    void IncRef() {
        ++count_;
    };
    void DecRef() {
        --count_;
    };
    size_t RefCount() const {
        return count_;
    };

private:
    size_t count_ = 0;
};

struct DefaultDelete {

    DefaultDelete() = default;

    template <typename T>
    static void Destroy(T* object) {
        delete object;
    }
};

template <typename Derived, typename Counter, typename Deleter>
class RefCounted {
public:
    RefCounted() : counter_(Counter()){};
    RefCounted(const RefCounted& other) : counter_(Counter()){};
    RefCounted& operator=(const RefCounted& other) {
        return *this;
    };
    ~RefCounted() = default;
    // Increase reference counter.
    void IncRef() {
        counter_.IncRef();
    };

    // Decrease reference counter.
    // Destroy object using Deleter when the last instance dies.
    void DecRef() {
        if (counter_.RefCount() > 0) {
            counter_.DecRef();
        }
        if (counter_.RefCount() == 0) {
            Deleter{}.Destroy(static_cast<Derived*>(this));
        }
    };

    // Get current counter value (the number of strong references).
    size_t RefCount() const {
        return counter_.RefCount();
    };

private:
    Counter counter_;
};

template <typename Derived, typename D = DefaultDelete>
using SimpleRefCounted = RefCounted<Derived, SimpleCounter, D>;

template <typename T>
class IntrusivePtr {
    template <typename Y>
    friend class IntrusivePtr;

public:
    // Constructors
    IntrusivePtr() = default;
    IntrusivePtr(std::nullptr_t) : ptr_(nullptr){};
    IntrusivePtr(T* ptr) : ptr_(ptr) {
        if (ptr_ != nullptr) {
            ptr_->IncRef();
        }
    };

    template <typename Y>
    IntrusivePtr(Y* ptr) : ptr_(ptr) {
        if (ptr_ != nullptr) {
            ptr_->IncRef();
        }
    };

    template <typename Y>
    IntrusivePtr(const IntrusivePtr<Y>& other) : ptr_(other.ptr_) {
        if (ptr_ != nullptr) {
            ptr_->IncRef();
        }
    };

    template <typename Y>
    IntrusivePtr(IntrusivePtr<Y>&& other) : ptr_(other.ptr_) {
        other.ptr_ = nullptr;
    };

    IntrusivePtr(const IntrusivePtr& other) : ptr_(other.ptr_) {
        if (ptr_ != nullptr) {
            ptr_->IncRef();
        }
    };
    IntrusivePtr(IntrusivePtr&& other) : ptr_(other.ptr_) {
        other.ptr_ = nullptr;
    };

    // `operator=`-s
    IntrusivePtr& operator=(const IntrusivePtr& other) {
        if (this->Get() == (&other)->Get()) {
            return *this;
        }
        if (ptr_ != nullptr) {
            ptr_->DecRef();
        }
        ptr_ = other.ptr_;
        if (ptr_ != nullptr) {
            ptr_->IncRef();
        }
        return *this;
    };
    IntrusivePtr& operator=(IntrusivePtr&& other) {
        if (this->Get() == (&other)->Get()) {
            return *this;
        }
        if (ptr_ != nullptr) {
            ptr_->DecRef();
        }
        ptr_ = other.ptr_;
        other.ptr_ = nullptr;
        return *this;
    };

    // Destructor
    ~IntrusivePtr() {
        if (ptr_ != nullptr) {
            ptr_->DecRef();
        }
    };

    // Modifiers
    void Reset() {
        if (ptr_ != nullptr) {
            ptr_->DecRef();
        }
        ptr_ = nullptr;
    };
    template <typename Y>
    void Reset(Y* ptr) {
        if (ptr_ != nullptr) {
            ptr_->DecRef();
        }
        ptr_ = ptr;
        if (ptr_ != nullptr) {
            ptr_->IncRef();
        }
    };
    void Swap(IntrusivePtr& other) {
        std::swap(ptr_, other.ptr_);
    };

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
        if (ptr_ == nullptr) {
            return 0;
        }
        return ptr_->RefCount();
    };
    explicit operator bool() const {
        return ptr_ != nullptr;
    };

private:
    T* ptr_ = nullptr;
};

template <typename T, typename... Args>
IntrusivePtr<T> MakeIntrusive(Args&&... args) {
    T* ptr = new T(std::forward<Args>(args)...);
    return IntrusivePtr(ptr);
};
