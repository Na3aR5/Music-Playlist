#include <jade/InputSystem.h>
#include <jade/EventSystem.h>
#include <jade/Platform.h>

#include <stdexcept>

namespace {
	jade::Key ConvertNativeKeyCode(int code) noexcept;
	void NativeUpdate(jade::InputSystem::UpdateContext&);
}

namespace {
	jade::InputSystem* g_InputSystem = nullptr;
}

jade::InputSystem::InputSystem() : m_keyPressedState(512) {
	if (g_InputSystem != nullptr) {
		throw std::runtime_error("Attempt to create input system twice");
	}
	g_InputSystem = this;
}

jade::InputSystem::~InputSystem() {
	g_InputSystem = nullptr;
}

jade::InputSystem& jade::InputSystem::Get() noexcept { return *g_InputSystem; }
const jade::InputSystem& jade::InputSystem::GetConst() noexcept { return *g_InputSystem; }

void jade::InputSystem::GenerateKeyPressed(Key key, bool pressed, KeyModifier mods) const noexcept {
	EventEmitter<OnKeyAction>().Emit(OnKeyAction{
		.pressed = pressed,
		.key     = key,
		.mods    = mods
	});
}

char jade::InputSystem::KeyToChar(Key key, KeyModifier mods) const noexcept {
	bool hasShift = (bool)(mods & KeyModifier::Shift);

	if (key >= Key::A && key <= Key::Z) {
		int offset = hasShift ? 'A' : 'a';
		return (char)(offset + ((int)key - (int)Key::A));
	}
	if (key >= Key::N0 && key <= Key::N9) {
		if (hasShift) {
			switch (key) {
				case Key::N1: return '!';
				case Key::N2: return '@';
				case Key::N3: return '#';
				case Key::N4: return '$';
				case Key::N5: return '%';
				case Key::N6: return '^';
				case Key::N7: return '&';
				case Key::N8: return '*';
				case Key::N9: return '(';
				case Key::N0: return ')';
			}
		}
		return (char)('0' + ((int)key - (int)Key::N0));
	}
	switch (key) {
		case Key::Space: return ' ';
		case Key::Minus: return hasShift ? '_' : '-';
		case Key::Colon: return hasShift ? ':' : ';';
		case Key::Quote: return hasShift ? '"' : '\'';
		case Key::Comma: return hasShift ? '<' : ',';
	}
	return '\0';
}

void jade::InputSystem::Update(Timestep deltaTime) {
	UpdateContext ctx = {
		.deltaTime    = deltaTime,
		.keyPressedState = m_keyPressedState
	};
	NativeUpdate(ctx);
}

#if defined(_WIN32) || defined(WIN32)
#include <Windows.h>

namespace {
	jade::Key ConvertNativeKeyCode(int code) noexcept {
		if (code >= (int)'A' && code <= (int)'Z') {
			return (jade::Key)((int)jade::Key::A + (code - (int)'A'));
		}
		if (code >= (int)'0' && code <= (int)'9') {
			return (jade::Key)((int)jade::Key::N0 + (code - (int)'0'));
		}
		switch (code) {
			case VK_SPACE:     return jade::Key::Space;
			case VK_RETURN:    return jade::Key::Enter;
			case VK_ESCAPE:    return jade::Key::Escape;
			case VK_BACK:      return jade::Key::Backspace;
			case VK_TAB:       return jade::Key::Tab;

			case VK_LEFT:      return jade::Key::Left;
			case VK_RIGHT:     return jade::Key::Right;
			case VK_UP:        return jade::Key::Up;
			case VK_DOWN:      return jade::Key::Down;

			case VK_LSHIFT:    return jade::Key::LShift;
			case VK_RSHIFT:    return jade::Key::RShift;
			case VK_LCONTROL:  return jade::Key::LCtrl;
			case VK_RCONTROL:  return jade::Key::RCtrl;
			case VK_LMENU:     return jade::Key::LAlt;
			case VK_RMENU:     return jade::Key::RAlt;

			case VK_OEM_MINUS: return jade::Key::Minus;
			case VK_OEM_1:     return jade::Key::Colon;
			case VK_OEM_7:     return jade::Key::Quote;
			case VK_OEM_COMMA: return jade::Key::Comma;
		}
		return jade::Key::None;
	}

	void NativeUpdate(jade::InputSystem::UpdateContext& ctx) {
		if (!jade::IsConsoleWindowFocused()) {
			return;
		}
		std::vector<jade::InputSystem::KeyState> newState = ctx.keyPressedState;

		for (int vk = 0; vk < 256; ++vk) {
			bool isPressed = (GetAsyncKeyState(vk) & 0x8000) != 0;
			bool wasPressed = ctx.keyPressedState[vk].pressed;

			if (isPressed != wasPressed) {
				jade::Key key = ConvertNativeKeyCode(vk);
				if (key == jade::Key::None) {
					continue;
				}
				jade::KeyModifier mods = jade::KeyModifier::None;

				if (GetAsyncKeyState(VK_LSHIFT) & 0x8000) {
					mods = mods | jade::KeyModifier::LShift | jade::KeyModifier::Shift;
				}
				if (GetAsyncKeyState(VK_RSHIFT) & 0x8000) {
					mods = mods | jade::KeyModifier::RShift | jade::KeyModifier::Shift;
				}
				if (GetAsyncKeyState(VK_LCONTROL) & 0x8000) {
					mods = mods | jade::KeyModifier::LCtrl | jade::KeyModifier::Ctrl;
				}
				if (GetAsyncKeyState(VK_RCONTROL) & 0x8000) {
					mods = mods | jade::KeyModifier::RCtrl | jade::KeyModifier::Ctrl;
				}
				if (GetAsyncKeyState(VK_LMENU) & 0x8000) {
					mods = mods | jade::KeyModifier::LAlt | jade::KeyModifier::Alt;
				}
				if (GetAsyncKeyState(VK_RMENU) & 0x8000) {
					mods = mods | jade::KeyModifier::RAlt | jade::KeyModifier::Alt;
				}
				g_InputSystem->GenerateKeyPressed(key, isPressed, mods);

				newState[vk].pressed = isPressed;
				newState[vk].mods    = mods;
				newState[vk].delay   = 0.45;
			}
			else if (wasPressed) {
				if (ctx.deltaTime >= ctx.keyPressedState[vk].delay) {
					newState[vk].delay = 0.0;
					jade::Key key = ConvertNativeKeyCode(vk);
					g_InputSystem->GenerateKeyPressed(key, isPressed, ctx.keyPressedState[vk].mods);
				}
				else {
					newState[vk].delay -= ctx.deltaTime;
				}
			}
		}
		ctx.keyPressedState = std::move(newState);
	}
}
#endif // WIN32