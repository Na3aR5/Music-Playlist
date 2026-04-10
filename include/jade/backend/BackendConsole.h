#ifndef JADE_BACKEND_CONSOLE_HEADER
#define JADE_BACKEND_CONSOLE_HEADER

#include <jade/backend/Backend.h>

namespace jade {
	class BackendConsole : public IBackend {
	public:
		BackendConsole();

	public:
		virtual void Update() override;
		virtual void Render() override;
	};
}

#endif // !JADE_BACKEND_CONSOLE_HEADER