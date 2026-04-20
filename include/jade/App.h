#ifndef JADE_APPLICATION_HEADER
#define JADE_APPLICATION_HEADER

#include <jade/Config.h>
#include <jade/Event.h>
#include <jade/EventSystem.h>
#include <jade/InputSystem.h>
#include <jade/MusicLibrary.h>
#include <jade/backend/Backend.h>

namespace jade {
	class Application {
	public:
		enum State {
			StartedBit		 = 0x1,
			StoppedBit		 = 0x2,
			WaitForOthersBit = 0x4
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
		uint32_t	 m_states = 0;
		IBackend*    m_backend = nullptr;
		EventSystem  m_eventSystem;
		InputSystem  m_inputSystem;
		MusicLibrary m_musicLibrary;
	};
}

#endif // !JADE_APPLICATION_HEADER