#include <jade/backend/BackendConsole.h>
#include <jade/Platform.h>
#include <jade/App.h>

#include <iostream>

namespace {
	void ClearConsoleLine() { std::cout << "\33[2K\r"; }
	jade::BackendConsole::Command GetCommandFromName(const std::string&);
	std::vector<std::vector<std::string>> Tokenize(const std::string&);

	void PlaylistCreateExecute(std::vector<std::vector<std::string>>& tokens);
}

namespace {
	const std::map<std::string, jade::BackendConsole::Command> g_CommandMap = {
		{ "close",		     jade::BackendConsole::Command::Close },
		{ "close_unsaved",   jade::BackendConsole::Command::CloseUnsaved },

		{ "lib_add",         jade::BackendConsole::Command::LibraryAdd },
		{ "lib_save",        jade::BackendConsole::Command::LibrarySave },
		{ "lib_show",        jade::BackendConsole::Command::LibraryShow },

		{ "play",            jade::BackendConsole::Command::Play },
		{ "pause",           jade::BackendConsole::Command::Pause },
		{ "resume",          jade::BackendConsole::Command::Resume },
		{ "volume",          jade::BackendConsole::Command::Volume },

		{ "playlist_create", jade::BackendConsole::Command::PlaylistCreate }
	};
}

jade::BackendConsole::BackendConsole() {
	for (size_t i = 0; i < (size_t)TaskType::AsyncCancellableCount; ++i) {
		m_futureTasks[i] = std::make_shared<FutureTask>();
	}
	m_musicLibrary.SetMusicLibrary(&MusicLibrary::Get());

	EventSystem::Get().Subscribe<OnKeyAction>(50, [this](const OnKeyAction& e) {
		m_taskQueue.emplace(Task{
			.type     = Task::Type::KeyAction,
			.category = TaskCategory::Sync,
			.payload  = Task::Payload(e)
		});
	});
	EventSystem::Get().Subscribe<OnTaskEnded>(50, [this](const OnTaskEnded& e) {
		if (e.status == OnTaskEnded::Status::Failed) {
			ShowError(e.errorMsg);
			return;
		}
		if (e.status == OnTaskEnded::Status::Success) {
			DispatchTaskResult(e);
			return;
		}
	});
	EventSystem::Get().Subscribe<OnAsyncTaskEnded>(50, [this](OnAsyncTaskEnded& e) {
		--m_workingTaskCount;
		if (e.task) {
			e.task->Wait();
		}
		if (e.status == OnTaskEnded::Status::Failed) {
			ShowError(e.errorMsg);
			return;
		}
		if (e.status == OnTaskEnded::Status::Success) {
			DispatchTaskResult(e);
			return;
		}
	});
	EventSystem::Get().Subscribe<OnApplicationClose>(50, [this](OnApplicationClose& e) {
		if (m_workingTaskCount > 0) {
			if (!(m_states & State::AllTasksCancelledBit)) {
				for (auto& task : m_futureTasks) {
					task->Cancel();
				}
				m_states |= State::AllTasksCancelledBit;
			}
			e.closeState = e.WaitForOthers;
			m_states |= State::ShouldTerminateBit;
		}
	});
}

void jade::BackendConsole::Update(Timestep deltaTime) {
	if (m_states & State::ShouldTerminateBit) {
		if (m_workingTaskCount == 0) m_states &= ~State::ShouldTerminateBit;
		return;
	}
	while (!m_taskQueue.empty()) {
		DispatchTask(m_taskQueue.front());
		m_taskQueue.pop();
	}
}

void jade::BackendConsole::Render() {
	if (m_states & State::ShouldShowNewInputBit) {
		ShowNewInput();
		m_states &= ~State::ShouldShowNewInputBit;
	}
}

void jade::BackendConsole::ShowNewInput() const {
	std::cout << "<Jade> ";
}

size_t jade::BackendConsole::ShowNewInputCharSize() const noexcept {
	return sizeof("<Jade> ") - 1;
}

void jade::BackendConsole::ShowError(const std::string& error) const {
	ClearConsoleLine();
	std::cout << "Error: " << error << '\n';
	ShowNewInput();
	std::cout << m_commandBuffer;
}

void jade::BackendConsole::DispatchTask(const Task& task) {
	((*this).*m_dispatchTaskTable[(size_t)task.type])(task);
}

void jade::BackendConsole::DispatchTaskResult(const OnTaskEnded& endedTask) const {
	ClearConsoleLine();

	ShowNewInput();
	std::cout << m_commandBuffer;
}

void jade::BackendConsole::DispatchTaskResult(const OnAsyncTaskEnded& endedTask) const {
	ClearConsoleLine();

	switch (endedTask.whatTask) {
		case TaskType::AsyncMusicLibrarySave:
			std::cout << "Music library changes have been successfully saved\n";
			break;

		case TaskType::AsyncMusicLibraryAdd:
			std::cout << "Track has been successfully added to music library\n";
			break;
	}
	ShowNewInput();
	std::cout << m_commandBuffer;
}

void jade::BackendConsole::KeyActionTask(const Task& task) {
	const OnKeyAction& keyAction = std::get<OnKeyAction>(task.payload);

	if (keyAction.pressed) {
		if (keyAction.key == Key::V && (bool)(keyAction.mods & KeyModifier::LCtrl)) {
			std::string clipboardText = GetClipboardTextContent();
			m_commandBuffer.insert(m_cursorPosition, clipboardText);

			RerenderCommandLineTask(Task{
				.type     = Task::Type::RerenderCommandLine,
				.category = TaskCategory::Sync,
				.payload  = Task::RerenderCommandLine{
					.prevSize = m_commandBuffer.size() - clipboardText.size()
				}
			});
			return;
		}

		char keyChar = InputSystem::Get().KeyToChar(keyAction.key, keyAction.mods);
		if (keyChar != '\0') {
			if (m_cursorPosition > m_commandBuffer.size()) {
				m_cursorPosition = m_commandBuffer.size();
			}
			m_commandBuffer.insert(m_cursorPosition, 1, keyChar);

			RerenderCommandLineTask(Task{
				.type = Task::Type::RerenderCommandLine,
				.category = TaskCategory::Sync,
				.payload = Task::RerenderCommandLine{
					.prevSize = m_commandBuffer.size() - 1
				}
			});
			return;
		}

		switch (keyAction.key) {
			case Key::Backspace:
				if (!m_commandBuffer.empty() && m_cursorPosition > 0) {
					m_commandBuffer.erase(m_cursorPosition - 1, 1);

					RerenderCommandLineTask(Task{
						.type = Task::Type::RerenderCommandLine,
						.category = TaskCategory::Sync,
						.payload = Task::RerenderCommandLine{
							.prevSize = m_commandBuffer.size() + 1
						}
					});
				}
				break;

			case Key::Enter:
				m_taskQueue.emplace(Task{
					.type = Task::Type::Execute,
					.category = TaskCategory::Sync,
					.payload = Task::Payload(Task::ExecuteCmd{
						.cmd = std::move(m_commandBuffer)
					})
				});
				m_commandBuffer.clear();
				break;

			case Key::Left:
				if (m_cursorPosition > 0) {
					--m_cursorPosition;
					std::cout << '\b';
				}
				break;

			case Key::Right:
				if (m_cursorPosition < m_commandBuffer.size()) {
					++m_cursorPosition;
					std::cout << "\033[1C";
				}
				break;
		}
	}
}

void jade::BackendConsole::ExecuteCmdTask(const Task& task) {
	std::cout << '\n';
	DispatchCmdExecution(task);
}

void jade::BackendConsole::RerenderCommandLineTask(const Task& task) {
	Task::RerenderCommandLine rerenderInfo = std::get<Task::RerenderCommandLine>(task.payload);

	if (m_commandBuffer.size() > rerenderInfo.prevSize) {
		size_t insertedSize = m_commandBuffer.size() - rerenderInfo.prevSize;

		std::cout << m_commandBuffer.data() + m_cursorPosition;
		m_cursorPosition += insertedSize;
	}
	else {
		size_t removedCount = rerenderInfo.prevSize - m_commandBuffer.size();

		std::cout << "\033[" << removedCount << 'D';
		std::cout << m_commandBuffer.data() + (m_cursorPosition - removedCount) << "\033[K";
		m_cursorPosition -= removedCount;
	}
	std::cout << "\r\033[" << ShowNewInputCharSize() + m_cursorPosition << 'C';
}

void jade::BackendConsole::DispatchCmdExecution(const Task& task) {
	const std::string& cmd = std::get<Task::ExecuteCmd>(task.payload).cmd;
	if (cmd.empty()) {
		return;
	}
	std::vector<std::vector<std::string>> tokens = Tokenize(cmd);
	if (!tokens.back().empty()) {
		ShowError(tokens.back().front());
		return;
	}
	tokens.pop_back();
	Command command = GetCommandFromName(tokens.front().front());

	if (command == Command::None) {
		std::cout << "Unknown command '" << tokens.front().front() << "'\n";
		m_states |= State::ShouldShowNewInputBit;
		return;
	}
	((*this).*m_dispatchCmdTable[(size_t)command])(tokens);
}

void jade::BackendConsole::ExecuteCloseCmd(std::vector<std::vector<std::string>>& tokens) {
	jade::Application::Get().CloseRequest();
}

void jade::BackendConsole::ExecuteCloseUnsavedCmd(std::vector<std::vector<std::string>>& tokens) {
	jade::Application::Get().CloseUnsavedRequest();
}

void jade::BackendConsole::ExecuteLibraryShowCmd(std::vector<std::vector<std::string>>& tokens) {
	auto trackIt    = m_musicLibrary.TrackIteratorBegin();
	auto trackItEnd = m_musicLibrary.TrackIteratorEnd();

	if (trackIt == trackItEnd) {
		std::cout << "Music library is empty\n";
		m_states |= State::ShouldShowNewInputBit;
		return;
	}

	for (; trackIt != trackItEnd; ++trackIt) {
		std::cout << "\t- ID " << trackIt->second.id << ": ";
		for (size_t i = 0; i < trackIt->second.artists.size(); ++i) {
			std::cout << trackIt->second.artists[i];
			if (i + 1 < trackIt->second.artists.size()) {
				std::cout << ", ";
			}
		}
		std::cout << " - " << trackIt->second.name;
		if (!trackIt->second.feat.empty()) {
			std::cout << " (feat ";
			for (size_t i = 0; i < trackIt->second.feat.size(); ++i) {
				std::cout << trackIt->second.feat[i];
				if (i + 1 < trackIt->second.feat.size()) {
					std::cout << ", ";
				}
			}
			std::cout << ')';
		}
		std::cout << '\n';
	}
	m_states |= State::ShouldShowNewInputBit;
}

void jade::BackendConsole::ExecuteLibrarySaveCmd(std::vector<std::vector<std::string>>& tokens) {
	++m_workingTaskCount;
	m_musicLibrary.SaveChanges();
}

void jade::BackendConsole::ExecuteLibraryAddCmd(std::vector<std::vector<std::string>>& tokens) {
	std::vector<std::string> artists;
	std::vector<std::string> feat;
	std::string name;
	std::string path;

	for (size_t i = 1; i < tokens.size(); ++i) {
		std::vector<std::string>& pack = tokens[i];
		if (std::strcmp("artists:", pack.front().c_str()) == 0) {
			for (size_t i = 1; i < pack.size(); ++i) {
				artists.emplace_back(std::move(pack[i]));
			}
		}
		else if (std::strcmp("feat:", pack.front().c_str()) == 0) {
			for (size_t i = 1; i < pack.size(); ++i) {
				feat.emplace_back(std::move(pack[i]));
			}
		}
		else if (std::strcmp("name:", pack.front().c_str()) == 0) {
			name = std::move(pack[1]);
		}
		else if (std::strcmp("path:", pack.front().c_str()) == 0) {
			path = std::move(pack[1]);
		}
		else {
			ShowError(std::string("Unknown parameter pack '") + pack.front() + '\'');
			return;
		}
	}
	std::shared_ptr<jade::FutureTask>& currFutureTask = m_futureTasks[(size_t)jade::TaskType::AsyncMusicLibraryAdd];

	currFutureTask->Wait();
	++m_workingTaskCount;
	currFutureTask->SetTask(m_musicLibrary.Add(artists, feat, name, path, currFutureTask));
}

void jade::BackendConsole::ExecutePlayCmd(std::vector<std::vector<std::string>>& tokens) {
	uint64_t id = std::atoi(tokens[1][1].c_str());
	MusicLibrary::TrackIterator track = m_musicLibrary.GetTrackByID(id);

	if (track == m_musicLibrary.TrackIteratorEnd()) {
		std::cout << "No track found with ID = " << id << '\n';
	}
	else {
		Application::Get().Player().Play(track->second);
	}
	m_states |= State::ShouldShowNewInputBit;
}

void jade::BackendConsole::ExecutePauseCmd(std::vector<std::vector<std::string>>& tokens) {
	Application::Get().Player().Pause();
	m_states |= State::ShouldShowNewInputBit;
}

void jade::BackendConsole::ExecuteResumeCmd(std::vector<std::vector<std::string>>& tokens) {
	Application::Get().Player().Resume();
	m_states |= State::ShouldShowNewInputBit;
}

void jade::BackendConsole::ExecuteVolumeCmd(std::vector<std::vector<std::string>>& tokens) {
	if (strcmp("%:", tokens[1][1].c_str())) {
		float volume;
		try {
			volume = std::stof(tokens[1][1]);
		}
		catch (const std::invalid_argument&) {
			std::cout << "Invalid percentage number format\n";
			m_states |= State::ShouldShowNewInputBit;
			return;
		}
		Application::Get().Player().SetVolume(volume * 0.01f);

		std::cout << "Player sound volume has been set to " << volume << "%\n";
		m_states |= State::ShouldShowNewInputBit;
	}
}

namespace {
	jade::BackendConsole::Command GetCommandFromName(const std::string& name) {
		auto it = g_CommandMap.find(name);
		if (it == g_CommandMap.cend()) {
			return jade::BackendConsole::Command::None;
		}
		return it->second;
	}

	std::vector<std::vector<std::string>> Tokenize(const std::string& str) {
		std::vector<std::vector<std::string>> tokens;

		size_t i = 0;
		size_t len = str.length();

		while (i < len && std::isspace(str[i])) { ++i; }
		{
			std::string commandToken;
			while (i < len && !std::isspace(str[i])) {
				commandToken += str[i++];
			}
			tokens.emplace_back();
			tokens.back().emplace_back(std::move(commandToken));
		}

		while (i < len) {
			while (i < len && std::isspace(str[i])) { ++i; }

			std::string token;
			while (i < len && !std::isspace(str[i])) {
				token += str[i++];
			}
			if (token.back() != ':' && tokens.back().front().back() != ':') {
				tokens.emplace_back();
				tokens.back().emplace_back("Parameter pack name was not specified");
				return tokens;
			}
			tokens.emplace_back();
			tokens.back().emplace_back(std::move(token));

			while (i < len) {
				while (i < len && std::isspace(str[i])) { ++i; }

				size_t pos = i;
				while (i < len && str[i] != ',' && str[i] != ';') { ++i; }

				tokens.back().emplace_back(str.substr(pos, i - pos));
				if (str[i] == ';') {
					++i;
					break;
				}
				++i;
			}
		}
		tokens.emplace_back();
		return tokens;
	}

	void PlaylistCreateExecute(std::vector<std::vector<std::string>>& tokens) {
		std::string name;
		std::vector<uint64_t> ids;

		// todo
	}
}