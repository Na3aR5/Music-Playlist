#ifndef MUSIC_PLAYLIST_DATABASE_HEADER
#define MUSIC_PLAYLIST_DATABASE_HEADER

#include <set>
#include <vector>
#include <fstream>
#include <filesystem>

#include <musicplaylist/Playlist.h>

namespace mspl {
	struct DatabaseElement {
		double			         durationSeconds = 0.0;
		std::string				 name;
		std::filesystem::path    audioPath;
		std::vector<std::string> artists;
	};

	struct DatabaseElementComparator {
	public:
		bool operator()(const DatabaseElement& l, const DatabaseElement& r) const {
			return std::strcmp(l.name.c_str(), r.name.c_str()) < 0;
		}
	};

	class Database {
	public:
		Database(const char* metadataPath, const char* playlistPath);
		~Database();

		void Add(const DatabaseElement&);
		const DatabaseElement* Get(const std::string& name) const;

	private:
		std::multiset<DatabaseElement, DatabaseElementComparator> m_metadataSet;
		std::fstream m_metadataFile;
	};
}

#endif // !MUSIC_PLAYLIST_DATABASE_HEADER