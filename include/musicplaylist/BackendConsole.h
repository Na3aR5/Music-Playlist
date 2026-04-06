#ifndef MUSIC_PLAYLIST_BACKEND_CONSOLE_HEADER
#define MUSIC_PLAYLIST_BACKEND_CONSOLE_HEADER

#include <musicplaylist/Backend.h>
#include <musicplaylist/Config.h>

namespace mspl {
	class BackendConsole : public IBackend {
	public:
		virtual void HandleEvents() override;
		virtual void Update() override;
		virtual void Render() override;
	};
}

#endif // !MUSIC_PLAYLIST_APP_BACKEND_CONSOLE_HEADER