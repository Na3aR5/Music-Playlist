#ifndef JADE_BACKEND_INTERFACE_HEADER
#define JADE_BACKEND_INTERFACE_HEADER

namespace jade {
	class IBackend {
	public:
		virtual ~IBackend() = default;

		virtual void Update() = 0;
		virtual void Render() = 0;
	};
}

#endif // !JADE_BACKEND_INTERFACE_HEADER