#ifndef MUSIC_PLAYLIST_CORE_HEADER
#define MUSIC_PLAYLIST_CORE_HEADER

#include <vector>
#include <string>
#include <random>
#include <memory>
#include <filesystem>

namespace mspl {
	class Playlist;

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
			size_t			     m_current = 0;
			Playlist* m_playlist = nullptr;
			std::vector<size_t>* m_order = nullptr;
		};

	public:
		PlaylistShuffled(
			Playlist* playlist,
			std::vector<size_t>&& order
		) : m_playlist(playlist), m_order(std::move(order)) {
		}

	public:
		Iterator begin() noexcept;
		Iterator end() noexcept;

	private:
		Playlist* m_playlist;
		std::vector<size_t> m_order;
	};

	// instead of using pre-generated order, generates next file on GenerateNext()
	class PlaylistOrderGenerator {
	public:
		enum class GenerationStrategy {
			ORDERED,
			ORDERED_CYCLE,
			RANDOM,
			RANDOM_CYCLE,
			RANDOM_CYCLE_REGENERATE,
		};

		class IGenerationStrategy {
		public:
			virtual ~IGenerationStrategy() = default;

			virtual size_t GenerateNext() noexcept = 0;
			virtual bool HasNext() const noexcept = 0;
		};

		class GenerationStrategy_Ordered : public IGenerationStrategy {
		public:
			GenerationStrategy_Ordered(size_t listSize);

			virtual size_t GenerateNext() noexcept override;
			virtual bool HasNext() const noexcept override;

		private:
			size_t m_current = 0;
			size_t m_listSize;
		};

		class GenerationStrategy_OrderedCycle : public IGenerationStrategy {
		public:
			GenerationStrategy_OrderedCycle(size_t listSize);

			virtual size_t GenerateNext() noexcept override;
			virtual bool HasNext() const noexcept override;

		private:
			size_t m_current = 0;
			size_t m_listSize;
		};

		class GenerationStrategy_Random : public IGenerationStrategy {
		public:
			GenerationStrategy_Random(size_t listSize);

			virtual size_t GenerateNext() noexcept override;
			virtual bool HasNext() const noexcept override;

		private:
			std::vector<size_t> m_order;
		};

		class GenerationStrategy_RandomCycle : public IGenerationStrategy {
		public:
			GenerationStrategy_RandomCycle(size_t listSize);

			virtual size_t GenerateNext() noexcept override;
			virtual bool HasNext() const noexcept override;

		private:
			size_t              m_current = 0;
			std::vector<size_t> m_generatedOrder;
			std::vector<size_t> m_order;
		};

		class GenerationStrategy_RandomCycleRegenerate : public IGenerationStrategy {
		public:
			GenerationStrategy_RandomCycleRegenerate(size_t listSize);

			virtual size_t GenerateNext() noexcept override;
			virtual bool HasNext() const noexcept override;

		private:
			size_t              m_current = 0;
			size_t              m_listSize;
			std::vector<size_t> m_generatedOrder;
			std::vector<size_t> m_order;
		};

	public:
		PlaylistOrderGenerator(Playlist* playlist, GenerationStrategy strategy);

		PlaylistElement& GenerateNext() noexcept;
		bool HasNext() const noexcept;

	private:
		std::unique_ptr<IGenerationStrategy> m_generationStrategy;
		Playlist* m_playlist;
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