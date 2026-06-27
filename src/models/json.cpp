/**
 * @file json.cpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Brief description
 * @version 1.1.0
 * @date 2026-06-27
 *
 * @copyright Copyright (c) 2026
 */

#include <algorithm>
#include <expected>
#include "parser/parser.hpp"
#include "tokenizer/policies.hpp"
#include "tokenizer/tokenizer.hpp"
#include "utils/compiler.hpp"
#include "zuu_json/core/engine.hpp"
#include "zuu_json/models/json.hpp"

namespace {
    [[nodiscard]] inline auto switch_tokenizer(std::span<const char> raw, const zuu::models::Policy& policy) noexcept {
        switch (policy.tokenizer_engine) {
            case zuu::core::TokenizerEngine::Avx2:
                return zuu::tokenizer::Tokenizer<zuu::tokenizer::Avx2Policy>::Tokenize(raw);
            default:
                return zuu::tokenizer::Tokenizer<zuu::tokenizer::SwarPolicy>::Tokenize(raw);
        }
    }

    [[nodiscard]] inline auto switch_parser(
        std::span<const zuu::models::Token> tokens, 
        const zuu::traits::HintTrait<zuu::models::Token>& hint, 
        const zuu::models::Policy& policy) noexcept {
        
        switch (policy.parser_engine) {
            case zuu::core::ParserEngine::Avx2:
                return zuu::parser::Parser<zuu::parser::Avx2Policy>::Parse(tokens, hint);
            default:
                return zuu::parser::Parser<zuu::parser::DefaultPolicy>::Parse(tokens, hint);
        }
    }
} // namespace

namespace zuu::models {

Json::Json(Json&&) noexcept = default;
Json& Json::operator=(Json&&) noexcept = default;
Json::~Json() noexcept = default;

Json::Json(std::unique_ptr<Storage> storage) noexcept
    : storage_(std::move(storage)) {}

Json::Result<Json> Json::parse(std::string_view content, Policy policy) noexcept {
    const auto raw = std::span<const char>(
        content.data(), 
        content.size()
    );
    
    auto tokens = switch_tokenizer(raw, policy);
    if (!tokens) { 
        return std::unexpected{tokens.error()}; 
    }

    auto parsed = switch_parser(tokens.value().first, tokens.value().second, policy);
    if (!parsed) { 
        return std::unexpected{parsed.error()}; 
    }

    return Json(std::make_unique<Storage>(std::move(parsed.value())));
}

void Json::sort() noexcept {
    if (storage_) { storage_->sortAllObjects(); }
}

Value Json::root() const noexcept {
    return Value::fromInternal(storage_.get(), storage_->root());
}

std::string Json::dump(int indent) const noexcept {
    return root().dump(indent);
}

ZUU_HOT ZUU_ALIGN(64) Json::Result<Value> Json::operator[](std::string_view key) const noexcept {
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
            [](std::string_view a, std::string_view b) {
                if (a.size() != b.size()) {
                    return a.size() < b.size();
                }
                return a < b;
            },
            [this](const JsonMember& member) { 
                return storage_->resolveKey(member); 
            }
        );
        if (
            it != obj.end() && 
            storage_->resolveKey(*it) == key
        ) {
            return Value::fromInternal(
                storage_.get(), 
                it->value_
            );
        }
    } else {
        auto it = std::ranges::find_if(
            obj, 
            [this, key](const JsonMember& member) {
                auto m_key = storage_->resolveKey(member);
                return m_key.size() == key.size() && m_key == key;
            }
        );
        if (it != obj.end()) { 
            return Value::fromInternal(
                storage_.get(), 
                it->value_
            ); 
        }
    }

    return std::unexpected{Error::InvalidValue};
}

} // namespace zuu::models