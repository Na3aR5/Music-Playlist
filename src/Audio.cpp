#include <jade/Audio.h>
#include <miniaudio.h>

size_t jade::Audio::GetTrackLengthSeconds(const char* path) {
	ma_decoder decoder;
	ma_decoder_init_file(path, nullptr, &decoder);
	ma_uint64 lengthInFrames = 0;
	ma_decoder_get_length_in_pcm_frames(&decoder, &lengthInFrames);

	size_t seconds = lengthInFrames / decoder.outputSampleRate;
	ma_decoder_uninit(&decoder);

	return seconds;
}