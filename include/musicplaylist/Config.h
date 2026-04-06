#ifndef MUSIC_PLAYLIST_CONFIG_HEADER
#define MUSIC_PLAYLIST_CONFIG_HEADER

#include <cstdint>

namespace mspl {
	enum class BackendType {
		CONSOLE
	};

	class Config {
	public:
		static const BackendType BACKEND_TYPE  = BackendType::CONSOLE;
		static const uint16_t	 VERSION_MAJOR = 1;
		static const uint16_t    VERSION_MINOR = 0;
		static const uint16_t    VERSION_PATCH = 0;
		static const char*		 DATABASE_FILEPATH;
	};
}

#endif // !MUSIC_PLAYLIST_CONFIG_HEADER