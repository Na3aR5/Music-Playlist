#ifndef MUSIC_PLAYLIST_BIDIR_PQUEUE_HEADER
#define MUSIC_PLAYLIST_BIDIR_PQUEUE_HEADER

#include <map>
#include <list>

namespace mspl {
	template <typename T, typename PriorityT>
	class BidirPriorityQueue {
	public:
		struct Slot;
		using MapType = std::multimap<PriorityT, typename std::list<Slot>::iterator>;

		struct Slot {
		public:
			Slot() : value(), priority() {}
			Slot(const T& value, const PriorityT& priority) :
				value(value), priority(priority), mapIt() {}

		public:
			T				  value;
			PriorityT		  priority;
			MapType::iterator mapIt;
		};

	public:
		void Enqueue(const T& value, const PriorityT& priority) {
			m_orderList.emplace_back(value, priority);
			m_orderList.back().mapIt = m_priorityMap.emplace(priority, --m_orderList.end());
		}

		T DequeueHighest() {
			auto highestIt = --m_priorityMap.end();
			T value = std::move(highestIt->second->value);

			m_orderList.erase(highestIt->second);
			m_priorityMap.erase(highestIt);

			return value;
		}

		T DequeueLowest() {
			auto lowestIt = m_priorityMap.begin();
			T value = std::move(lowestIt->second->value);

			m_orderList.erase(lowestIt->second);
			m_priorityMap.erase(lowestIt);

			return value;
		}

		T DequeueNewest() {
			auto newestIt = --m_orderList.end();
			T value = std::move(newestIt->value);

			m_priorityMap.erase(m_orderList.back().mapIt);
			m_orderList.pop_back();

			return value;
		}

		T DequeueOldest() {
			auto oldestIt = m_orderList.begin();
			T value = std::move(oldestIt->value);

			m_priorityMap.erase(m_orderList.front().mapIt);
			m_orderList.pop_front();

			return value;
		}

		const T& PeekHighest() const {
			return (--m_priorityMap.end())->second.value;
		}

		const T& PeekLowest() const {
			return m_priorityMap.begin()->second.value;
		}

		const T& PeekNewest() const {
			return m_orderList.back().value;
		}

		const T& PeekOldest() const {
			return m_orderList.front().value;
		}

	public:
		bool IsEmpty() const noexcept { return m_orderList.empty(); }
		bool GetSize() const noexcept { return m_orderList.size(); }

	private:
		std::list<Slot> m_orderList;
		MapType         m_priorityMap;
	};
}

#endif // !MUSIC_PLAYLIST_BIDIR_PQUEUE_HEADER