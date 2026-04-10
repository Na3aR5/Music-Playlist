#ifndef JADE_EVENT_SYSTEM_HEADER
#define JADE_EVENT_SYSTEM_HEADER

#include <functional>
#include <vector>
#include <array>

namespace jade {
	class EventSystem;

	template <typename EventData>
	class EventEmitter {
	public:
		static_assert(sizeof(EventData) <= EventSystem::s_MaxEventDataBytes,
			"jade::EventSystem - exceeded max event data struct size in bytes");

	public:
		EventEmitter() = default;

	public:
		void Emit(const EventData& data) { EventSystem::Get().Register(data); }
		void Emit(EventData&& data) { EventSystem::Get().Register(std::move(data)); }

		void Emit() requires(std::is_empty_v<EventData>) {
			EventSystem::Get().template Register<EventData>();
		}

		void EmitInstant(const EventData& data) { EventSystem::Get().RegisterInstant(data); }

		void EmitInstant() requires(std::is_empty_v<EventData>) {
			EventSystem::Get().template RegisterInstant<EventData>();
		}
	};

	class EventSystem {
	public:
		template <typename EventData>
		struct EventID {
			inline static uint32_t id = UINT32_MAX;
		};

		template <>
		struct EventID<void> {
			inline static uint32_t ids = 0;
		};

		class MemoryStorage {
		public:
			MemoryStorage() = default;
			MemoryStorage(size_t bytes);

			~MemoryStorage();

		public:
			void Allocate(size_t bytes);
			void Deallocate();

			template <typename T>
			void Push(T&& value) {
				if (m_storageTail - m_storageHead + sizeof(T) >= m_allocated) {
					return;
				}
				T* where = (T*)(m_storage + m_storageTail);
				new (where) T(std::forward<T>(value));
				m_storageTail += sizeof(T);
			}

			template <typename T>
			void Pop() {
				if constexpr (!std::is_trivially_destructible_v<T>) {
					if (m_storageHead + sizeof(T) <= m_storageTail) {
						(T*)(m_storage + m_storageHead)->~T();
					}
				}
				m_storageHead += sizeof(T);
			}

			template <typename T>
			T* Get() {
				return (T*)(m_storage + m_storageHead);
			}

			inline void Reset() { m_storageHead = m_storageTail = 0; }

		private:
			size_t	   m_allocated	 = 0;
			size_t     m_storageHead = 0;
			size_t     m_storageTail = 0;
			std::byte* m_storage     = nullptr;
		};

	public:
		template <typename T>
		friend class EventEmitter;

		EventSystem();
		~EventSystem();

	public:
		static constexpr uint16_t s_QueueSize		  = 1024;
		static constexpr uint16_t s_MaxEventDataBytes = 256;

		static EventSystem& Get() noexcept;
		static const EventSystem& GetConst() noexcept;

	public:
		template <typename EventData>
		void Register(EventData&& data) {
			_ValidateEvent<EventData>();

			if (m_eventQueueTail - m_eventQueueHead >= s_QueueSize) {
				return;
			}
			m_eventQueue[m_eventQueueTail++] = EventID<EventData>::id;
			m_eventsData.Push<EventData>(std::forward<EventData>(data));
		}

		template <typename EventData>
		void Register() requires(std::is_empty_v<EventData>) {
			_ValidateEvent<EventData>();

			if (m_eventQueueTail - m_eventQueueHead >= s_QueueSize) {
				return;
			}
			m_eventQueue[m_eventQueueTail++] = EventID<EventData>::id;
		}

		template <typename EventData>
		void RegisterInstant(const EventData& data) {
			for (auto& subscriber : m_subscriberTable[EventID<EventData>::id]) {
				subscriber(&data);
			}
		}

		template <typename EventData>
		void RegisterInstant() requires(std::is_empty_v<EventData>) {
			for (auto& subscriber : m_subscriberTable[EventID<EventData>::id]) {
				subscriber(nullptr);
			}
		}

		template <typename EventData, typename Function>
		void Subscribe(uint64_t priority, Function function) {
			_ValidateEvent<EventData>();

			auto comparator = [](const _Subscriber& l, const _Subscriber& r) -> bool {
				return l.priority > r.priority;
			};
			auto pos = std::lower_bound(
				m_subscriberTable[EventID<EventData>::id].begin(),
				m_subscriberTable[EventID<EventData>::id].end(),
				_Subscriber{ .priority = priority, .function = {} },
				comparator
			);

			_Subscriber subscriber = {};
			subscriber.priority = priority;
			subscriber.function = [function](void* data) {
				if constexpr (std::is_empty_v<EventData>) {
					function();
				}
				else {
					EventData& ref = *(EventData*)data;
					function(ref);
				}
			};
			m_subscriberTable[EventID<EventData>::id].insert(pos, std::move(subscriber));
		}

		void Dispatch();

	private:
		template <typename EventData>
		void _ValidateEvent() noexcept {
			static_assert(sizeof(EventData) <= EventSystem::s_MaxEventDataBytes,
				"jade::EventSystem - exceeded max event data struct size in bytes");

			if (EventSystem::EventID<EventData>::id == UINT32_MAX) {
				EventSystem::EventID<EventData>::id = EventSystem::EventID<void>::ids;
				++EventSystem::EventID<void>::ids;

				if constexpr (!std::is_empty_v<EventData>) {
					m_dispatchers.emplace_back([this]() {
						EventData* data = m_eventsData.Get<EventData>();
						for (auto& subscriber : m_subscriberTable[EventSystem::EventID<EventData>::id]) {
							subscriber.function(data);
						}
						m_eventsData.Pop<EventData>();
					});
				}
				else {
					m_dispatchers.emplace_back([this]() {
						for (auto& subscriber : m_subscriberTable[EventSystem::EventID<EventData>::id]) {
							subscriber.function(nullptr);
						}
					});
				}
			}
		}

	private:
		struct _Subscriber {
			size_t					   priority = 0;
			std::function<void(void*)> function;
		};

		using _SubscriberTable = std::vector<std::vector<_Subscriber>>;
		using _Dispatchers     = std::vector<std::function<void()>>;

		std::array<uint32_t, s_QueueSize> m_eventQueue;
		MemoryStorage					  m_eventsData;
		uint32_t						  m_eventQueueHead = 0;
		uint32_t						  m_eventQueueTail = 0;
		_Dispatchers                      m_dispatchers;
		_SubscriberTable			      m_subscriberTable;
	};
}

#endif // !JADE_EVENT_SYSTEM_HEADER