#ifndef MUSIC_PLAYLIST_PLAYER_HEADER
#define MUSIC_PLAYLIST_PLAYER_HEADER

#include <musicplaylist/Playlist.h>
#include <musicplaylist/Cache.h>

#include <miniaudio.h>

#include <thread>
#include <atomic>

namespace mspl {
	class AudioStream {
	public:
		AudioStream(uint32_t maxFramesCached = 20);
		AudioStream(const PlaylistElement& element, uint32_t maxFramesCached = 20);

		~AudioStream();

		void ResetIndex();
		void Set(const PlaylistElement& element);

	public:
		std::vector<float> Read(size_t chunkSize);

	private:
		size_t		    m_chunkIndex = 0;
		PlaylistElement m_element;
		ma_decoder      m_decoder;
		LRUCache<size_t, std::vector<float>> m_cache;
	};

	class Player {
	public:
		Player();
		~Player();

		void Play(const PlaylistElement& element);
		void StopDevice();

	private:
		bool		m_deviceIsInitialized = false;
		ma_device   m_device;
		AudioStream m_currentTrackStream;
	};
}

#endif // !MUSIC_PLAYLIST_PLAYER_HEADER