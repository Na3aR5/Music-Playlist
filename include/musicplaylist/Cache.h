#ifndef MUSIC_PLAYLIST_CACHE_HEADER
#define MUSIC_PLAYLIST_CACHE_HEADER

#include <list>
#include <unordered_map>
#include <functional>

namespace mspl {
	template <typename KeyT, typename ValueT>
	class LRUCache {
	public:
		using KeyType = KeyT;
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

	template <typename Function, typename ... Types>
	class MemoizedFunction {
	public:
		using ReturnType = std::invoke_result_t<Function, Types...>;

	public:
		MemoizedFunction() = default;

		ReturnType operator()(const Types& ... args) {
			std::tuple<Types...> key = std::tuple<Types...>(args...);
			ReturnType* valuePtr = m_cache.Get(key);
			if (valuePtr) {
				return *valuePtr;
			}
			ReturnType value = m_function(args...);
			m_cache.Insert(key, value);
			return value;
		}

		template <typename FFunction>
		void SetFunction(FFunction function) {
			m_function = function;
		}

	private:
		std::function<Function> m_function;
		LRUCache<std::tuple<Types...>, ReturnType> m_cache;
	};
}

#endif // !MUSIC_PLAYLIST_CACHE_HEADER