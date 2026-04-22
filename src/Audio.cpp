#include <jade/audio/Audio.h>
#include <miniaudio.h>

struct jade::AudioStream::_Impl {
public:
	enum State {
		DecoderInitializedBit	  = 0x1,
		HasPrecomputedDurationBit = 0x2
	};

public:
	bool Initialize(const std::string& path, double seconds) {
		ma_decoder newDecoder;
		ma_result initResult = ma_decoder_init_file(path.c_str(), nullptr, &newDecoder);
		if (initResult != ma_result::MA_SUCCESS) {
			return false;
		}
		this->decoder = newDecoder;
		this->duration = seconds;

		this->states = State::DecoderInitializedBit | State::HasPrecomputedDurationBit;
		return true;
	}

	void Terminate() {
		if (states & State::DecoderInitializedBit) {
			ma_decoder_uninit(&decoder);
		}
		states = 0;
	}

public:
	uint64_t   states     = 0;
	ma_uint64  framesRead = 0;
	double     duration   = 0.0;
	ma_decoder decoder    = {};
};

double jade::Audio::GetTrackLengthSeconds(const std::string& path) {
	ma_decoder decoder;
	ma_decoder_init_file(path.c_str(), nullptr, &decoder);
	ma_uint64 lengthInFrames = 0;
	ma_decoder_get_length_in_pcm_frames(&decoder, &lengthInFrames);

	double seconds = (double)lengthInFrames / decoder.outputSampleRate;
	ma_decoder_uninit(&decoder);

	return seconds;
}

jade::AudioStream::AudioStream() {
	m_impl = std::make_unique<_Impl>();
}

jade::AudioStream::~AudioStream() {
	if (m_impl) {
		m_impl->Terminate();
		m_impl.reset();
	}
}

bool jade::AudioStream::Initialize(const std::string& path, double seconds) {
	return m_impl->Initialize(path, seconds);
}

std::vector<float> jade::AudioStream::Read(size_t chunkSize) {
	std::vector<float> buffer(chunkSize * m_impl->decoder.outputChannels);

	ma_uint64 framesRead = 0;
	ma_decoder_read_pcm_frames(&m_impl->decoder, buffer.data(), chunkSize, &framesRead);
	buffer.resize(framesRead * m_impl->decoder.outputChannels);

	this->m_impl->framesRead += framesRead;
	return buffer;
}

size_t jade::AudioStream::GetSampleRate() const {
	return (size_t)m_impl->decoder.outputSampleRate;
}

uint32_t jade::AudioStream::GetChannelCount() const {
	return (uint32_t)m_impl->decoder.outputChannels;
}