#ifndef JADE_EVENT_HEADER
#define JADE_EVENT_HEADER

#include <cstdint>

namespace jade {
	// Intents

	struct OnApplicationClose {
		enum : uint8_t {
			ShouldClose,
			DatabaseChangesUnsaved
		};

		uint8_t closeState = ShouldClose;
	};

	// Results

	struct OnGlobalAdded {};
	struct OnGlobalSaved {};
}

#endif // !JADE_EVENT_HEADER