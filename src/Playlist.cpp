#include <musicplaylist/Playlist.h>

#include <set>
#include <list>

mspl::PlaylistShuffled::Iterator::Iterator(int current, Playlist* playlist, std::vector<int>* order) :
m_current(current), m_playlist(playlist), m_order(order) {}

std::filesystem::directory_entry& mspl::PlaylistShuffled::Iterator::operator*() noexcept {
	return *(m_playlist->begin() + (*m_order)[m_current]);
}

std::filesystem::directory_entry* mspl::PlaylistShuffled::Iterator::operator->() noexcept {
	return (m_playlist->begin() + (*m_order)[m_current]).operator->();
}

const std::filesystem::directory_entry* mspl::PlaylistShuffled::Iterator::operator->() const noexcept {
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

const std::filesystem::directory_entry& mspl::PlaylistShuffled::Iterator::operator*() const noexcept {
	return *(m_playlist->begin() + (*m_order)[m_current]);
}

mspl::PlaylistShuffled::Iterator mspl::PlaylistShuffled::begin() noexcept {
	return Iterator(0, m_playlist, &m_order);
}

mspl::PlaylistShuffled::Iterator mspl::PlaylistShuffled::end() noexcept {
	return Iterator(m_order.size(), m_playlist, &m_order);
}

mspl::Playlist::Playlist(const std::filesystem::path& directory, const std::vector<std::string>& fileExtensions) :
m_randomGenerator(std::random_device()()) {
	std::set<std::string> extensionSet(fileExtensions.begin(), fileExtensions.end());

	for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(directory)) {
		if (entry.is_regular_file() && extensionSet.find(entry.path().extension().string()) != extensionSet.cend()) {
			m_files.emplace_back(entry);
		}
	}
}

mspl::PlaylistShuffled mspl::Playlist::GetShuffled() noexcept {
	std::vector<int> originOrder;

	for (int i = 0; i < m_files.size(); ++i) {
		originOrder.emplace_back(i);
	}
	std::uniform_int_distribution<int> dist(0, originOrder.size());

	std::vector<int> shuffledOrder;
	while (!originOrder.empty()) {
		int index = dist(m_randomGenerator);
		shuffledOrder.emplace_back(index);
		std::swap(originOrder.back(), originOrder[index]);
		originOrder.pop_back();
		dist.param(std::uniform_int_distribution<int>::param_type(0, originOrder.size()));
	}

	return PlaylistShuffled(std::move(shuffledOrder), *this);
}

std::vector<std::filesystem::directory_entry>::iterator mspl::Playlist::begin() noexcept {
	return m_files.begin();
}

std::vector<std::filesystem::directory_entry>::iterator mspl::Playlist::end() noexcept {
	return m_files.end();
}