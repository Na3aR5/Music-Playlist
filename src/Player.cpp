#include <jade/audio/Player.h>
#include <miniaudio.h>

namespace {
	void DeviceDataCallback(ma_device* device, void* output, const void*, ma_uint32 frameCount);
}

struct jade::Player::Impl {
public:
	enum State {
		DeviceInitializedBit = 0x1,
		DeviceStartedBit     = 0x2,
		IsPlayingNowBit      = 0x4
	};

public:
	Impl() {
		this->stream = std::make_shared<AudioStream>();
	}

public:
	bool SetTrack(const std::string& path, double duration) {
		this->stream->Initialize(path, duration);

		if (!(this->states & State::DeviceInitializedBit)) {
			ma_device_config deviceConfig  = ma_device_config_init(ma_device_type_playback);
			deviceConfig.playback.format   = ma_format_f32;
			deviceConfig.playback.channels = this->stream->GetChannelCount();
			deviceConfig.sampleRate		   = (ma_uint32)this->stream->GetSampleRate();
			deviceConfig.dataCallback	   = DeviceDataCallback;
			deviceConfig.pUserData         = this;

			ma_device_init(nullptr, &deviceConfig, &this->device);
			this->states |= State::DeviceInitializedBit;
		}
		return true;
	}

	void Start() {
		ma_device_start(&this->device);
		this->states |= State::IsPlayingNowBit;
	}

	void Stop() {
		ma_device_stop(&this->device);
		this->states &= ~State::IsPlayingNowBit;
	}

	void SetVolume(float volume) {
		ma_device_set_master_volume(&this->device, volume);
	}

	void Terminate() {
		if (this->states & State::DeviceInitializedBit) {
			if (this->states & State::IsPlayingNowBit) {
				ma_device_stop(&this->device);
			}
			ma_device_uninit(&this->device);
		}
		this->states = 0;
	}

public:
	uint64_t					  states = 0;
	std::shared_ptr<IAudioStream> stream = {};
	ma_device					  device = {};
};

jade::Player::Player() {
	m_impl = std::make_unique<Impl>();
}

jade::Player::~Player() {
	if (m_impl) {
		m_impl->Terminate();
		m_impl.reset();
	}
}

bool jade::Player::IsPlaying() const {
	return m_impl->states & Impl::State::IsPlayingNowBit;
}

void jade::Player::Play(const MusicLibrary::TrackElement& track) {
	m_impl->SetTrack(track.audioPath.string(), track.seconds);
	m_impl->Start();
}

void jade::Player::Resume() {
	if (!IsPlaying()) {
		m_impl->Start();
	}
}

void jade::Player::Pause() {
	if (IsPlaying()) {
		m_impl->Stop();
	}
}

void jade::Player::SetVolume(float volume) {
	m_impl->SetVolume(volume);
}

void jade::Player::SetSpeed(double speed) {
	m_impl->stream = jade::SetSpeed(m_impl->stream, speed);
}

namespace {
	void DeviceDataCallback(ma_device* device, void* output, const void*, ma_uint32 frameCount) {
		jade::Player::Impl* player = (jade::Player::Impl*)device->pUserData;
		std::vector<float> frame = player->stream->Read(frameCount);

		float* out = (float*)output;

		size_t sampleCount = frame.size();
		for (size_t i = 0; i < sampleCount; ++i) {
			out[i] = frame[i];
		}
		for (size_t i = sampleCount; i < (size_t)frameCount * player->device.playback.channels; ++i) {
			out[i] = 0;
		}
	}
}