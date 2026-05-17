#ifndef JADE_CORE_HEADER
#define JADE_CORE_HEADER

#include <atomic>
#include <future>
#include <memory>

namespace _jade {
	class FutureTaskTypeErased {
	public:
		virtual ~FutureTaskTypeErased() = default;
	};

	template <typename T>
	class FutureTaskTypeErasedImpl : public FutureTaskTypeErased {
	public:
		FutureTaskTypeErasedImpl(std::future<T>&& future) : future(std::move(future)) {}
		virtual ~FutureTaskTypeErasedImpl() = default;

	public:
		std::future<T> future;
	};
}

namespace jade {
	enum class TaskType {
		// Cancellable
		AsyncMusicLibraryAdd,

		AsyncCancellableCount,

		// Non-Cancellable
		AsyncMusicLibrarySave = AsyncCancellableCount,

		AsyncCount,
		None
	};

	enum TaskCategory {
		None = 0x0,

		Sync  = 0x1,
		Async = 0x2
	};

	enum class TaskModuleOrigin {
		UI,

		None
	};

	class Timestep {
	public:
		Timestep() = default;
		Timestep(double timestep) : m_timestep(timestep) {}

		inline bool operator==(const Timestep& other) noexcept {
			return std::fabs(m_timestep - other.m_timestep) < DBL_EPSILON;
		}

		inline bool operator<(const Timestep& other) noexcept { return m_timestep < other.m_timestep; }
		inline bool operator<=(const Timestep& other) noexcept { return m_timestep <= other.m_timestep; }
		inline bool operator>(const Timestep& other) noexcept { return m_timestep > other.m_timestep; }
		inline bool operator>=(const Timestep& other) noexcept { return m_timestep >= other.m_timestep; }

		inline Timestep& operator+=(const Timestep& other) noexcept {
			m_timestep += other.m_timestep;
			return *this;
		}

		inline Timestep& operator-=(const Timestep& other) noexcept {
			m_timestep -= other.m_timestep;
			return *this;
		}

	public:
		inline double Seconds() const noexcept { return m_timestep; }

	private:
		double m_timestep = 0.0;
	};

	class AsyncCancellationController {
	public:
		AsyncCancellationController() = default;

		inline void Cancel() { m_shouldCancel.store(true, std::memory_order_relaxed); }
		inline void Reset() { m_shouldCancel.store(false, std::memory_order_relaxed); }
		inline bool ShouldCancel() { return m_shouldCancel.load(std::memory_order_relaxed); }

	private:
		std::atomic<bool> m_shouldCancel{ false };
	};

	struct FutureTask {
	public:
		FutureTask() = default;

	public:
		inline void Wait() noexcept { m_future.reset(); }
		inline void Cancel() noexcept { m_controller.Cancel(); }
		inline bool ShouldCancel() noexcept { return m_controller.ShouldCancel(); }

		template <typename T>
		void SetTask(std::future<T>&& future) {
			Wait();
			m_future = std::make_unique<_jade::FutureTaskTypeErasedImpl<T>>(std::move(future));
		}

	private:
		AsyncCancellationController                  m_controller;
		std::unique_ptr<_jade::FutureTaskTypeErased> m_future;
	};
}

#endif // !JADE_CORE_HEADER