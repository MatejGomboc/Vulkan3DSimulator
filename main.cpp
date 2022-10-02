#include <SDKDDKVer.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <cstdlib>
#include "renderer.h"

static LRESULT CALLBACK wndProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	switch (message) {
	case WM_CLOSE:
		DestroyWindow(window);
		return 0;
	case WM_DESTROY:
		PostQuitMessage(EXIT_SUCCESS);
		return 0;
	default:
		return DefWindowProc(window, message, wparam, lparam);
	}
}

int APIENTRY wWinMain(_In_ HINSTANCE app_instance, _In_opt_ HINSTANCE prev_app_instance,
	_In_ LPWSTR cmd_line, _In_ int cmd_show)
{
	UNREFERENCED_PARAMETER(prev_app_instance);
	UNREFERENCED_PARAMETER(cmd_line);

	WNDCLASSEXW window_class{};
	window_class.cbSize = sizeof(WNDCLASSEX);
	window_class.style = CS_HREDRAW | CS_VREDRAW;
	window_class.lpfnWndProc = wndProc;
	window_class.cbClsExtra = 0;
	window_class.cbWndExtra = 0;
	window_class.hInstance = app_instance;
	window_class.hIcon = nullptr;
	window_class.hCursor = LoadCursor(nullptr, IDC_ARROW);
	window_class.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	window_class.lpszMenuName = nullptr;
	window_class.lpszClassName = TEXT("MainWindow");
	window_class.hIconSm = nullptr;

	if (RegisterClassEx(&window_class) == 0) {
		MessageBox(nullptr, TEXT("Cannot register window class."), TEXT("ERROR"), MB_ICONERROR | MB_OK | MB_SYSTEMMODAL);
		return EXIT_FAILURE;
	}

	HWND window = CreateWindow(TEXT("MainWindow"), TEXT("Simulator"), WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, app_instance, nullptr);

	if (window == nullptr) {
		MessageBox(nullptr, TEXT("Cannot create main window."), TEXT("ERROR"), MB_ICONERROR | MB_OK | MB_SYSTEMMODAL);
		return EXIT_FAILURE;
	}

	ShowWindow(window, cmd_show);

	MSG message;
	while (GetMessage(&message, nullptr, 0, 0)) {
		TranslateMessage(&message);
		DispatchMessage(&message);
	}

	return (int)message.wParam;
}