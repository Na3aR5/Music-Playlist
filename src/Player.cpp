#include <musicplaylist/Player.h>

#include <miniaudio.h>

mspl::Player::Player() : m_device() {}

mspl::Player::~Player() {
    StopDevice();
}

void mspl::Player::Play(const PlaylistElement& element) {
    StopDevice();

    m_currentTrackStream.Set(element);
    m_currentTrackStream.ResetIndex();

    ma_device_config deviceConfig = ma_device_config_init(ma_device_type_playback);
    deviceConfig.playback.format = ma_format_f32;
    deviceConfig.playback.channels = 2;
    deviceConfig.sampleRate = 44100;

    deviceConfig.dataCallback = [](ma_device* pDevice, void* pOutput, const void*, ma_uint32 frameCount) {
        AudioStream* stream = (AudioStream*)pDevice->pUserData;

        std::vector<float> buffer = stream->Read(frameCount);

        float* out = (float*)pOutput;

        size_t samples = buffer.size();
        for (size_t i = 0; i < samples; ++i)
            out[i] = buffer[i];

        for (size_t i = samples; i < frameCount * 2; ++i)
            out[i] = 0;
        };

    deviceConfig.pUserData = &m_currentTrackStream;

    ma_device_init(nullptr, &deviceConfig, &m_device);
    ma_device_start(&m_device);

    m_deviceIsInitialized = true;
}

void mspl::Player::StopDevice() {
    if (m_deviceIsInitialized) {
        ma_device_stop(&m_device);
        ma_device_uninit(&m_device);
        m_deviceIsInitialized = false;
    }
}

mspl::AudioStream::AudioStream(uint32_t maxFramesCached) : m_element(), m_decoder(), m_cache(maxFramesCached) {}

mspl::AudioStream::AudioStream(const PlaylistElement& element, uint32_t maxFramesCached) :
    m_element(element), m_decoder(), m_cache(maxFramesCached) {
    ma_decoder_init_file(element.GetPath().string().c_str(), nullptr, &m_decoder);
}

mspl::AudioStream::~AudioStream() {
    ma_decoder_uninit(&m_decoder);
}

void mspl::AudioStream::ResetIndex() {
    m_chunkIndex = 0;
}

void mspl::AudioStream::Set(const PlaylistElement& element) {
    ma_decoder_uninit(&m_decoder);
    ma_decoder_init_file(element.GetPath().string().c_str(), nullptr, &m_decoder);
}

std::vector<float> mspl::AudioStream::Read(size_t chunkSize) {
    std::vector<float>* cached = m_cache.Get(m_chunkIndex);
    if (cached != nullptr) {
        return *cached;
    }
    std::vector<float> buffer(chunkSize * m_decoder.outputChannels);

    ma_uint64 framesRead = 0;
    ma_decoder_read_pcm_frames(&m_decoder, buffer.data(), chunkSize, &framesRead);

    buffer.resize(framesRead * m_decoder.outputChannels);
    m_cache.Insert(m_chunkIndex, buffer);
    ++m_chunkIndex;

    return buffer;
}