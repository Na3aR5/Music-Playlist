#include <musicplaylist/App.h>
#include <musicplaylist/BackendConsole.h>
#include <thread>

namespace {
	mspl::Application* g_Application = nullptr;
}

mspl::Application::Application() : m_config(), m_database(m_config.DATABASE_FILEPATH) {
	if (g_Application) {
		return;
	}
	switch (m_config.BACKEND_TYPE) {
	case BackendType::CONSOLE:
		m_backend = (BackendConsole*)::operator new(sizeof(BackendConsole));
		new (m_backend) BackendConsole();
		break;

	default:
		break;
	}
	g_Application = this;
}

mspl::Application::~Application() {
	if (m_backend) {
		m_backend->~IBackend();
		::operator delete(m_backend);
		m_backend = nullptr;
	}
}

mspl::Application* mspl::Application::Get() noexcept {
	return g_Application;
}

mspl::Database& mspl::Application::Database() noexcept {
	return m_database;
}

const mspl::Database& mspl::Application::Database() const noexcept {
	return m_database;
}

mspl::Player& mspl::Application::Player() noexcept {
	return m_player;
}

const mspl::Player& mspl::Application::Player() const noexcept {
	return m_player;
}

void mspl::Application::MainLoop() {
	m_states |= State::STARTED_BIT;

	while (m_states & State::STARTED_BIT) {
		_Update();
		_Render();
		_HandleEvents();

		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}

void mspl::Application::StopLoop() {
	m_states &= ~State::STARTED_BIT;
}

void mspl::Application::_HandleEvents() {
	m_backend->HandleEvents();
}

void mspl::Application::_Update() {
	m_backend->Update();
}

void mspl::Application::_Render() {
	m_backend->Render();
}