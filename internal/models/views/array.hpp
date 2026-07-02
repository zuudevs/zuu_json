/**
 * @file array.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Brief description
 * @version 0.1.0
 * @date 2026-06-21
 *
 * @copyright Copyright (c) 2026
 */

#pragma once

#include "allocators/storage.hpp"

namespace zuu::models {

class Value;

} // namespace zuu::models

namespace zuu::models::views {

class Array {
  private:
    const allocators::Storage* storage_{nullptr};
    std::span<const JsonValue> span_{};

  public:
    Array() = default;
    Array(const allocators::Storage* storage, std::span<const JsonValue> span) noexcept
        : storage_(storage)
        , span_(span) {}

    class Iterator {
        const allocators::Storage* storage_{nullptr};
        const JsonValue* ptr_{nullptr};

      public:
        using iterator_concept = std::random_access_iterator_tag;
        using iterator_category = std::random_access_iterator_tag;
        using value_type = Value;
        using difference_type = std::ptrdiff_t;
        using pointer = void;
        using reference = Value;

        Iterator() = default;
        Iterator(const allocators::Storage* storage, const JsonValue* ptr) noexcept
            : storage_(storage)
            , ptr_(ptr) {}

        [[nodiscard]] Value
            operator*() const noexcept;
        [[nodiscard]] Value
            operator[](difference_type n) const noexcept;

        Iterator&
            operator++() noexcept {
            ++ptr_;
            return *this;
        }
        Iterator
            operator++(int) noexcept {
            auto tmp = *this;
            ++ptr_;
            return tmp;
        }
        Iterator&
            operator--() noexcept {
            --ptr_;
            return *this;
        }
        Iterator
            operator--(int) noexcept {
            auto tmp = *this;
            --ptr_;
            return tmp;
        }

        Iterator&
            operator+=(difference_type n) noexcept {
            ptr_ += n;
            return *this;
        }
        Iterator&
            operator-=(difference_type n) noexcept {
            ptr_ -= n;
            return *this;
        }

        [[nodiscard]] friend Iterator
            operator+(Iterator it, difference_type n) noexcept {
            it += n;
            return it;
        }
        [[nodiscard]] friend Iterator
            operator+(difference_type n, Iterator it) noexcept {
            it += n;
            return it;
        }
        [[nodiscard]] friend Iterator
            operator-(Iterator it, difference_type n) noexcept {
            it -= n;
            return it;
        }
        [[nodiscard]] friend difference_type
            operator-(const Iterator& a, const Iterator& b) noexcept {
            return a.ptr_ - b.ptr_;
        }

        [[nodiscard]] auto
            operator<=>(const Iterator& other) const noexcept {
            return ptr_ <=> other.ptr_;
        }
        [[nodiscard]] bool
            operator==(const Iterator& other) const noexcept {
            return ptr_ == other.ptr_;
        }
    };

    [[nodiscard]] Iterator
        begin() const noexcept {
        return Iterator(storage_, span_.data());
    }
    [[nodiscard]] Iterator
        end() const noexcept {
        return Iterator(storage_, span_.data() + span_.size());
    }
    [[nodiscard]] std::size_t
        size() const noexcept {
        return span_.size();
    }
    [[nodiscard]] bool
        empty() const noexcept {
        return span_.empty();
    }
    [[nodiscard]] Value
        operator[](std::size_t index) const noexcept;
};

} // namespace zuu::models::views