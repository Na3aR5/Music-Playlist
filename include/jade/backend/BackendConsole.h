#ifndef JADE_BACKEND_CONSOLE_HEADER
#define JADE_BACKEND_CONSOLE_HEADER

#include <jade/Core.h>
#include <jade/Event.h>
#include <jade/backend/Backend.h>

#include <queue>
#include <vector>
#include <string>
#include <memory>
#include <variant>

namespace jade {
	class BackendConsole : public IBackend {
	public:
		enum State : uint64_t {
			ShouldShowNewInputBit = 0x1,
			ShouldTerminateBit    = 0x2,
			AllTasksCancelledBit  = 0x4,
		};

		enum class Command {
			Close,
			CloseUnsaved,

			LibraryShow,
			LibrarySave,
			LibraryAdd,

			Play,
			Pause,

			PlaylistCreate,

			None
		};

		struct Task {
		public:
			enum class Type {
				KeyAction,
				Execute,

				None
			};

		public:
			struct ExecuteCmd {
				std::string cmd;
			};

			using Payload = std::variant<
				OnKeyAction,
				ExecuteCmd
			>;

		public:
			Type	     type     = Type::None;
			TaskCategory category = TaskCategory::None;
			Payload      payload  = {};
		};

	public:
		BackendConsole();

	public:
		virtual void Update(Timestep) override;
		virtual void Render() override;

	public:
		void ShowNewInput() const;
		void ShowError(const std::string&) const;

	public:
		void DispatchTask(const Task& task);
		void DispatchTaskResult(const OnTaskEnded& endedTask) const;

		void KeyActionTask(const Task& task);
		void ExecuteCmdTask(const Task& task);

		void DispatchCmdExecution(const Task& task);

		void ExecuteCloseCmd(std::vector<std::vector<std::string>>&);
		void ExecuteCloseUnsavedCmd(std::vector<std::vector<std::string>>&);
		void ExecuteLibraryShowCmd(std::vector<std::vector<std::string>>&);
		void ExecuteLibrarySaveCmd(std::vector<std::vector<std::string>>&);
		void ExecuteLibraryAddCmd(std::vector<std::vector<std::string>>&);
		void ExecutePlayCmd(std::vector<std::vector<std::string>>&);
		void ExecutePauseCmd(std::vector<std::vector<std::string>>&);

	private:
		uint64_t         m_states = (uint64_t)State::ShouldShowNewInputBit;
		std::string      m_commandBuffer;
		std::queue<Task> m_taskQueue;
		
		size_t m_workingTaskCount = 0;
		std::shared_ptr<FutureTask> m_futureTasks[(size_t)TaskType::AsyncCancellableCount] = {};

		std::vector<void(BackendConsole::*)(const Task&)> m_dispatchTaskTable = {
			&BackendConsole::KeyActionTask,
			&BackendConsole::ExecuteCmdTask,
		};

		std::vector<void(BackendConsole::*)(std::vector<std::vector<std::string>>&)> m_dispatchCmdTable = {
			&BackendConsole::ExecuteCloseCmd,
			&BackendConsole::ExecuteCloseUnsavedCmd,
			&BackendConsole::ExecuteLibraryShowCmd,
			&BackendConsole::ExecuteLibrarySaveCmd,
			&BackendConsole::ExecuteLibraryAddCmd,
			&BackendConsole::ExecutePlayCmd,
			&BackendConsole::ExecutePauseCmd,
		};
	};
}

#endif // !JADE_BACKEND_CONSOLE_HEADER