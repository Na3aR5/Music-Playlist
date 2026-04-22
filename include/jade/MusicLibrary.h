#ifndef JADE_MUSIC_LIBRARY_HEADER
#define JADE_MUSIC_LIBRARY_HEADER

#include <jade/Event.h>

#include <map>
#include <vector>
#include <future>
#include <memory>
#include <fstream>
#include <filesystem>

namespace jade {
	class MusicLibrary {
	public:
		struct TrackElement {
			uint64_t                 id;
			double					 seconds;
			std::vector<std::string> artists;
			std::vector<std::string> feat;
			std::string				 name;
			std::filesystem::path    audioPath;
		};

		struct PlaylistElement {
			uint64_t			  id;
			double				  seconds;
			std::string			  name;
			std::vector<uint64_t> tracks;
		};

		using TrackIterator = std::map<uint64_t, TrackElement>::const_iterator;

		enum ChangeState : uint64_t {
			TrackListChangeBit = 0x1,
			PlaylistChangeBit  = 0x2
		};

	public:
		MusicLibrary();
		~MusicLibrary();

	public:
		static MusicLibrary& Get();
		static const MusicLibrary& GetConst();

	public:
		std::future<void> SaveChanges();
		TrackIterator GetTrackByID(uint64_t id) const;

		std::future<void> Add(
			const std::vector<std::string>& artists,
			const std::vector<std::string>& feat,
			const std::string& name,
			const std::filesystem::path& path,
			const std::shared_ptr<FutureTask>& task
		);

		std::string CreatePlaylist(
			const std::string& name,
			const std::vector<uint64_t>& ids
		);

		TrackIterator TrackIteratorBegin() const;
		TrackIterator TrackIteratorEnd() const;

	private:
		uint64_t				         m_changeStates = 0;
		std::map<uint64_t, TrackElement> m_tracks;
		std::vector<PlaylistElement>     m_playlists;
		std::fstream			         m_playlistMetadataFile;
		std::fstream			         m_tracksMetadataFile;
	};
}

#endif // !JADE_MUSIC_LIBRARY_HEADER