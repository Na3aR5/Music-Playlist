#ifndef JADE_AUDIO_HEADER
#define JADE_AUDIO_HEADER

namespace jade {
	class Audio {
	public:
		static size_t GetTrackLengthSeconds(const char* path);
	};
}

#endif // !JADE_AUDIO_HEADER