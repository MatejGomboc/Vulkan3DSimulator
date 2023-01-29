#include <SDKDDKVer.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "renderer.h"

static LRESULT CALLBACK wndProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	switch (message) {
	case WM_CREATE: {
		auto create_info = reinterpret_cast<CREATESTRUCTA*>(lparam);
		auto renderer = reinterpret_cast<Simulator::Renderer*>(create_info->lpCreateParams);

		SetLastError(0);
		if (SetWindowLongPtr(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(renderer)) != 0) {
			return -1;
		}

		if (GetLastError() != ERROR_SUCCESS) {
			return -1;
		}

		return 0;
	}
	case WM_DESTROY: {
		auto renderer = reinterpret_cast<Simulator::Renderer*>(GetWindowLongPtr(window, GWLP_USERDATA));
		if (renderer == nullptr) {
			PostQuitMessage(GetLastError());
			return 0;
		}

		PostQuitMessage(ERROR_SUCCESS);
		return 0;
	}
	default:
		return DefWindowProc(window, message, wparam, lparam);
	}
}

int APIENTRY wWinMain(_In_ HINSTANCE app_instance, _In_opt_ HINSTANCE prev_app_instance, _In_ LPWSTR cmd_line, _In_ int cmd_show)
{
	UNREFERENCED_PARAMETER(prev_app_instance);
	UNREFERENCED_PARAMETER(cmd_line);

	Simulator::Renderer renderer;

	WNDCLASSEX window_class{};
	window_class.cbSize = sizeof(WNDCLASSEX);
	window_class.style = CS_HREDRAW | CS_VREDRAW;
	window_class.lpfnWndProc = wndProc;
	window_class.cbClsExtra = 0;
	window_class.cbWndExtra = sizeof(LONG_PTR);
	window_class.hInstance = app_instance;
	window_class.hIcon = nullptr;
	window_class.hCursor = LoadCursor(nullptr, IDC_ARROW);
	window_class.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	window_class.lpszMenuName = nullptr;
	window_class.lpszClassName = TEXT("MainWindow");
	window_class.hIconSm = nullptr;

	if (RegisterClassEx(&window_class) == 0) {
		return GetLastError();
	}

	HWND main_window = CreateWindow(TEXT("MainWindow"), TEXT("Simulator"), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, nullptr, nullptr, app_instance, &renderer);

	if (main_window == nullptr) {
		return GetLastError();
	}

	ShowWindow(main_window, cmd_show);

	MSG message;
	while (GetMessage(&message, nullptr, 0, 0)) {
		TranslateMessage(&message);
		DispatchMessage(&message);
	}

	return (int)message.wParam;
}