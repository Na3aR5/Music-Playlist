#include <jade/backend/BackendConsole.h>
#include <jade/App.h>

#include <map>
#include <string>
#include <vector>
#include <iostream>

enum class Command {
	NoCmd,

	Close,
	CloseUnsaved,

	GlobalAdd,
	GlobalSave,
	GlobalShow,

	PlaylistCreate
};

namespace {
	void ShowInputRow();
	void ShowError(const std::string& error);
	Command GetCommandFromName(const std::string&);
	std::vector<std::vector<std::string>> Tokenize(const std::string&);

	void GlobalShowExecute(std::vector<std::vector<std::string>>& tokens);
	void GlobalAddExecute(std::vector<std::vector<std::string>>& tokens);
	void PlaylistCreateExecute(std::vector<std::vector<std::string>>& tokens);
}

namespace {
	const std::map<std::string, Command> g_CommandMap = {
		{ "close",		     Command::Close },
		{ "close_unsaved",   Command::CloseUnsaved },
		{ "global_add",      Command::GlobalAdd },
		{ "global_save",     Command::GlobalSave },
		{ "global_show",     Command::GlobalShow },
		{ "playlist_create", Command::PlaylistCreate },
	};

	using ExecuteCommandFunction = void(*)(std::vector<std::vector<std::string>>&);
	const ExecuteCommandFunction g_ExecuteCommandFunctions[] = {
		[](std::vector<std::vector<std::string>>& tokens) { // NoCmd
			std::cout << " - Unknown command '" << tokens.front().front() << "'!\n";
		},
		[](std::vector<std::vector<std::string>>& tokens) { // Close
			jade::Application::Get().CloseRequest();
		},
		[](std::vector<std::vector<std::string>>& tokens) { // CloseUnsaved
			jade::Application::Get().CloseUnsavedRequest();
		},
		GlobalAddExecute,
		[](std::vector<std::vector<std::string>>& tokens) { // GlobalSave
			jade::MusicDatabase::Get().SaveChanges();
			jade::EventEmitter<jade::OnGlobalSaved>().Emit();
		},
		GlobalShowExecute,
		PlaylistCreateExecute
	};
}

jade::BackendConsole::BackendConsole() {
	EventSystem::Get().Subscribe<OnApplicationClose>(50, [](OnApplicationClose e) {
		if (e.closeState == OnApplicationClose::DatabaseChangesUnsaved) {
			std::cout <<
			" - You have unsaved global changes. To save type 'global_save' or 'close_unsaved' to exit without changes\n";
		}
	});
}

void jade::BackendConsole::Update() {
	std::string cmdLine;

	ShowInputRow();
	std::getline(std::cin, cmdLine);

	if (cmdLine.empty()) {
		return;
	}
	std::vector<std::vector<std::string>> tokens = Tokenize(cmdLine);
	if (!tokens.back().empty()) {
		ShowError(tokens.back().front());
		return;
	}
	tokens.pop_back();
	Command command = GetCommandFromName(tokens.front().front());

	g_ExecuteCommandFunctions[(int)command](tokens);
}

void jade::BackendConsole::Render() {

}

namespace {
	void ShowInputRow() {
		std::cout << "<Jade> ";
	}

	void ShowError(const std::string& error) {
		std::cout << " - Error: " << error << '\n';
	}

	Command GetCommandFromName(const std::string& name) {
		auto it = g_CommandMap.find(name);
		if (it == g_CommandMap.cend()) {
			return Command::NoCmd;
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

	void GlobalShowExecute(std::vector<std::vector<std::string>>& tokens) {
		auto trackIt	= jade::MusicDatabase::Get().TrackIteratorBegin();
		auto trackItEnd = jade::MusicDatabase::Get().TrackIteratorEnd();

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
	}

	void GlobalAddExecute(std::vector<std::vector<std::string>>& tokens) {
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
				std::cout << " - Unknown parameter pack '" << pack.front() << "'\n";
				return;
			}
		}
		std::string error = jade::MusicDatabase::Get().Add(artists, feat, name, path);
		if (!error.empty()) {
			ShowError(error);
			return;
		}
		jade::EventEmitter<jade::OnGlobalAdded>().Emit();
	}

	void PlaylistCreateExecute(std::vector<std::vector<std::string>>& tokens) {
		std::string name;
		std::vector<uint64_t> ids;

		// todo
	}
}