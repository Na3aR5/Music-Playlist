#ifndef MUSIC_PLAYLIST_CACHE_HEADER
#define MUSIC_PLAYLIST_CACHE_HEADER

#include <map>
#include <list>

namespace {
	template <typename CacheT, typename Function>
	class MemoizedFunction {
	public:
		MemoizedFunction(CacheT* cache, Function& function) :
			m_cache(cache), m_function(function) {}

		MemoizedFunction(CacheT* cache, Function&& function) :
			m_cache(cache), m_function(std::move(function)) {}

	public:
		template <typename ... Types>
		auto operator()(const CacheT::KeyType& key, Types&& ... args) -> decltype(Function()(std::forward<Types>(args)...)) {
			auto result = m_function(std::forward<Types>(args)...);
			m_cache->Insert(key, result);
		}

	public:
		CacheT*  m_cache = nullptr;
		Function m_function;
	};

	template <typename KeyT, typename ValueT>
	class LRUCache {
	public:
		using KeyType   = KeyT;
		using ValueType = ValueT;

	public:
		LRUCache(uint32_t capacity = UINT32_MAX) : m_capacity(capacity) {}

	public:
		template <typename Function>
		MemoizedFunction<LRUCache, Function> GetMemoized(Function& function) {
			return MemoizedFunction<LRUCache, Function>(this, function);
		}

		void Insert(const KeyType& key, const ValueType& value) {
			auto it = m_keys.find(key);
			if (it != m_keys.cend()) {
				m_values.erase(it->second);
				m_values.emplace_back(key, value);
				it->second = --m_values.end();
				return;
			}
			if (m_values.size() == m_capacity) {
				const KeyType& lruKey = m_values.front().first;
				m_values.pop_front();
				m_keys.erase(lruKey);
			}
			m_values.emplace_back(key, value);
			m_keys.emplace(key, --m_values.end());
		}

		ValueType* Get(const KeyType& key) {
			auto it = m_keys.find(key);
			if (it != m_keys.cend()) {
				ValueType& value = it->second->second;
				m_values.erase(it->second);
				m_values.emplace_back(key, value);
				it->second = --m_values.end();

				return &value;
			}
			return nullptr;
		}

		const ValueType* Get(const KeyType& key) const {
			auto it = m_keys.find(key);
			if (it != m_keys.cend()) {
				ValueType& value = it->second->second;
				m_values.erase(it->second);
				m_values.emplace_back(key, value);
				it->second = --m_values.end();

				return &value;
			}
			return nullptr;
		}

	private:
		uint32_t m_capacity = 0;
		std::list<std::pair<KeyType, ValueType>> m_values;
		std::map<KeyType, typename std::list<std::pair<KeyType, ValueType>>::iterator> m_keys;
	};
}

#endif // !MUSIC_PLAYLIST_CACHE_HEADER