#ifndef MUSIC_PLAYLIST_BACKEND_HEADER
#define MUSIC_PLAYLIST_BACKEND_HEADER

namespace mspl {
	class IBackend {
	public:
		virtual void HandleEvents() = 0;
		virtual void Update() = 0;
		virtual void Render() = 0;
	};
}

#endif // !MUSIC_PLAYLIST_BACKEND_HEADER