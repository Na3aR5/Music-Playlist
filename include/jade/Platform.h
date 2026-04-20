#ifndef JADE_PLATFORM_HEADER
#define JADE_PLATFORM_HEADER

#include <string>

namespace jade {
	bool IsConsoleWindowFocused();
	std::string GetClipboardTextContent();
}

#endif // !JADE_PLATFORM_HEADER