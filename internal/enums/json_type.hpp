/**
 * @file json_type.hpp
 * @author zuudevs (zuudevs@gmail.com)
 * @brief Brief description
 * @version 0.1.0
 * @date 2026-07-01
 * 
 * @copyright Copyright (c) 2026
 */

#pragma once

namespace zuu::enums {

enum class JsonType : unsigned char {
	Null,
	Boolean,
	Integer,
	Double,
	String,
	Array,
	Object,
};

} // namespace zuu::enums