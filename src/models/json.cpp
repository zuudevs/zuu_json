/**
 * @file json.cpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Brief description
 * @version 1.0.0
 * @date 2026-06-06
 *
 * @copyright Copyright (c) 2026
 */

#include <algorithm>
#include "parser/parser.hpp"
#include "tokenizer/tokenizer.hpp"
#include "utils/strings.hpp"
#include "zuu_json/models/json.hpp"

namespace zuu::models {

Json::Json(Json&&) noexcept = default;
Json& Json::operator=(Json&&) noexcept = default;

Json::~Json() noexcept = default;

Json::Json(std::unique_ptr<Storage> storage) noexcept
    : storage_(std::move(storage)) {}

Json::Result<Json> Json::parse(std::string_view content) noexcept {
    const auto raw = std::span<const char>(
		content.data(), 
		content.size()
	);
    auto tokens = tokenizer::Tokenizer::Tokenize(raw);
    if (!tokens) {
        return std::unexpected{tokens.error()};
    }

    parser::Parser parser(
		tokens.value().first, 
		tokens.value().second
	);
    auto parsed = std::move(parser).result();
    if (!parsed) {
        return std::unexpected{parsed.error()};
    }

    return Json(std::make_unique<Storage>(std::move(parsed.value())));
}

void Json::sort() noexcept {
    if (storage_) {
        storage_->sortAllObjects();
    }
}

Value Json::root() const noexcept {
    return Value::fromInternal(storage_.get(), storage_->root());
}

Json::Result<Value> Json::operator[](std::string_view key) const noexcept {
    const auto& root_val = storage_->root();

    if (root_val.get_type() != JsonValue::Type::Object) {
        return std::unexpected{Error::IsNotObject};
    }

    const auto obj_index = root_val.as_index();
    const auto obj = storage_->object(obj_index);
    const bool is_sorted = storage_->isObjectSorted(obj_index);

    if (is_sorted) {
        auto it = std::ranges::lower_bound(
            obj, 
            key, 
            {}, 
            [this](const JsonMember& member) {
                return storage_->resolveKey(member);
            }
        );

        if (it != obj.end() && storage_->resolveKey(*it) == key) {
            return Value::fromInternal(storage_.get(), it->value_);
        }
    } else {
        auto it = std::ranges::find_if(
            obj, 
            [this, key](const JsonMember& member) {
                return member.key_.length_ == key.size() && storage_->resolveKey(member) == key;
            }
        );

        if (it != obj.end()) {
            return Value::fromInternal(storage_.get(), it->value_);
        }
    }

    return std::unexpected{Error::InvalidValue};
}

} // namespace zuu::models