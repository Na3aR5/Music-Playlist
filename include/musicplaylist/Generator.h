#ifndef MUSIC_PLAYLIST_GENERATOR_HEADER
#define MUSIC_PLAYLIST_GENERATOR_HEADER

#include <memory>
#include <vector>
#include <random>

namespace mspl {
	class PlaylistElement;
	class Playlist;

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
			GenerationStrategy_Random(size_t listSize, std::mt19937* generator);

			virtual size_t GenerateNext() noexcept override;
			virtual bool HasNext() const noexcept override;

		private:
			std::mt19937*		m_generator;
			std::vector<size_t> m_order;
		};

		class GenerationStrategy_RandomCycle : public IGenerationStrategy {
		public:
			GenerationStrategy_RandomCycle(size_t listSize, std::mt19937* generator);

			virtual size_t GenerateNext() noexcept override;
			virtual bool HasNext() const noexcept override;

		private:
			std::mt19937*		m_generator;
			size_t              m_current = 0;
			std::vector<size_t> m_generatedOrder;
			std::vector<size_t> m_order;
		};

		class GenerationStrategy_RandomCycleRegenerate : public IGenerationStrategy {
		public:
			GenerationStrategy_RandomCycleRegenerate(size_t listSize, std::mt19937* generator);

			virtual size_t GenerateNext() noexcept override;
			virtual bool HasNext() const noexcept override;

		private:
			std::mt19937*		m_generator;
			size_t              m_current = 0;
			size_t              m_listSize;
			std::vector<size_t> m_generatedOrder;
			std::vector<size_t> m_order;
		};

	public:
		PlaylistOrderGenerator(Playlist* playlist, GenerationStrategy strategy, std::mt19937* generator);

		PlaylistElement& GenerateNext() noexcept;
		bool HasNext() const noexcept;

	private:
		std::unique_ptr<IGenerationStrategy> m_generationStrategy;
		Playlist* m_playlist;
		std::mt19937* m_randomGenerator;
	};
}

#endif // !MUSIC_PLAYLIST_GENERATOR_HEADER