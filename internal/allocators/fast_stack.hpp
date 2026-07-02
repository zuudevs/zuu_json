/**
 * @file fast_stack.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Raw pointer-based stack for Trivially Copyable types with SBO
 * @version 1.6.0
 * @date 2026-06-29
 *
 * @copyright Copyright (c) 2026
 */

#pragma once

#include "constants/general.hpp"
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <type_traits>
#include <utility>

namespace zuu::allocators {

template <typename T, uint32_t InlineCapacity = 512>
class FastStack {
    static_assert(std::is_trivially_copyable_v<T>, "FastStack requires trivially copyable types");

  public:
    FastStack() noexcept
        : data_(reinterpret_cast<T*>(inline_buffer_))
        , size_(constants::zero)
        , capacity_(InlineCapacity) {}

    ~FastStack() noexcept {
        if (data_ != reinterpret_cast<T*>(inline_buffer_)) {
            std::free(data_);
        }
    }

    FastStack(const FastStack&) = delete;
    FastStack&
        operator=(const FastStack&) = delete;

    FastStack(FastStack&& other) noexcept
        : size_(other.size_)
        , capacity_(other.capacity_) {

        if (other.data_ == reinterpret_cast<T*>(other.inline_buffer_)) {
            data_ = reinterpret_cast<T*>(inline_buffer_);
            std::memcpy(data_, other.data_, size_ * sizeof(T));
        } else {
            data_ = other.data_;
        }

        other.data_ = reinterpret_cast<T*>(other.inline_buffer_);
        other.size_ = constants::zero;
        other.capacity_ = InlineCapacity;
    }

    FastStack&
        operator=(FastStack&& other) noexcept {
        if (this != &other) {
            if (data_ != reinterpret_cast<T*>(inline_buffer_)) {
                std::free(data_);
            }

            size_ = other.size_;
            capacity_ = other.capacity_;

            if (other.data_ == reinterpret_cast<T*>(other.inline_buffer_)) {
                data_ = reinterpret_cast<T*>(inline_buffer_);
                std::memcpy(data_, other.data_, size_ * sizeof(T));
            } else {
                data_ = other.data_;
            }

            other.data_ = reinterpret_cast<T*>(other.inline_buffer_);
            other.size_ = constants::zero;
            other.capacity_ = InlineCapacity;
        }
        return *this;
    }

    inline void
        push_back(const T& item) noexcept {
        if (size_ == capacity_) [[unlikely]] {
            uint32_t new_capacity = capacity_ * 2;
            T* new_data = static_cast<T*>(std::malloc(new_capacity * sizeof(T)));

            std::memcpy(new_data, data_, size_ * sizeof(T));

            if (data_ != reinterpret_cast<T*>(inline_buffer_)) {
                std::free(data_);
            }
            data_ = new_data;
            capacity_ = new_capacity;
        }
        data_[size_++] = item;
    }

    inline void
        resize(uint32_t new_size) noexcept {
        size_ = new_size;
    }

    [[nodiscard]] inline T*
        data() noexcept {
        return data_;
    }
    [[nodiscard]] inline const T*
        data() const noexcept {
        return data_;
    }
    [[nodiscard]] inline uint32_t
        size() const noexcept {
        return size_;
    }

  private:
    T* data_;
    uint32_t size_;
    uint32_t capacity_;

    alignas(T) std::byte inline_buffer_[InlineCapacity * sizeof(T)];
};

} // namespace zuu::allocators