#ifndef JADE_EVENT_HEADER
#define JADE_EVENT_HEADER

#include <jade/Core.h>

#include <cstdint>
#include <string>

namespace jade {
	enum class KeyModifier : uint32_t {
		None = 0x0,

		LShift = 0x1, LCtrl = 0x2, LAlt = 0x4,
		RShift = 0x8, RCtrl = 0x10, RAlt = 0x20,
		Shift = 0x40, Ctrl = 0x80, Alt = 0x100
	};

	inline KeyModifier operator|(KeyModifier left, KeyModifier right) {
		return (KeyModifier)((uint32_t)left | (uint32_t)right);
	}
	inline KeyModifier operator&(KeyModifier left, KeyModifier right) {
		return (KeyModifier)((uint32_t)left & (uint32_t)right);
	}

	enum class Key {
		None,

		A, B, C, D, E, F, G, H, I, J, K, L, M,
		N, O, P, Q, R, S, T, U, V, W, X, Y, Z,

		N0, N1, N2, N3, N4, N5, N6, N7, N8, N9,

		Enter, Escape, Space, Backspace, Tab,
		Left, Right, Up, Down,
		LShift, LCtrl, LAlt, RShift, RCtrl, RAlt, CapsLock,

		F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,

		Minus, // minus | underscore
		Colon, // colon | semicolon
		Quote, // single quote | double quote
		Comma, // comma | '<'
		Dot,   // dot | '>'

		Count
	};

	struct OnTaskEnded {
		enum class Status : uint8_t {
			None,
			Success,
			Failed,
			Cancelled
		};

		Status	     status   = Status::None;
		TaskType     whatTask = TaskType::None;
		TaskCategory category = TaskCategory::None;
		std::string  errorMsg;
	};

	struct OnAsyncTaskEnded {
		OnTaskEnded::Status			status   = OnTaskEnded::Status::None;
		TaskType				    whatTask = TaskType::None;
		TaskCategory			    category = TaskCategory::None;
		std::shared_ptr<FutureTask> task;
		std::string				    errorMsg;
	};

	struct OnKeyAction {
	public:
		bool	    pressed;
		Key			key;
		KeyModifier mods;
	};
	
	struct OnApplicationClose {
		enum : uint8_t {
			ShouldClose,
			WaitForOthers,
			LibraryChangesUnsaved
		};

		uint8_t closeState = ShouldClose;
	};
}

#endif // !JADE_EVENT_HEADER