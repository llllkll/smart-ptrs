#pragma once

#include "compressed_pair.h"

#include <cstddef>  // std::nullptr_t
#include <type_traits>
#include <algorithm>

template <typename T>
struct DefaultDeleter {
    DefaultDeleter() = default;

    template <typename U>
    DefaultDeleter(const DefaultDeleter<U>&) {
    }
    void operator()(T* ptr) const {
        delete ptr;
    }
};

template <typename T>
struct DefaultDeleter<T[]> {
    DefaultDeleter() = default;

    template <typename U>
    DefaultDeleter(const DefaultDeleter<U[]>&) {
    }
    void operator()(T* ptr) const {
        delete[] ptr;
    }
};

// Primary template
template <typename T, typename Deleter = DefaultDeleter<T>>
class UniquePtr {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    explicit UniquePtr(T* ptr = nullptr) {
        pair_.GetFirst() = ptr;
        pair_.GetSecond() = Deleter{};
    };
    template <typename NewDeleter>
    UniquePtr(T* ptr, NewDeleter deleter) {
        pair_.GetFirst() = ptr;
        pair_.GetSecond() = std::forward<NewDeleter>(deleter);
    };
    UniquePtr(const UniquePtr&) = delete;
    UniquePtr(UniquePtr&& other) noexcept {
        pair_.GetFirst() = std::move(other.pair_.GetFirst());
        pair_.GetSecond() = std::move(other.pair_.GetSecond());
        other.pair_.GetFirst() = nullptr;
    };
    template <typename U, typename DU = DefaultDeleter<U>>
    UniquePtr(UniquePtr<U, DU>&& other) noexcept {
        pair_.GetFirst() = other.Release();
        pair_.GetSecond() = std::move(other.GetDeleter());
        other.Reset(nullptr);
    };
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s
    UniquePtr& operator=(UniquePtr&& other) noexcept {
        Reset(other.Release());
        pair_.GetSecond() = std::forward<Deleter>(other.pair_.GetSecond());
        return *this;
    };
    template <typename U, typename DU = DefaultDeleter<U>>
    UniquePtr& operator=(UniquePtr<U, DU>&& other) noexcept {
        Reset(other.Release());
        pair_.GetSecond() = std::forward<Deleter>(other.GetDeleter());
        return *this;
    };
    UniquePtr& operator=(std::nullptr_t) {
        Reset();
        return *this;
    };
    UniquePtr& operator=(const UniquePtr& other) = delete;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~UniquePtr() {
        auto& ptr = pair_.GetFirst();
        if (ptr != nullptr) {
            pair_.GetSecond()(std::move(pair_.GetFirst()));
            ptr = nullptr;
        }
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    T* Release() {
        T* ptr = pair_.GetFirst();
        pair_.GetFirst() = nullptr;
        return ptr;
    };
    void Reset(T* ptr = nullptr) {
        T* old_ptr = pair_.GetFirst();
        pair_.GetFirst() = ptr;
        if (old_ptr != nullptr) {
            pair_.GetSecond()(old_ptr);
        }
    };
    void Swap(UniquePtr& other) {
        std::swap(this->pair_, other.pair_);
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T* Get() const {
        return pair_.GetFirst();
    };
    Deleter& GetDeleter() {
        return pair_.GetSecond();
    };
    const Deleter& GetDeleter() const {
        return pair_.GetSecond();
    };
    explicit operator bool() const {
        if (pair_.GetFirst() == nullptr) {
            return false;
        }
        return true;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Single-object dereference operators

    std::add_lvalue_reference_t<T> operator*() const {
        return *pair_.GetFirst();
    };
    T* operator->() const {
        return pair_.GetFirst();
    };

private:
    CompressedPair<T*, Deleter> pair_;
};

// Specialization for arrays
template <typename T, typename Deleter>
class UniquePtr<T[], Deleter> {
public:
    explicit UniquePtr(T* ptr = nullptr) {
        pair_.GetFirst() = ptr;
        pair_.GetSecond() = Deleter{};
    };
    template <typename NewDeleter>
    UniquePtr(T* ptr, NewDeleter deleter) {
        pair_.GetFirst() = ptr;
        pair_.GetSecond() = std::forward<NewDeleter>(deleter);
    };
    UniquePtr(const UniquePtr&) = delete;
    UniquePtr(UniquePtr&& other) noexcept {
        pair_.GetFirst() = std::move(other.pair_.GetFirst());
        pair_.GetSecond() = std::move(other.pair_.GetSecond());
        other.pair_.GetFirst() = nullptr;
    };
    template <typename U, typename DU = DefaultDeleter<U>>
    UniquePtr(UniquePtr<U, DU>&& other) noexcept {
        pair_.GetFirst() = other.Release();
        pair_.GetSecond() = std::move(other.GetDeleter());
        other.Reset(nullptr);
    };
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s
    UniquePtr& operator=(UniquePtr&& other) noexcept {
        Reset(other.Release());
        pair_.GetSecond() = std::forward<Deleter>(other.pair_.GetSecond());
        return *this;
    };
    template <typename U, typename DU = DefaultDeleter<U>>
    UniquePtr& operator=(UniquePtr<U, DU>&& other) noexcept {
        Reset(other.Release());
        pair_.GetSecond() = std::forward<Deleter>(other.GetDeleter());
        return *this;
    };
    UniquePtr& operator=(std::nullptr_t) {
        Reset();
        return *this;
    };
    UniquePtr& operator=(const UniquePtr& other) = delete;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~UniquePtr() {
        auto& ptr = pair_.GetFirst();
        if (ptr != nullptr) {
            pair_.GetSecond()(std::move(pair_.GetFirst()));
            ptr = nullptr;
        }
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    T* Release() {
        T* ptr = pair_.GetFirst();
        pair_.GetFirst() = nullptr;
        return ptr;
    };
    void Reset(T* ptr = nullptr) {
        T* old_ptr = pair_.GetFirst();
        pair_.GetFirst() = ptr;
        if (old_ptr != nullptr) {
            pair_.GetSecond()(old_ptr);
        }
    };
    void Swap(UniquePtr& other) {
        std::swap(this->pair_, other.pair_);
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T* Get() const {
        return pair_.GetFirst();
    };
    Deleter& GetDeleter() {
        return pair_.GetSecond();
    };
    const Deleter& GetDeleter() const {
        return pair_.GetSecond();
    };
    explicit operator bool() const {
        if (pair_.GetFirst() == nullptr) {
            return false;
        }
        return true;
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Single-object dereference operators

    std::add_lvalue_reference_t<T> operator*() const {
        return *pair_.GetFirst();
    };
    T* operator->() const {
        return pair_.GetFirst();
    };
    T& operator[](size_t i) {
        return pair_.GetFirst()[i];
    }

    const T& operator[](size_t i) const {
        return pair_.GetFirst()[i];
    }

private:
    CompressedPair<T*, Deleter> pair_;
};
