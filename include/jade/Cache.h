#ifndef JADE_CACHE_HEADER
#define JADE_CACHE_HEADER

#include <list>
#include <unordered_map>
#include <functional>

namespace jade {
	template <typename KeyT, typename ValueT>
	class LRUCache {
	public:
		using KeyType   = KeyT;
		using ValueType = ValueT;

	public:
		LRUCache(uint32_t capacity = UINT32_MAX) : m_capacity(capacity) {}

	public:
		void Insert(const KeyType& key, const ValueType& value) {
			auto it = m_keys.find(key);
			if (it != m_keys.cend()) {
				m_values.erase(it->second);
				m_values.emplace_back(key, value);
				it->second = --m_values.end();
				return;
			}
			if (m_values.size() == m_capacity) {
				KeyType& frontKey = m_values.front().first;
				m_keys.erase(frontKey);
				m_values.pop_front();
			}
			m_values.emplace_back(key, value);
			m_keys.emplace(key, --m_values.end());
		}

		void Insert(const KeyType& key, ValueType&& value) {
			auto it = m_keys.find(key);
			if (it != m_keys.cend()) {
				m_values.erase(it->second);
				m_values.emplace_back(key, std::move(value));
				it->second = --m_values.end();
				return;
			}
			if (m_values.size() == m_capacity) {
				KeyType& frontKey = m_values.front().first;
				m_keys.erase(frontKey);
				m_values.pop_front();
			}
			m_values.emplace_back(key, std::move(value));
			m_keys.emplace(key, --m_values.end());
		}

		ValueType* Get(const KeyType& key) {
			auto it = m_keys.find(key);
			if (it != m_keys.cend()) {
				const ValueType& value = it->second->second;
				m_values.emplace_back(key, value);
				m_values.erase(it->second);
				it->second = --m_values.end();
				return &m_values.back().second;
			}
			return nullptr;
		}

	private:
		uint32_t m_capacity = UINT32_MAX;
		std::list<std::pair<KeyType, ValueType>> m_values;
		std::unordered_map<KeyType, typename std::list<std::pair<KeyType, ValueType>>::iterator> m_keys;
	};
}

#endif // !JADE_CACHE_HEADER