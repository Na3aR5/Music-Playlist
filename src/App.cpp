#include <jade/App.h>
#include <jade/backend/BackendConsole.h>

#include <thread>
#include <stdexcept>

namespace {
	jade::Application* g_Application = nullptr;
}

jade::Application::Application() : m_eventSystem(), m_inputSystem(), m_musicLibrary() {
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
			m_states |= State::StoppedBit;
			m_states &= ~(State::StartedBit | State::WaitForOthersBit);
			return;
		}
		if (e.closeState == e.WaitForOthers) {
			m_states |= State::WaitForOthersBit;
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

jade::Player& jade::Application::Player() { return g_Application->m_player; }
const jade::Player& jade::Application::ConstPlayer() { return g_Application->m_player; }

void jade::Application::MainLoop() {
	if (m_states & State::StartedBit) {
		return;
	}
	m_states |= State::StartedBit;

	auto startTimestamp = std::chrono::high_resolution_clock::now();

	while (m_states & State::StartedBit) {
		auto frameTimestamp = std::chrono::high_resolution_clock::now();
		Timestep deltaTime = std::chrono::duration<double>(frameTimestamp - startTimestamp).count();
		startTimestamp = frameTimestamp;

		m_backend->Render();

		m_inputSystem.Update(deltaTime);
		m_backend->Update(deltaTime);
	
		if (m_states & State::WaitForOthersBit) {
			CloseRequest();
		}
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
	m_states |= State::StoppedBit;
	m_states &= ~State::StartedBit;
}