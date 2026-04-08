#include <musicplaylist/BackendConsole.h>
#include <musicplaylist/App.h>

#include <miniaudio.h>

#include <iostream>
#include <string>
#include <vector>
#include <map>

namespace {
	enum class Command {
		EXIT,
		ADD_GLOBAL,
		GET_GLOBAL,
		PLAY,
		STOP,
		CREATE_PLAYLIST,

		ENUM_SIZE
	};

	struct CommandInfo {
		Command cmd;
		std::vector<std::string> tokens;
	};

	const std::map<std::string, Command> COMMAND_MAP = {
		{ "exit", Command::EXIT },
		{ "add_global", Command::ADD_GLOBAL },
		{ "get_global", Command::GET_GLOBAL },
		{ "play", Command::PLAY },
		{ "stop", Command::STOP },
		{ "create_playlist", Command::CREATE_PLAYLIST }
	};

	void (*EXECUTE_COMMANDS[(int)Command::ENUM_SIZE])(const CommandInfo&) = {
		[](const CommandInfo& info) {
			mspl::Application::Get()->StopLoop();
		},
		[](const CommandInfo& info) {
			mspl::DatabaseElement element;
			uint32_t artistCount = (uint32_t)info.tokens.size() - 3;
			element.artists.assign(info.tokens.begin() + 1, info.tokens.begin() + (1 + artistCount));
			element.name = info.tokens[1 + artistCount];
			element.audioPath = info.tokens[2 + artistCount];

			ma_decoder decoder;
			ma_decoder_init_file(info.tokens[2 + artistCount].c_str(), nullptr, &decoder);
			ma_uint64 lengthInFrames = 0;
			ma_decoder_get_length_in_pcm_frames(&decoder, &lengthInFrames);
			element.durationSeconds = (double)lengthInFrames / decoder.outputSampleRate;

			mspl::Application::Get()->Database().Add(element);
		},
		[](const CommandInfo& info) {
			const mspl::DatabaseElement* element = mspl::Application::Get()->Database().Get(info.tokens[1]);
			if (element == nullptr) {
				std::cout << "No track found with name '" << info.tokens[1] << "'\n";
				return;
			}
			for (uint32_t i = 0; i < element->artists.size(); ++i) {
				std::cout << element->artists[i];
				if (i + 1 < element->artists.size()) {
					std::cout << ", ";
				}
			}
			std::cout << " - " << element->name;
			std::cout << " (" << element->durationSeconds << "s)\n";
		},
		[](const CommandInfo& info) {
			const mspl::DatabaseElement* element = mspl::Application::Get()->Database().Get(info.tokens[1]);
			mspl::Application::Get()->Player().Play(mspl::PlaylistElement(
				element->audioPath, element->durationSeconds
			));
		},
		[](const CommandInfo& info) {
			mspl::Application::Get()->Player().StopDevice();
		},
		[](const CommandInfo& info) {
			mspl::Playlist playlist;

			size_t tokenSize = info.tokens.size();
			for (size_t i = 2; i < tokenSize; ++i) {
				const mspl::DatabaseElement* it = mspl::Application::Get()->Database().Get(info.tokens[i]);
				if (it == nullptr) {
					std::cout << " - Not found '" << info.tokens[i] << "' in global database\n";
					continue;
				}
				mspl::PlaylistElement element(it->audioPath, it->durationSeconds);
				playlist.Add(element);
				std::cout << " - Added '" << info.tokens[i] << "'\n";
			}
		},
	};

	void DispatchCommand(Command cmd, std::vector<std::string>& tokens) {
		CommandInfo cmdInfo = {
			.cmd = cmd,
			.tokens = std::move(tokens)
		};
		EXECUTE_COMMANDS[(int)cmd](cmdInfo);
	}

	std::vector<std::string> TokenizeCommandLine(const std::string& line) {
		std::vector<std::string> tokens;

		size_t i = 0;
		size_t lineLength = line.length();

		while (i < lineLength) {
			if (std::isspace(line[i])) {
				++i;
				continue;
			}
			if (line[i] == '"') {
				++i;
				std::string token;
				while (i < lineLength && line[i] != '"') {
					token += line[i];
					++i;
				}
				++i;
				tokens.emplace_back(std::move(token));
				continue;
			}
			std::string token;
			while (i < lineLength && !std::isspace(line[i])) {
				token += line[i];
				++i;
			}
			tokens.emplace_back(std::move(token));
		}
		return tokens;
	}
}

namespace {
	void ShowInputLine() {
		std::cout << "<Music playlist v" <<
			mspl::Config::VERSION_MAJOR << '.' <<
			mspl::Config::VERSION_MINOR << '.' <<
			mspl::Config::VERSION_PATCH << "> ";
	}
}

void mspl::BackendConsole::HandleEvents() {
	std::string cmdLine;

	ShowInputLine();
	std::getline(std::cin, cmdLine);
	std::vector<std::string> tokens = TokenizeCommandLine(cmdLine);

	if (tokens.empty()) {
		std::cout << '\n';
		return;
	}
	auto cmdIt = COMMAND_MAP.find(tokens[0]);
	if (cmdIt == COMMAND_MAP.cend()) {
		std::cout << "Unknown command!\n";
		return;
	}
	DispatchCommand(cmdIt->second, tokens);
}

void mspl::BackendConsole::Update() {

}

void mspl::BackendConsole::Render() {}