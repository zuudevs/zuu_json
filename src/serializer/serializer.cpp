/**
 * @file serializer.cpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Brief description
 * @version 1.0.0
 * @date 2026-06-21
 *
 * @copyright Copyright (c) 2026
 */

#include <array>
#include <charconv>
#include <cstring>
#include "serializer/serializer.hpp"
#include "utils/swar.hpp"

namespace zuu::serializer {

Serializer::Serializer(const allocators::Storage* storage, int indent) noexcept
    : indent_(indent), storage_(storage) {
    
    std::size_t estimated_size = 4096; 
    if (storage) {
        estimated_size += (storage->getArrayElementsCount() + storage->getObjectElementsCount()) * 24;
    }
    
    out_.reserve(estimated_size);
}

std::string Serializer::dump(const allocators::Storage* storage, const models::JsonValue& root, int indent) noexcept {
    Serializer s(storage, indent);
    s.serializeValue(root);
    return std::move(s.out_);
}

void Serializer::writeIndent() noexcept {
    if (indent_ >= 0) {
        out_ += '\n';
        out_.append(current_indent_, ' ');
    }
}

void Serializer::serializeValue(const models::JsonValue& value) noexcept {
    switch (value.get_type()) {
        case enums::JsonType::Null:    out_.append("null", 4); break;
        case enums::JsonType::Boolean: 
            if (value.as_bool()) out_.append("true", 4);
            else out_.append("false", 5);
            break;
        case enums::JsonType::Integer: serializeInteger(value.as_integer()); break;
        case enums::JsonType::Double:  serializeDouble(value.as_double()); break;
        case enums::JsonType::String:  serializeString(storage_->string(value.as_index())); break;
        case enums::JsonType::Array:   serializeArray(value); break;
        case enums::JsonType::Object:  serializeObject(value); break;
    }
}

void Serializer::serializeString(std::string_view str) noexcept {
    out_ += '"';
    
    static constexpr auto needs_escape = []() {
        std::array<bool, 256> arr{};
        for (int i = 0; i < 0x20; ++i) arr[i] = true;
        arr[static_cast<uint8_t>('\"')] = true;
        arr[static_cast<uint8_t>('\\')] = true;
        return arr;
    }();

    const char* ptr = str.data();
    const char* end = ptr + str.size();
    const char* start = ptr;

    while (ptr < end) {
        while (ptr + sizeof(uint64_t) <= end) {
            uint64_t block{};
            std::memcpy(&block, ptr, sizeof(uint64_t));

            uint64_t quote_mask  = utils::find_zero_byte_mask(block ^ constants::swar8_dqt);
            uint64_t escape_mask = utils::find_zero_byte_mask(block ^ constants::swar8_esc);
            uint64_t ctrl_mask   = (block - constants::swar8_sp) & ~block & constants::swar8_msb;

            uint64_t mask = quote_mask | escape_mask | ctrl_mask;
            if (mask != 0) {
                ptr += (std::countr_zero(mask) >> 3);
                goto escape_found;
            }
            ptr += sizeof(uint64_t);
        }

        while (ptr < end && !needs_escape[static_cast<unsigned char>(*ptr)]) {
            ++ptr;
        }

    escape_found:
        if (ptr > start) {
            out_.append(start, static_cast<std::size_t>(ptr - start));
        }

        if (ptr == end) break;

        char c = *ptr;
        switch (c) {
            case '"':  out_.append("\\\"", 2); break;
            case '\\': out_.append("\\\\", 2); break;
            case '\b': out_.append("\\b", 2); break;
            case '\f': out_.append("\\f", 2); break;
            case '\n': out_.append("\\n", 2); break;
            case '\r': out_.append("\\r", 2); break;
            case '\t': out_.append("\\t", 2); break;
            default: {
                char buf[7] = "\\u0000";
                constexpr char hex_chars[] = "0123456789abcdef";
                buf[4] = hex_chars[(c >> 4) & 0xF];
                buf[5] = hex_chars[c & 0xF];
                out_.append(buf, 6);
                break;
            }
        }
        ++ptr;
        start = ptr;
    }
    out_ += '"';
}

void Serializer::serializeInteger(long long val) noexcept {
    char buf[32];
    auto [ptr, ec] = std::to_chars(buf, buf + sizeof(buf), val);
    if (ec == std::errc()) {
        out_.append(buf, ptr - buf);
    } else {
        out_.append("0", 1);
    }
}

void Serializer::serializeDouble(double val) noexcept {
    char buf[64];
    auto [ptr, ec] = std::to_chars(buf, buf + sizeof(buf), val);
    if (ec == std::errc()) {
        out_.append(buf, ptr - buf);
    } else {
        out_.append("0.0", 3);
    }
}

void Serializer::serializeArray(const models::JsonValue& value) noexcept {
    out_ += '[';
    const auto arr = storage_->array(value.as_index());
    if (arr.empty()) {
        out_ += ']';
        return;
    }

    if (indent_ >= 0) current_indent_ += indent_;
    
    bool first = true;
    for (const auto& item : arr) {
        if (!first) out_ += ',';
        writeIndent();
        serializeValue(item);
        first = false;
    }

    if (indent_ >= 0) {
        current_indent_ -= indent_;
        writeIndent();
    }
    out_ += ']';
}

void Serializer::serializeObject(const models::JsonValue& value) noexcept {
    out_ += '{';
    const auto obj = storage_->object(value.as_index());
    if (obj.empty()) {
        out_ += '}';
        return;
    }

    if (indent_ >= 0) current_indent_ += indent_;
    
    bool first = true;
    for (const auto& member : obj) {
        if (!first) out_ += ',';
        writeIndent();
        serializeString(storage_->resolveKey(member));
        out_ += (indent_ >= 0) ? ": " : ":";
        serializeValue(member.value_);
        first = false;
    }

    if (indent_ >= 0) {
        current_indent_ -= indent_;
        writeIndent();
    }
    out_ += '}';
}

} // namespace zuu::serializer