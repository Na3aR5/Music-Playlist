#ifndef JADE_MUSIC_DATABASE_HEADER
#define JADE_MUSIC_DATABASE_HEADER

#include <jade/Event.h>

#include <fstream>
#include <vector>
#include <filesystem>

namespace jade {
	class MusicDatabase {
	public:
		struct TrackElement {
			uint64_t                 id;
			size_t					 seconds;
			std::vector<std::string> artists;
			std::vector<std::string> feat;
			std::string				 name;
			std::filesystem::path    audioPath;
		};

		struct PlaylistElement {
			uint64_t			  id;
			size_t				  seconds;
			std::string			  name;
			std::vector<uint64_t> tracks;
		};

		enum ChangeState : uint64_t {
			TrackListChangeBit = 0x1,
			PlaylistChangeBit  = 0x2
		};

	public:
		MusicDatabase();
		~MusicDatabase();

	public:
		static MusicDatabase& Get();
		static const MusicDatabase& GetConst();

	public:
		void SaveChanges();

		std::string Add(
			const std::vector<std::string>& artists,
			const std::vector<std::string>& feat,
			const std::string& name,
			const std::filesystem::path& path
		);

		std::string CreatePlaylist(
			const std::string& name,
			const std::vector<uint64_t>& ids
		);

		std::vector<TrackElement>::const_iterator TrackIteratorBegin() const;
		std::vector<TrackElement>::const_iterator TrackIteratorEnd() const;

	private:
		uint64_t				     m_changeStates = 0;
		std::vector<TrackElement>    m_tracks;
		std::vector<PlaylistElement> m_playlists;
		std::fstream			     m_playlistMetadataFile;
		std::fstream			     m_tracksMetadataFile;
	};
}

#endif // !JADE_MUSIC_DATABASE_HEADER