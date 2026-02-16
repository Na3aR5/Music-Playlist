#include <musicplaylist/Playlist.h>
#include <thread>

#include <set>

namespace {
	std::mt19937 g_RandomGenerator{ std::random_device()() };
}

// ---------------------------------------------------- PlaylistElement ----------------------------------------------------

mspl::PlaylistElement::PlaylistElement(const std::filesystem::directory_entry& entry) :
	m_directoryEntry(entry), m_durationSeconds(1.0) {
}

const std::filesystem::path& mspl::PlaylistElement::GetPath() const noexcept {
	return m_directoryEntry.path();
}

double mspl::PlaylistElement::GetDurationSeconds() const noexcept {
	return m_durationSeconds;
}

// ----------------------------------------------- PlaylistShuffled::Iterator ----------------------------------------------

mspl::PlaylistShuffled::Iterator::Iterator(int current, Playlist* playlist, std::vector<size_t>* order) :
	m_current(current), m_playlist(playlist), m_order(order) {
}

mspl::PlaylistElement& mspl::PlaylistShuffled::Iterator::operator*() noexcept {
	return *(m_playlist->begin() + (*m_order)[m_current]);
}

const mspl::PlaylistElement& mspl::PlaylistShuffled::Iterator::operator*() const noexcept {
	return *(m_playlist->begin() + (*m_order)[m_current]);
}

mspl::PlaylistElement* mspl::PlaylistShuffled::Iterator::operator->() noexcept {
	return (m_playlist->begin() + (*m_order)[m_current]).operator->();
}

const mspl::PlaylistElement* mspl::PlaylistShuffled::Iterator::operator->() const noexcept {
	return (m_playlist->begin() + (*m_order)[m_current]).operator->();
}

mspl::PlaylistShuffled::Iterator& mspl::PlaylistShuffled::Iterator::operator++() noexcept {
	++m_current;
	return *this;
}

mspl::PlaylistShuffled::Iterator& mspl::PlaylistShuffled::Iterator::operator--() noexcept {
	++m_current;
	return *this;
}

bool mspl::PlaylistShuffled::Iterator::operator==(const Iterator& other) const noexcept {
	return m_current == other.m_current && m_playlist == other.m_playlist && m_order == other.m_order;
}

bool mspl::PlaylistShuffled::Iterator::operator!=(const Iterator& other) const noexcept {
	return !(m_current == other.m_current && m_playlist == other.m_playlist && m_order == other.m_order);
}

mspl::PlaylistShuffled::Iterator& mspl::PlaylistShuffled::Iterator::Next() noexcept {
	++m_current;
	return *this;
}

bool mspl::PlaylistShuffled::Iterator::HasNext() const noexcept {
	return static_cast<size_t>(m_current + 1) < m_order->size();
}

// ---------------------------------------------------- PlaylistShuffled ---------------------------------------------------

mspl::PlaylistShuffled::Iterator mspl::PlaylistShuffled::begin() noexcept {
	return Iterator(0, m_playlist, &m_order);
}

mspl::PlaylistShuffled::Iterator mspl::PlaylistShuffled::end() noexcept {
	return Iterator(static_cast<int>(m_order.size()), m_playlist, &m_order);
}

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

mspl::PlaylistOrderGenerator::GenerationStrategy_Random::GenerationStrategy_Random(size_t listSize) {
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
	size_t index = dist(g_RandomGenerator);
	size_t result = m_order[index];

	std::swap(m_order.back(), m_order[index]);
	m_order.pop_back();

	return result;
}

bool mspl::PlaylistOrderGenerator::GenerationStrategy_Random::HasNext() const noexcept {
	return !m_order.empty();
}

// ---------------------------------------------- GenerationStrategy_RandomCycle -------------------------------------------

mspl::PlaylistOrderGenerator::GenerationStrategy_RandomCycle::GenerationStrategy_RandomCycle(size_t listSize) {
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
		size_t index = dist(g_RandomGenerator);
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

mspl::PlaylistOrderGenerator::GenerationStrategy_RandomCycleRegenerate::GenerationStrategy_RandomCycleRegenerate(size_t listSize) :
	m_listSize(listSize) {
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
	size_t index = dist(g_RandomGenerator);
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

mspl::PlaylistOrderGenerator::PlaylistOrderGenerator(Playlist* playlist, GenerationStrategy strategy) :
	m_playlist(playlist) {
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

// --------------------------------------------------------- Playlist -------------------------------------------------------

mspl::Playlist::Playlist(const std::filesystem::path& directory, const std::vector<std::string>& fileExtensions) {
	std::set<std::string> extensionSet(fileExtensions.begin(), fileExtensions.end());

	for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(directory)) {
		if (entry.is_regular_file() && extensionSet.find(entry.path().extension().string()) != extensionSet.cend()) {
			m_elements.emplace_back(entry);
		}
	}
}

size_t mspl::Playlist::GetSize() const noexcept {
	return m_elements.size();
}

mspl::PlaylistShuffled mspl::Playlist::GetShuffled() {
	std::vector<size_t> originOrder;

	originOrder.reserve(m_elements.size());
	for (size_t i = 0; i < m_elements.size(); ++i) {
		originOrder.emplace_back(i);
	}
	std::uniform_int_distribution<size_t> dist;

	std::vector<size_t> shuffledOrder;
	while (!originOrder.empty()) {
		dist.param(std::uniform_int_distribution<size_t>::param_type(0, originOrder.size() - 1));

		size_t index = dist(g_RandomGenerator);
		shuffledOrder.emplace_back(originOrder[index]);

		std::swap(originOrder.back(), originOrder[index]);
		originOrder.pop_back();
	}

	return PlaylistShuffled(this, std::move(shuffledOrder));
}

mspl::PlaylistOrderGenerator mspl::Playlist::GetOrderGenerator(PlaylistOrderGenerator::GenerationStrategy strategy) {
	return PlaylistOrderGenerator(this, strategy);
}

void mspl::Playlist::PlayFor(double seconds, PlayStrategy strategy) {
	PlaylistOrderGenerator generator(this, strategy);

	auto start = std::chrono::steady_clock::now();
	while (generator.HasNext()) {
		auto end = std::chrono::steady_clock::now();
		double duration = std::chrono::duration<double>(end - start).count();

		const PlaylistElement& elem = generator.GenerateNext();

		if (duration + elem.GetDurationSeconds() >= seconds) {
			break;
		}
		_PlayElement(elem);

		std::this_thread::sleep_for(std::chrono::duration<double>(elem.GetDurationSeconds()));
	}
}

std::vector<mspl::PlaylistElement>::iterator mspl::Playlist::begin() noexcept {
	return m_elements.begin();
}

std::vector<mspl::PlaylistElement>::iterator mspl::Playlist::end() noexcept {
	return m_elements.end();
}

#include <iostream>
void mspl::Playlist::_PlayElement(const PlaylistElement& elem) noexcept {
	std::cout << elem.GetPath() << '\n';
}