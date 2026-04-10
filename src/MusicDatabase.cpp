#include <jade/MusicDatabase.h>
#include <jade/Audio.h>
#include <jade/App.h>

#include <stdexcept>

template <typename T>
struct ObjectSerializer {
	void operator()(std::fstream& file, const T& object) const {
		file.write((const char*)&object, sizeof(T));
	}
};

template <typename T>
struct ObjectDeserializer {
	T operator()(const char*& source) const {
		T object = *(const T*)source;
		source += sizeof(T);
		return object;
	}
};

template <>
struct ObjectSerializer<std::string> {
	void operator()(std::fstream& file, const std::string& str) const {
		size_t length = str.length();
		file.write((const char*)&length, sizeof(size_t));
		file.write(str.c_str(), length);
	}
};

template <>
struct ObjectSerializer<std::filesystem::path> {
	void operator()(std::fstream& file, const std::filesystem::path& path) const {
		std::string str = path.string();
		ObjectSerializer<std::string>()(file, str);
	}
};

template <typename T>
struct ObjectSerializer<std::vector<T>> {
	void operator()(std::fstream& file, const std::vector<T>& vec) const {
		size_t size = vec.size();
		file.write((const char*)&size, sizeof(size_t));

		if constexpr (std::is_trivial_v<T>) {
			file.write((const char*)vec.data(), sizeof(T) * size);
		}
		else {
			for (const T& v : vec) {
				ObjectSerializer<T>()(file, v);
			}
		}
	}
};

template <>
struct ObjectSerializer<jade::MusicDatabase::TrackElement> {
	void operator()(std::fstream& file, const jade::MusicDatabase::TrackElement& elem) const {
		ObjectSerializer<decltype(elem.id)>()(file, elem.id);
		ObjectSerializer<decltype(elem.seconds)>()(file, elem.seconds);
		ObjectSerializer<decltype(elem.artists)>()(file, elem.artists);
		ObjectSerializer<decltype(elem.feat)>()(file, elem.feat);
		ObjectSerializer<decltype(elem.name)>()(file, elem.name);
		ObjectSerializer<decltype(elem.audioPath)>()(file, elem.audioPath);
	}
};

template <>
struct ObjectSerializer<jade::MusicDatabase::PlaylistElement> {
	void operator()(std::fstream& file, const jade::MusicDatabase::PlaylistElement& list) const {
		ObjectSerializer<decltype(list.id)>()(file, list.id);
		ObjectSerializer<decltype(list.seconds)>()(file, list.seconds);
		ObjectSerializer<decltype(list.name)>()(file, list.name);

		ObjectSerializer<decltype(list.tracks.size())>()(file, list.tracks.size());
		for (uint64_t trackId : list.tracks) {
			ObjectSerializer<uint64_t>()(file, trackId);
		}
	}
};

template <>
struct ObjectDeserializer<std::string> {
	std::string operator()(const char*& source) const {
		size_t length = *(const size_t*)source;
		source += sizeof(size_t);

		std::string str;
		str.assign(source, length);

		source += length;
		return str;
	}
};

template <>
struct ObjectDeserializer<std::filesystem::path> {
	std::filesystem::path operator()(const char*& source) const {
		return ObjectDeserializer<std::string>()(source);
	}
};

template <typename T>
struct ObjectDeserializer<std::vector<T>> {
	std::vector<T> operator()(const char*& source) const {
		size_t size = *(const size_t*)source;
		source += sizeof(size_t);

		std::vector<T> vec;
		if constexpr (std::is_trivial_v<T>) {
			vec.resize(size);
			std::memcpy(vec.data(), source, sizeof(T) * size);
			source += sizeof(T) * size;
		}
		else {
			vec.reserve(size);
			for (size_t i = 0; i < size; ++i) {
				vec.emplace_back(ObjectDeserializer<T>()(source));
			}
		}
		return vec;
	}
};

template <>
struct ObjectDeserializer<jade::MusicDatabase::TrackElement> {
	jade::MusicDatabase::TrackElement operator()(const char*& source) const {
		jade::MusicDatabase::TrackElement track = {};

		track.id		= ObjectDeserializer<decltype(jade::MusicDatabase::TrackElement::id)>()(source);
		track.seconds   = ObjectDeserializer<decltype(jade::MusicDatabase::TrackElement::seconds)>()(source);
		track.artists   = std::move(ObjectDeserializer<decltype(jade::MusicDatabase::TrackElement::artists)>()(source));
		track.feat	    = std::move(ObjectDeserializer<decltype(jade::MusicDatabase::TrackElement::feat)>()(source));
		track.name      = std::move(ObjectDeserializer<decltype(jade::MusicDatabase::TrackElement::name)>()(source));
		track.audioPath = std::move(ObjectDeserializer<decltype(jade::MusicDatabase::TrackElement::audioPath)>()(source));

		return track;
	}
};

template <>
struct ObjectDeserializer<jade::MusicDatabase::PlaylistElement> {
	jade::MusicDatabase::PlaylistElement operator()(const char*& source) const {
		jade::MusicDatabase::PlaylistElement playlist = {};

		playlist.id		 = ObjectDeserializer<decltype(jade::MusicDatabase::PlaylistElement::id)>()(source);
		playlist.seconds = ObjectDeserializer<decltype(jade::MusicDatabase::PlaylistElement::seconds)>()(source);
		playlist.name    = std::move(ObjectDeserializer<decltype(jade::MusicDatabase::PlaylistElement::name)>()(source));

		size_t playlistSize = ObjectDeserializer<decltype(jade::MusicDatabase::PlaylistElement::tracks.size())>()(source);
		playlist.tracks.reserve(playlistSize);

		for (size_t i = 0; i < playlistSize; ++i) {
			uint64_t id = ObjectDeserializer<decltype(jade::MusicDatabase::TrackElement::id)>()(source);
			playlist.tracks.push_back(id);
		}
		return playlist;
	}
};

namespace {
	jade::MusicDatabase* g_Database = nullptr;
}

namespace {
	void TryOpenFile(std::fstream& file, const char* path);
	std::string ReadFileContents(std::fstream& file);
}

jade::MusicDatabase::MusicDatabase() {
	if (g_Database != nullptr) {
		throw std::runtime_error("Music database is already created");
	}
	TryOpenFile(m_tracksMetadataFile, Config::Paths::MusicMetadataFile);
	{
		std::string readContents = ReadFileContents(m_tracksMetadataFile);
		if (!readContents.empty()) {
			const char* source = readContents.c_str();
			m_tracks = std::move(ObjectDeserializer<decltype(m_tracks)>()(source));
		}
	}
	TryOpenFile(m_playlistMetadataFile, Config::Paths::MusicPlaylistFile);
	{
		std::string readContents = ReadFileContents(m_playlistMetadataFile);
		if (!readContents.empty()) {
			const char* source = readContents.c_str();
			m_playlists = std::move(ObjectDeserializer<decltype(m_playlists)>()(source));
		}
	}

	if (!std::filesystem::exists(Config::Paths::MusicStorage)) {
		std::filesystem::create_directories(Config::Paths::MusicStorage);
	}

	EventSystem::Get().Subscribe<OnApplicationClose>(100, [this](OnApplicationClose& e) {
		if (m_changeStates != 0) {
			e.closeState = OnApplicationClose::DatabaseChangesUnsaved;
		}
	});

	g_Database = this;
}

jade::MusicDatabase::~MusicDatabase() {
	g_Database = nullptr;
}

jade::MusicDatabase& jade::MusicDatabase::Get() {
	return *g_Database;
}

const jade::MusicDatabase& jade::MusicDatabase::GetConst() {
	return *g_Database;
}

void jade::MusicDatabase::SaveChanges() {
	if (m_changeStates == 0) {
		return;
	}
	if (m_changeStates & ChangeState::TrackListChangeBit) {
		m_tracksMetadataFile.seekp(0, std::ios::beg);
		ObjectSerializer<decltype(m_tracks)>()(m_tracksMetadataFile, m_tracks);
		m_changeStates &= ~ChangeState::TrackListChangeBit;
	}
	if (m_changeStates & ChangeState::PlaylistChangeBit) {
		m_playlistMetadataFile.seekp(0, std::ios::beg);
		ObjectSerializer<decltype(m_playlists)>()(m_playlistMetadataFile, m_playlists);
		m_changeStates &= ~ChangeState::PlaylistChangeBit;
	}
}

std::string jade::MusicDatabase::Add(
const std::vector<std::string>& artists,
const std::vector<std::string>& feat,
const std::string& name,
const std::filesystem::path& path) {
	jade::MusicDatabase::TrackElement track = {};
	track.artists = artists;
	track.feat    = feat;
	track.name    = name;

	if (!std::filesystem::exists(path)) {
		return std::string("Path '") + path.string() + "' does not exist in filesystem";
	}
	std::filesystem::copy_file(
		path,
		Config::Paths::MusicStorage / path.filename(),
		std::filesystem::copy_options::overwrite_existing
	);
	track.audioPath = Config::Paths::MusicStorage / path.filename();
	track.seconds = Audio::GetTrackLengthSeconds(track.audioPath.string().c_str());
	track.id = m_tracks.size();

	m_tracks.emplace_back(std::move(track));
	m_changeStates |= ChangeState::TrackListChangeBit;

	return {};
}

std::string jade::MusicDatabase::CreatePlaylist(const std::string& name, const std::vector<uint64_t>& ids) {
	PlaylistElement playlist = {};
	playlist.seconds = 0;

	std::string error;
	for (uint64_t id : ids) {
		if (id >= m_tracks.size()) {
			if (error.empty()) error += std::to_string(id);
			else error += std::string(", ") + std::to_string(id);
			continue;
		}
		TrackElement& track = m_tracks[id];
		playlist.seconds += track.seconds;
		playlist.tracks.emplace_back(id);
	}
	playlist.name = name;
	playlist.id = m_playlists.size();
	m_playlists.emplace_back(std::move(playlist));

	m_changeStates |= ChangeState::PlaylistChangeBit;

	if (!error.empty()) {
		error += " do not exits in music database";
		return error;
	}
	return {};
}

std::vector<jade::MusicDatabase::TrackElement>::const_iterator jade::MusicDatabase::TrackIteratorBegin() const {
	return m_tracks.cbegin();
}

std::vector<jade::MusicDatabase::TrackElement>::const_iterator jade::MusicDatabase::TrackIteratorEnd() const {
	return m_tracks.cend();
}

namespace {
	void TryOpenFile(std::fstream& file, const char* path) {
		file.open(path, std::ios::binary | std::ios::in | std::ios::out);
		if (!file.is_open()) {
			{
				std::ofstream temp(path, std::ios::binary);
				temp.close();
			}
			file.open(path, std::ios::binary | std::ios::in | std::ios::out);
			if (!file.is_open()) {
				throw std::runtime_error("Failed to read open file");
			}
		}
	}

	std::string ReadFileContents(std::fstream& file) {
		std::string buffer;

		file.seekg(0, std::ios::end);
		size_t size = file.tellg();
		file.seekg(0, std::ios::beg);

		buffer.resize(size);

		if (!file.read(buffer.data(), size)) {
			throw std::runtime_error("Failed to read file contents");
		}
		return buffer;
	}
}