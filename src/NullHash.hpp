#pragma once

#include <unordered_map>

template <typename T>
struct NullHash {
	std::size_t operator()(const T& t) const noexcept {
		return t;
	}
};

template <typename K, typename V>
using NullHashMap = std::unordered_map<K, V, NullHash<K>>;
