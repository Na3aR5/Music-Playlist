#ifndef JADE_INPUT_SYSTEM_HEADER
#define JADE_INPUT_SYSTEM_HEADER

#include <jade/Core.h>
#include <jade/Event.h>

#include <vector>
#include <chrono>

namespace jade {
	class InputSystem {
	public:
		struct KeyState {
			bool		pressed = false;
			KeyModifier mods    = KeyModifier::None;
			Timestep    delay   = 0.0;
		};

		struct UpdateContext {
		public:
			Timestep		       deltaTime;
			std::vector<KeyState>& keyPressedState;
		};

	public:
		InputSystem();
		~InputSystem();

		static InputSystem& Get() noexcept;
		static const InputSystem& GetConst() noexcept;

	public:
		void Update(Timestep deltaTime);

		char KeyToChar(Key, KeyModifier) const noexcept;

		void GenerateKeyPressed(
			Key key,
			bool pressed,
			KeyModifier mods
		) const noexcept;

	private:
		std::vector<KeyState> m_keyPressedState;
	};
}

#endif // !JADE_INPUT_SYSTEM_HEADER