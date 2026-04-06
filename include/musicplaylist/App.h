#ifndef MUSIC_PLAYLIST_APP_HEADER
#define MUSIC_PLAYLIST_APP_HEADER

#include <musicplaylist/Config.h>
#include <musicplaylist/Backend.h>
#include <musicplaylist/Musicdb.h>
#include <musicplaylist/Player.h>

#include <cstdint>
#include <memory>

namespace mspl {
	class Application {
	public:
		enum State : uint64_t {
			STARTED_BIT = 0x1,
		};

	public:
		Application();
		~Application();

		static Application* Get() noexcept;

		mspl::Database& Database() noexcept;
		const mspl::Database& Database() const noexcept;

		mspl::Player& Player() noexcept;
		const mspl::Player& Player() const noexcept;

		void MainLoop();
		void StopLoop();

	private:
		void _HandleEvents();
		void _Update();
		void _Render();

	private:
		uint64_t m_states = 0;
		Config   m_config = {};
		IBackend* m_backend = nullptr;
		mspl::Player   m_player;
		mspl::Database m_database;

		std::vector<mspl::Playlist> m_playlists;
	};
}

#endif // !MUSIC_PLAYLIST_APP_HEADER