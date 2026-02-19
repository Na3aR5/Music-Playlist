#ifndef MUSIC_PLAYLIST_CORE_HEADER
#define MUSIC_PLAYLIST_CORE_HEADER

#include <musicplaylist/Generator.h>

#include <vector>
#include <string>
#include <random>
#include <memory>
#include <filesystem>

namespace mspl {
	class PlaylistElement {
	public:
		PlaylistElement(const std::filesystem::directory_entry& entry);

	public:
		const std::filesystem::path& GetPath() const noexcept;
		double GetDurationSeconds() const noexcept;

	private:
		double                           m_durationSeconds;
		std::filesystem::directory_entry m_directoryEntry;
	};

	// pre-shuffled order
	class PlaylistShuffled {
	public:
		class Iterator {
		public:
			Iterator(
				int current,
				Playlist* playlist,
				std::vector<size_t>* order
			);

		public:
			PlaylistElement& operator*() noexcept;
			const PlaylistElement& operator*() const noexcept;

			PlaylistElement* operator->() noexcept;
			const PlaylistElement* operator->() const noexcept;

			Iterator& operator++() noexcept;
			Iterator& operator--() noexcept;

			bool operator==(const Iterator& other) const noexcept;
			bool operator!=(const Iterator& other) const noexcept;

		public:
			Iterator& Next() noexcept;
			bool HasNext() const noexcept;

		private:
			size_t			     m_current  = 0;
			Playlist*			 m_playlist = nullptr;
			std::vector<size_t>* m_order    = nullptr;
		};

	public:
		PlaylistShuffled(
			Playlist* playlist,
			std::vector<size_t>&& order
		) : m_playlist(playlist), m_order(std::move(order)) {}

	public:
		Iterator begin() noexcept;
		Iterator end() noexcept;

	private:
		Playlist* m_playlist;
		std::vector<size_t> m_order;
	};

	// set of files
	class Playlist {
	public:
		using PlayStrategy = PlaylistOrderGenerator::GenerationStrategy;

	public:
		Playlist(
			const std::filesystem::path& directory,
			const std::vector<std::string>& fileExtensions
		);

	public:
		size_t GetSize() const noexcept;

		PlaylistShuffled GetShuffled();
		PlaylistOrderGenerator GetOrderGenerator(PlaylistOrderGenerator::GenerationStrategy strategy);

	public:
		void PlayFor(double seconds, PlayStrategy strategy);

	public:
		std::vector<PlaylistElement>::iterator begin() noexcept;
		std::vector<PlaylistElement>::iterator end() noexcept;

	private:
		void _PlayElement(const PlaylistElement& elem) noexcept;

	private:
		std::vector<PlaylistElement> m_elements;
	};
}

#endif // !MUSIC_PLAYLIST_CORE_HEADER