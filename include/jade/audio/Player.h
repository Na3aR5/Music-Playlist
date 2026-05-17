#ifndef JADE_AUDIO_PLAYER_HEADER
#define JADE_AUDIO_PLAYER_HEADER

#include <jade/audio/Audio.h>
#include <jade/MusicLibrary.h>

namespace jade {
	class Player {
	public:
		struct Impl;

		Player();
		~Player();

	public:
		bool IsPlaying() const;
		void Play(const MusicLibrary::TrackElement& track);
		void Resume();
		void Pause();
		void SetVolume(float volume);

	private:
		std::unique_ptr<Impl> m_impl;
	};
}

#endif // !JADE_AUDIO_PLAYER_HEADER