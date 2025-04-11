#pragma once

#include <type_traits>
#include <algorithm>

template <typename T, bool = std::is_empty_v<T>>
struct CompressedPairMember {
    CompressedPairMember() = default;

    CompressedPairMember(const T& member) : member_(member) {
    }

    CompressedPairMember(T&& member) : member_(std::move(member)) {
    }

    T& Get() {
        return member_;
    }

    const T& Get() const {
        return member_;
    }

    T member_;
};

template <typename T>
struct CompressedPairMember<T, true> : private T {
    CompressedPairMember() = default;

    CompressedPairMember(const T&) {
    }

    CompressedPairMember(T&&) {
    }

    T& Get() {
        return *this;
    }

    const T& Get() const {
        return *this;
    }
};

template <typename F, typename S>
struct CompressedPair : private CompressedPairMember<F>, private CompressedPairMember<S> {
    CompressedPair() = default;

    CompressedPair(const F& first, const S& second)
        : CompressedPairMember<F>(first), CompressedPairMember<S>(second) {
    }

    CompressedPair(F&& first, S&& second)
        : CompressedPairMember<F>(std::move(first)), CompressedPairMember<S>(std::move(second)) {
    }

    F& GetFirst() {
        return CompressedPairMember<F>::Get();
    }

    const F& GetFirst() const {
        return CompressedPairMember<F>::Get();
    }

    S& GetSecond() {
        return CompressedPairMember<S>::Get();
    };

    const S& GetSecond() const {
        return CompressedPairMember<S>::Get();
    };
};
