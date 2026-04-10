#include <jade/App.h>
#include <jade/backend/BackendConsole.h>

#include <thread>
#include <stdexcept>

namespace {
	jade::Application* g_Application = nullptr;
}

jade::Application::Application() : m_eventSystem(), m_musicDatabase() {
	if (g_Application != nullptr) {
		throw std::runtime_error("Application is already created");
	}
	switch (Config::BackendType) {
		case Config::Backend::CONSOLE: {
			m_backend = (BackendConsole*)::operator new(sizeof(BackendConsole));
			new (m_backend) BackendConsole();
			break;
		}
	}

	m_eventSystem.Subscribe<OnApplicationClose>(0, [this](OnApplicationClose e) {
		if (e.closeState == e.ShouldClose) {
			m_states |= State::STOPPED_BIT;
			m_states &= ~State::STARTED_BIT;
		}
	});

	g_Application = this;
}

jade::Application::~Application() {
	if (m_backend != nullptr) {
		m_backend->~IBackend();
		::operator delete (m_backend);
		m_backend = nullptr;
	}
	g_Application = nullptr;
}

jade::Application& jade::Application::Get() {
	return *g_Application;
}

const jade::Application& jade::Application::GetConst() {
	return *g_Application;
}

void jade::Application::MainLoop() {
	if (m_states & State::STARTED_BIT) {
		return;
	}
	m_states |= State::STARTED_BIT;

	while (m_states & State::STARTED_BIT) {
		m_backend->Update();
		m_backend->Render();
		m_eventSystem.Dispatch();

		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}

void jade::Application::CloseRequest() {
	EventEmitter<OnApplicationClose>().Emit(OnApplicationClose{
		.closeState = OnApplicationClose::ShouldClose	
	});
}

void jade::Application::CloseUnsavedRequest() {
	m_states |= State::STOPPED_BIT;
	m_states &= ~State::STARTED_BIT;
}