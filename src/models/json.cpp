/**
 * @file json.cpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Brief description
 * @version 1.0.0
 * @date 2026-06-06
 *
 * @copyright Copyright (c) 2026
 */

#include "zuu_json/models/json.hpp"
#include "parser/parser.hpp"
#include "tokenizer/tokenizer.hpp"
#include <algorithm>


namespace zuu::models {

Json::~Json() noexcept = default;

Json::Json(std::unique_ptr<Storage> storage) noexcept
    : storage_(std::move(storage)) {}

Json::Result<Json> Json::parse(std::string_view content) noexcept {
    const auto raw = std::span<const char>(content.data(), content.size());

    auto tokens = tokenizer::Tokenizer::Tokenize(raw);
    if (!tokens) {
        return std::unexpected{tokens.error()};
    }

    parser::Parser parser(tokens.value().first, tokens.value().second);
    auto parsed = std::move(parser).result();
    if (!parsed) {
        return std::unexpected{parsed.error()};
    }

    return Json(std::make_unique<Storage>(std::move(parsed.value())));
}

Value Json::root() const noexcept {
    return Value::fromInternal(storage_.get(), storage_->root());
}

Json::Result<Value> Json::operator[](std::string_view key) const noexcept {
    const auto& root_val = storage_->root();

    if (root_val.get_type() != JsonValue::Type::Object) {
        return std::unexpected{Error::IsNotObject};
    }

    const auto obj = storage_->object(root_val.as_index());

    // Binary Search: O(log N) Lookup
    auto it = std::lower_bound(
        obj.begin(), obj.end(), key, [this](const JsonMember& member, std::string_view k) {
            return storage_->string(member.key_index_) < k;
        });

    if (it != obj.end() && storage_->string(it->key_index_) == key) {
        return Value::fromInternal(storage_.get(), it->value_);
    }

    return std::unexpected{Error::InvalidValue};
}

} // namespace zuu::models