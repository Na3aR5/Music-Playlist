#ifndef JADE_CONFIG_HEADER
#define JADE_CONFIG_HEADER

namespace jade {
	class Config {
	public:
		static constexpr enum class Backend {
			CONSOLE
		} BackendType = Backend::CONSOLE;

		class Paths {
		public:
			static constexpr const char* MusicMetadataFile = "./mdb.bin";
			static constexpr const char* MusicPlaylistFile = "./mpl.bin";
			static constexpr const char* MusicStorage	   = "./music";
		};
	};
}

#endif // !JADE_CONFIG_HEADER