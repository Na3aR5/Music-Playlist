#ifndef JADE_AUDIO_HEADER
#define JADE_AUDIO_HEADER

#include <string>

namespace jade {
	class Audio {
	public:
		static size_t GetTrackLengthSeconds(const std::string& path);
	};
}

#endif // !JADE_AUDIO_HEADER