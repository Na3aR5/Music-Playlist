#include <jade/backend/BackendConsole.h>
#include <jade/Platform.h>
#include <jade/App.h>

#include <map>
#include <string>
#include <vector>
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
		{ "playlist_create", jade::BackendConsole::Command::PlaylistCreate },
	};
}

jade::BackendConsole::BackendConsole() {
	for (size_t i = 0; i < (size_t)TaskType::AsyncCancellableCount; ++i) {
		m_asyncTaskCancellationCtrls[i] = std::make_shared<AsyncCancellationController>();
	}

	EventSystem::Get().Subscribe<OnKeyAction>(50, [this](const OnKeyAction& e) {
		m_taskQueue.emplace(Task{
			.type     = Task::Type::KeyAction,
			.category = TaskCategory::Sync,
			.payload  = Task::Payload(e)
		});
	});
	EventSystem::Get().Subscribe<OnTaskEnded>(50, [this](const OnTaskEnded& e) {
		if (e.category & TaskCategory::Async) {
			--m_workingTaskCount;
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
				for (auto& ctrl : m_asyncTaskCancellationCtrls) {
					ctrl->Cancel();
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
			m_commandBuffer += clipboardText;
			std::cout << clipboardText;
			return;
		}
		char keyChar = InputSystem::Get().KeyToChar(keyAction.key, keyAction.mods);
		if (keyChar != '\0') {
			m_commandBuffer += keyChar;
			std::cout << keyChar;
			return;
		}
		if (keyAction.key == Key::Backspace) {
			if (!m_commandBuffer.empty()) {
				m_commandBuffer.pop_back();
				std::cout << "\b \b";
			}
			return;
		}
		if (keyAction.key == Key::Enter) {
			m_taskQueue.emplace(Task{
				.type     = Task::Type::Execute,
				.category = TaskCategory::Sync,
				.payload  = Task::Payload(Task::ExecuteCmd{
					.cmd = std::move(m_commandBuffer)
				})
			});
			m_commandBuffer.clear();
			return;
		}
	}
}

void jade::BackendConsole::ExecuteCmdTask(const Task& task) {
	std::cout << '\n';
	ShowNewInput();
	DispatchCmdExecution(task);
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

	((*this).*m_dispatchCmdTable[(size_t)command])(tokens);
}

void jade::BackendConsole::ExecuteCloseCmd(std::vector<std::vector<std::string>>& tokens) {
	jade::Application::Get().CloseRequest();
}

void jade::BackendConsole::ExecuteCloseUnsavedCmd(std::vector<std::vector<std::string>>& tokens) {
	jade::Application::Get().CloseUnsavedRequest();
}

void jade::BackendConsole::ExecuteLibraryShowCmd(std::vector<std::vector<std::string>>& tokens) {
	auto trackIt    = jade::MusicLibrary::Get().TrackIteratorBegin();
	auto trackItEnd = jade::MusicLibrary::Get().TrackIteratorEnd();

	std::cout << '\n';
	for (; trackIt != trackItEnd; ++trackIt) {
		std::cout << "\t- ID " << trackIt->id << ": ";
		for (size_t i = 0; i < trackIt->artists.size(); ++i) {
			std::cout << trackIt->artists[i];
			if (i + 1 < trackIt->artists.size()) {
				std::cout << ", ";
			}
		}
		std::cout << " - " << trackIt->name;
		if (!trackIt->feat.empty()) {
			std::cout << " (feat ";
			for (size_t i = 0; i < trackIt->feat.size(); ++i) {
				std::cout << trackIt->feat[i];
				if (i + 1 < trackIt->feat.size()) {
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
	MusicLibrary::Get().SaveChanges();
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
	m_asyncTaskCancellationCtrls[(size_t)TaskType::AsyncMusicLibraryAdd]->Reset();
	++m_workingTaskCount;
	MusicLibrary::Get().Add(artists, feat, name, path,
		m_asyncTaskCancellationCtrls[(size_t)TaskType::AsyncMusicLibraryAdd]);
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