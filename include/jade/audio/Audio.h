#ifndef JADE_AUDIO_HEADER
#define JADE_AUDIO_HEADER

#include <string>
#include <vector>
#include <memory>

namespace jade {
	class Audio {
	public:
		static double GetTrackLengthSeconds(const std::string& path);
	};

	class AudioStream {
	public:
		AudioStream();
		~AudioStream();

	public:
		bool Initialize(const std::string& audioPath, double seconds);
		std::vector<float> Read(size_t chunkSize);

		size_t GetSampleRate() const;
		uint32_t GetChannelCount() const;

	private:
		struct _Impl;
		std::unique_ptr<_Impl> m_impl;
	};
}

#endif // !JADE_AUDIO_HEADER