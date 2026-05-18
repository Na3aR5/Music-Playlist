#include <jade/audio/Audio.h>

#include <miniaudio.h>
#include <ma_reverb_node/ma_reverb_node.h>

struct jade::IAudioStream::_BaseImpl {
public:
	_BaseImpl() {
		ma_node_graph_config config = {};
		config.channels = 2;
		ma_node_graph_init(&config, nullptr, &this->nodeGraph);
	}

	~_BaseImpl() {
		ma_node_graph_uninit(&this->nodeGraph, nullptr);
	}

public:
	ma_node_graph nodeGraph = {};
};

struct jade::AudioStream::_Impl {
public:
	enum State {
		DecoderInitializedBit	  = 0x1,
		HasPrecomputedDurationBit = 0x2
	};

	struct DecoderNode {
		ma_node_base base;
		ma_decoder* decoder;
	};

	static void ReadFrames(ma_node* pNode, const float** ppIn, ma_uint32* pInFrames, float** ppOut, ma_uint32* pOutFrames) {
		DecoderNode* self = (DecoderNode*)pNode;

		ma_uint64 read = 0;
		ma_decoder_read_pcm_frames(self->decoder, ppOut[0], *pOutFrames, &read);

		if (read < *pOutFrames) {
			size_t ch = self->decoder->outputChannels;
			memset(ppOut[0] + read * ch, 0, (*pOutFrames - read) * ch * sizeof(float));
		}
	}

	static ma_node_vtable s_DecoderVTable;

	static void init_decoder_node(ma_node_graph* graph, ma_decoder* decoder, DecoderNode* node) {
		ma_node_config cfg = ma_node_config_init();
		cfg.vtable = &s_DecoderVTable;

		ma_node_init(graph, &cfg, NULL, &node->base);
		node->decoder = decoder;
	}

public:
	bool Initialize(const std::string& path, double seconds, ma_node_graph* nodeGraph) {
		ma_decoder newDecoder;
		ma_result initResult = ma_decoder_init_file(path.c_str(), nullptr, &newDecoder);
		if (initResult != ma_result::MA_SUCCESS) {
			return false;
		}
		this->decoder = newDecoder;
		this->duration = seconds;

		init_decoder_node(nodeGraph, &this->decoder, &this->decoderNode);
		ma_node_attach_output_bus(
			&this->decoderNode.base, 0,
			ma_node_graph_get_endpoint(nodeGraph), 0
		);

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
	uint64_t      states       = 0;
	ma_uint64     framesRead   = 0;
	double        duration     = 0.0;
	DecoderNode   decoderNode  = {};
	ma_decoder    decoder      = {};
};

ma_node_vtable jade::AudioStream::_Impl::s_DecoderVTable = {
	ReadFrames, nullptr, 1, 0
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
	m_baseImpl = std::make_shared<_BaseImpl>();
}

jade::AudioStream::~AudioStream() {
	if (m_impl) {
		m_impl->Terminate();
	}
}

bool jade::AudioStream::Initialize(const std::string& path, double seconds) {
	return m_impl->Initialize(path, seconds, &m_baseImpl->nodeGraph);
}

std::vector<float> jade::AudioStream::Read(size_t chunkSize) {
	std::vector<float> buffer(chunkSize * m_impl->decoder.outputChannels);

	ma_uint64 framesRead = 0;
	ma_decoder_read_pcm_frames(&m_impl->decoder, buffer.data(), chunkSize, &framesRead);
	buffer.resize(framesRead * m_impl->decoder.outputChannels);

	m_impl->framesRead += framesRead;
	return buffer;
}

size_t jade::AudioStream::GetSampleRate() const {
	return m_impl->decoder.outputSampleRate;
}

uint32_t jade::AudioStream::GetChannelCount() const {
	return (uint32_t)m_impl->decoder.outputChannels;
}

struct jade::AudioStreamSpeeded::_Impl {
public:
	_Impl(IAudioStream* stream, ma_node_graph* nodeGraph) {
		this->reverbNodeConfig = ma_reverb_node_config_init(
			(ma_uint32)stream->GetSampleRate(),
			stream->GetChannelCount()
		);

		this->reverbNodeConfig.roomSize  = 0.8f;
		this->reverbNodeConfig.damping   = 0.4f;
		this->reverbNodeConfig.wetVolume = 0.3f;
		this->reverbNodeConfig.dryVolume = 0.7f;

		ma_reverb_node_init(nodeGraph, &this->reverbNodeConfig, nullptr, &this->reverbNode);
	}

	~_Impl() {
		ma_reverb_node_uninit(&this->reverbNode, nullptr);
	}

public:
	ma_reverb_node_config reverbNodeConfig = {};
	ma_reverb_node		  reverbNode	   = {};
};

jade::AudioStreamSpeeded::AudioStreamSpeeded(const std::shared_ptr<IAudioStream>& stream, double speed) :
IAudioStreamEffect(stream), m_speed(speed) {
	m_baseImpl = _GetBaseImpl(m_stream.get());
	m_impl = std::make_unique<_Impl>(m_stream.get(), &m_baseImpl->nodeGraph);
}

jade::AudioStreamSpeeded::~AudioStreamSpeeded() {
	m_impl.reset();
}

bool jade::AudioStreamSpeeded::Initialize(const std::string& audioPath, double seconds) {
	return m_stream->Initialize(audioPath, seconds);
}

std::vector<float> jade::AudioStreamSpeeded::Read(size_t chunkSize) {
	return m_stream->Read((size_t)(chunkSize * m_speed));
}

size_t jade::AudioStreamSpeeded::GetSampleRate() const {
	return m_stream->GetSampleRate();
}

uint32_t jade::AudioStreamSpeeded::GetChannelCount() const {
	return m_stream->GetChannelCount();
}

std::shared_ptr<jade::IAudioStream> jade::SetSpeed(std::shared_ptr<IAudioStream>& stream, double speed) {
	std::shared_ptr<IAudioStream> currStream = stream;
	while (!dynamic_cast<AudioStream*>(currStream.get())) {
		AudioStreamSpeeded* speedStream = dynamic_cast<AudioStreamSpeeded*>(currStream.get());
		if (speedStream) {
			speedStream->SetSpeed(speed);
			return stream;
		}
		IAudioStreamEffect* effect = (IAudioStreamEffect*)currStream.get();
		currStream = effect->GetStream();
	}
	return std::make_shared<AudioStreamSpeeded>(stream, speed);
}