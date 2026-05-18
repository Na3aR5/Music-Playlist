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

	class IAudioStream {
	public:
		virtual ~IAudioStream() = default;

		virtual bool Initialize(const std::string& audioPath, double seconds) = 0;
		virtual std::vector<float> Read(size_t chunkSize) = 0;
		virtual size_t GetSampleRate() const = 0;
		virtual uint32_t GetChannelCount() const = 0;

	protected:
		struct _BaseImpl;
		std::shared_ptr<_BaseImpl> m_baseImpl;

		inline std::shared_ptr<_BaseImpl> _GetBaseImpl(IAudioStream* other) { return other->m_baseImpl; }
	};

	class AudioStream final : public IAudioStream {
	public:
		AudioStream();
		~AudioStream();

	public:
		virtual bool Initialize(const std::string& audioPath, double seconds) override;
		virtual std::vector<float> Read(size_t chunkSize) override;
		virtual size_t GetSampleRate() const override;
		virtual uint32_t GetChannelCount() const override;

	private:
		struct _Impl;
		std::unique_ptr<_Impl> m_impl;
	};

	class IAudioStreamEffect : public IAudioStream {
	public:
		IAudioStreamEffect(const std::shared_ptr<IAudioStream>& stream) : m_stream(stream) {}
		~IAudioStreamEffect() = default;

	public:
		std::shared_ptr<IAudioStream> GetStream() const noexcept { return m_stream; }

	protected:
		std::shared_ptr<IAudioStream> m_stream;
	};

	class AudioStreamSpeeded : public IAudioStreamEffect {
	public:
		AudioStreamSpeeded(const std::shared_ptr<IAudioStream>& stream, double speed);
		~AudioStreamSpeeded();

	public:
		virtual bool Initialize(const std::string& audioPath, double seconds) override;
		virtual std::vector<float> Read(size_t chunkSize) override;
		virtual size_t GetSampleRate() const override;
		virtual uint32_t GetChannelCount() const override;

		inline void SetSpeed(double speed) { m_speed = speed; }

	private:
		double m_speed;

		struct _Impl;
		std::unique_ptr<_Impl> m_impl;
	};

	std::shared_ptr<IAudioStream> SetSpeed(std::shared_ptr<IAudioStream>& stream, double speed);
}

#endif // !JADE_AUDIO_HEADER