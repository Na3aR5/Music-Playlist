#ifndef JADE_APPLICATION_HEADER
#define JADE_APPLICATION_HEADER

#include <jade/Config.h>
#include <jade/Event.h>
#include <jade/EventSystem.h>
#include <jade/MusicDatabase.h>
#include <jade/backend/Backend.h>

namespace jade {
	class Application {
	public:
		enum State {
			STARTED_BIT = 0x1,
			STOPPED_BIT = 0x2
		};

	public:
		Application();
		~Application();

	public:
		static Application& Get();
		static const Application& GetConst();

	public:
		void MainLoop();
		void CloseRequest();
		void CloseUnsavedRequest();

	private:
		uint32_t	  m_states = 0;
		IBackend*     m_backend = nullptr;
		EventSystem   m_eventSystem;
		MusicDatabase m_musicDatabase;
	};
}

#endif // !JADE_APPLICATION_HEADER