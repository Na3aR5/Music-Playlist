#ifndef MUSIC_PLAYLIST_CORE_HEADER
#define MUSIC_PLAYLIST_CORE_HEADER

#include <vector>
#include <string>
#include <random>
#include <filesystem>

namespace mspl {
	class Playlist;

	class PlaylistShuffled {
	public:
		class Iterator {
		public:
			Iterator(
				int current,
				Playlist* playlist,
				std::vector<int>* order
			);

		public:
			std::filesystem::directory_entry& operator*() noexcept {
				return *(m_playlist->begin() + (*m_order)[m_current]);
			}

			const std::filesystem::directory_entry& operator*() const noexcept {
				return *(m_playlist->begin() + (*m_order)[m_current]);
			}

			std::filesystem::directory_entry* operator->() noexcept {
				return (m_playlist->begin() + (*m_order)[m_current]).operator->();
			}

			std::filesystem::directory_entry* operator->() const noexcept {
				return (m_playlist->begin() + (*m_order)[m_current]).operator->();
			}

			Iterator& operator++() noexcept { ++m_current; return *this; }
			Iterator& operator--() noexcept { --m_current; return *this; }

			bool operator==(const Iterator& other) const noexcept {
				return m_current == other.m_current && m_playlist == other.m_playlist && m_order == other.m_order;
			}

			bool operator!=(const Iterator& other) const noexcept {
				return !(this->operator==(other));
			}

		private:
			int               m_current  = 0;
			Playlist*		  m_playlist = nullptr;
			std::vector<int>* m_order    = nullptr;
		};

	public:
		PlaylistShuffled(
			std::vector<int>&& order,
			Playlist& playlist) : m_playlist(&playlist), m_order(std::move(order)) {}

	public:
		Iterator begin() noexcept;
		Iterator end() noexcept;

	private:
		Playlist*  m_playlist;
		std::vector<int> m_order;
	};

	class Playlist {
	public:
		Playlist(
			const std::filesystem::path& directory,
			const std::vector<std::string>& fileExtensions
		);

	public:
		PlaylistShuffled GetShuffled() noexcept;

	public:
		std::vector<std::filesystem::directory_entry>::iterator begin() noexcept;
		std::vector<std::filesystem::directory_entry>::iterator end() noexcept;

	private:
		std::vector<std::filesystem::directory_entry> m_files;
		std::mt19937								  m_randomGenerator;
	};
}

#endif // !MUSIC_PLAYLIST_CORE_HEADER