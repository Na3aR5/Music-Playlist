#ifndef JADE_MUSIC_LIBRARY_HEADER
#define JADE_MUSIC_LIBRARY_HEADER

#include <jade/Event.h>
#include <jade/Cache.h>

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
			uint64_t                 id = UINT64_MAX;
			double					 seconds = 0.0;
			std::vector<std::string> artists;
			std::vector<std::string> feat;
			std::string				 name;
			std::filesystem::path    audioPath;
		};
		using TrackIterator = std::map<uint64_t, TrackElement>::const_iterator;

		struct PlaylistElement {
			uint64_t			  id;
			double				  seconds;
			std::string			  name;
			std::vector<uint64_t> tracks;
		};

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

	class MusicLibraryProxy {
	public:
		enum class Attachment : uint8_t {
			None  = 0x0,
			Cache = 0x1,
		};

	public:
		MusicLibraryProxy(Attachment attachments = Attachment::None);
		MusicLibraryProxy(MusicLibrary* library, Attachment attachments = Attachment::None);

		inline void SetMusicLibrary(MusicLibrary* library) noexcept { m_library = library; }

	public:
		std::future<void> SaveChanges();

		MusicLibrary::TrackIterator GetTrackByID(uint64_t id) const;
		inline MusicLibrary::TrackIterator TrackIteratorBegin() const { return m_library->TrackIteratorBegin(); }
		inline MusicLibrary::TrackIterator TrackIteratorEnd() const { return m_library->TrackIteratorEnd(); }

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

	private:
		void _CreateAttachments(Attachment attachments);

	private:
		Attachment    m_attachments = Attachment::None;
		MusicLibrary* m_library		= nullptr;

		std::unique_ptr<LRUCache<uint64_t, MusicLibrary::TrackIterator>> m_trackByIdCache = nullptr;
	};

	inline MusicLibraryProxy::Attachment operator|(MusicLibraryProxy::Attachment l, MusicLibraryProxy::Attachment r) noexcept {
		return (MusicLibraryProxy::Attachment)((uint8_t)l | (uint8_t)r);
	}
	inline MusicLibraryProxy::Attachment operator&(MusicLibraryProxy::Attachment l, MusicLibraryProxy::Attachment r) noexcept {
		return (MusicLibraryProxy::Attachment)((uint8_t)l & (uint8_t)r);
	}
}
#endif // !JADE_MUSIC_LIBRARY_HEADER