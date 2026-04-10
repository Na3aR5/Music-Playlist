#include <jade/EventSystem.h>

#include <stdexcept>

jade::EventSystem::MemoryStorage::MemoryStorage(size_t bytes) {
	m_storage = (std::byte*)::operator new (bytes);
	m_allocated = bytes;
}

jade::EventSystem::MemoryStorage::~MemoryStorage() {
	Deallocate();
}

void jade::EventSystem::MemoryStorage::Allocate(size_t bytes) {
	if (bytes <= m_allocated) {
		return;
	}
	Deallocate();

	m_storage = (std::byte*)::operator new (bytes);
	m_allocated = bytes;
}

void jade::EventSystem::MemoryStorage::Deallocate() {
	if (m_allocated > 0) {
		::operator delete (m_storage, m_allocated);
		m_allocated = 0;
	}
	m_storage = nullptr;
}

namespace {
	jade::EventSystem* g_EventSystem = nullptr;
}

jade::EventSystem::EventSystem() : m_eventQueue() {
	if (g_EventSystem != nullptr) {
		throw std::runtime_error("EventSystem is already created");
	}
	m_eventsData.Allocate(s_QueueSize * s_MaxEventDataBytes);

	m_subscriberTable.resize(128);
	m_dispatchers.reserve(128);

	g_EventSystem = this;
}

jade::EventSystem::~EventSystem() {
	g_EventSystem = nullptr;
}

jade::EventSystem& jade::EventSystem::Get() noexcept {
	return *g_EventSystem;
}

const jade::EventSystem& jade::EventSystem::GetConst() noexcept {
	return *g_EventSystem;
}

void jade::EventSystem::Dispatch() {
	while (m_eventQueueHead < m_eventQueueTail) {
		uint32_t eventId = m_eventQueue[m_eventQueueHead];
		m_dispatchers[eventId]();
		++m_eventQueueHead;
	}
	m_eventsData.Reset();
	m_eventQueueHead = m_eventQueueTail = 0;
}