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
	return PlaylistOrderGenerator(this, strategy, &g_RandomGenerator);
}

void mspl::Playlist::PlayFor(double seconds, PlayStrategy strategy) {
	PlaylistOrderGenerator generator(this, strategy, &g_RandomGenerator);

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