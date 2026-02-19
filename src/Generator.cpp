#include <musicplaylist/Generator.h>
#include <musicplaylist/Playlist.h>

// ------------------------------------------------ GenerationStrategy_Ordered ---------------------------------------------

mspl::PlaylistOrderGenerator::GenerationStrategy_Ordered::GenerationStrategy_Ordered(size_t listSize) :
	m_listSize(listSize) {
}

size_t mspl::PlaylistOrderGenerator::GenerationStrategy_Ordered::GenerateNext() noexcept {
	return m_current++;
}

bool mspl::PlaylistOrderGenerator::GenerationStrategy_Ordered::HasNext() const noexcept {
	return m_current < m_listSize;
}

// ---------------------------------------------- GenerationStrategy_OrderedCycle ------------------------------------------

mspl::PlaylistOrderGenerator::GenerationStrategy_OrderedCycle::GenerationStrategy_OrderedCycle(size_t listSize) :
	m_listSize(listSize) {
}

size_t mspl::PlaylistOrderGenerator::GenerationStrategy_OrderedCycle::GenerateNext() noexcept {
	size_t result = m_current;
	m_current = (m_current + 1) % m_listSize;
	return result;
}

bool mspl::PlaylistOrderGenerator::GenerationStrategy_OrderedCycle::HasNext() const noexcept {
	return true;
}

// ------------------------------------------------ GenerationStrategy_Random ----------------------------------------------

mspl::PlaylistOrderGenerator::GenerationStrategy_Random::GenerationStrategy_Random(size_t listSize, std::mt19937* generator) :
m_generator(generator) {
	m_order.reserve(listSize);
	for (size_t i = 0; i < listSize; ++i) {
		m_order.emplace_back(i);
	}
}

size_t mspl::PlaylistOrderGenerator::GenerationStrategy_Random::GenerateNext() noexcept {
	if (m_order.size() == 1) {
		size_t result = m_order[0];
		m_order.pop_back();
		return result;
	}

	std::uniform_int_distribution<size_t> dist(0, m_order.size() - 1);
	size_t index = dist(*m_generator);
	size_t result = m_order[index];

	std::swap(m_order.back(), m_order[index]);
	m_order.pop_back();

	return result;
}

bool mspl::PlaylistOrderGenerator::GenerationStrategy_Random::HasNext() const noexcept {
	return !m_order.empty();
}

// ---------------------------------------------- GenerationStrategy_RandomCycle -------------------------------------------

mspl::PlaylistOrderGenerator::GenerationStrategy_RandomCycle::GenerationStrategy_RandomCycle(size_t listSize, std::mt19937* generator) :
m_generator(generator) {
	m_order.reserve(listSize);
	for (size_t i = 0; i < listSize; ++i) {
		m_order.emplace_back(i);
	}
}

size_t mspl::PlaylistOrderGenerator::GenerationStrategy_RandomCycle::GenerateNext() noexcept {
	if (m_order.size() == 1) {
		size_t result = m_order[0];
		m_generatedOrder.emplace_back(result);
		m_order.pop_back();
		return result;
	}
	if (!m_order.empty()) {
		std::uniform_int_distribution<size_t> dist(0, m_order.size() - 1);
		size_t index = dist(*m_generator);
		size_t result = m_order[index];

		m_generatedOrder.emplace_back(result);
		std::swap(m_order.back(), m_order[index]);
		m_order.pop_back();

		return result;
	}
	size_t current = m_current;
	m_current = (m_current + 1) % m_generatedOrder.size();
	return m_generatedOrder[current];
}

bool mspl::PlaylistOrderGenerator::GenerationStrategy_RandomCycle::HasNext() const noexcept {
	return true;
}

// ----------------------------------------- GenerationStrategy_RandomCycleRegenerate --------------------------------------

mspl::PlaylistOrderGenerator::GenerationStrategy_RandomCycleRegenerate::GenerationStrategy_RandomCycleRegenerate(
size_t listSize, std::mt19937* generator) : m_listSize(listSize), m_generator(generator) {
	m_order.reserve(m_listSize);
	for (size_t i = 0; i < m_listSize; ++i) {
		m_order.emplace_back(i);
	}
}

size_t mspl::PlaylistOrderGenerator::GenerationStrategy_RandomCycleRegenerate::GenerateNext() noexcept {
	if (m_order.size() == 1) {
		size_t result = m_order[0];
		m_generatedOrder.emplace_back(result);
		m_order.pop_back();

		// reset
		for (size_t i = 0; i < m_listSize; ++i) {
			m_order.emplace_back(i);
		}
		return result;
	}
	std::uniform_int_distribution<size_t> dist(0, m_order.size() - 1);
	size_t index = dist(*m_generator);
	size_t result = m_order[index];

	m_generatedOrder.emplace_back(result);
	std::swap(m_order.back(), m_order[index]);
	m_order.pop_back();

	return result;
}

bool mspl::PlaylistOrderGenerator::GenerationStrategy_RandomCycleRegenerate::HasNext() const noexcept {
	return true;
}

// -------------------------------------------------- PlaylistOrderGenerator ------------------------------------------------

mspl::PlaylistOrderGenerator::PlaylistOrderGenerator(Playlist* playlist, GenerationStrategy strategy, std::mt19937* generator) :
m_playlist(playlist), m_randomGenerator(generator) {
	switch (strategy) {
	case GenerationStrategy::ORDERED:
		m_generationStrategy = std::make_unique<GenerationStrategy_Ordered>(m_playlist->GetSize());
		break;

	case GenerationStrategy::ORDERED_CYCLE:
		m_generationStrategy = std::make_unique<GenerationStrategy_OrderedCycle>(m_playlist->GetSize());
		break;

	case GenerationStrategy::RANDOM:
		m_generationStrategy = std::make_unique<GenerationStrategy_Random>(m_playlist->GetSize());
		break;

	case GenerationStrategy::RANDOM_CYCLE:
		m_generationStrategy = std::make_unique<GenerationStrategy_RandomCycle>(m_playlist->GetSize());
		break;

	case GenerationStrategy::RANDOM_CYCLE_REGENERATE:
		m_generationStrategy = std::make_unique<GenerationStrategy_RandomCycleRegenerate>(m_playlist->GetSize());
		break;
	}
}

mspl::PlaylistElement& mspl::PlaylistOrderGenerator::GenerateNext() noexcept {
	return *(m_playlist->begin() + m_generationStrategy->GenerateNext());
}

bool mspl::PlaylistOrderGenerator::HasNext() const noexcept {
	return m_generationStrategy->HasNext();
}