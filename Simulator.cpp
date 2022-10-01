#include <SDKDDKVer.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <Volk/volk.h>

LRESULT CALLBACK wndProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	if (message == WM_DESTROY) {
		DestroyWindow(window);
		PostQuitMessage(0);
	}

	return DefWindowProc(window, message, wparam, lparam);
}

int APIENTRY wWinMain(_In_ HINSTANCE app_instance, _In_opt_ HINSTANCE prev_app_instance,
	_In_ LPWSTR cmd_line, _In_ int cmd_show)
{
	UNREFERENCED_PARAMETER(prev_app_instance);
	UNREFERENCED_PARAMETER(cmd_line);

	if (volkInitialize() != VK_SUCCESS) {
		MessageBoxW(nullptr, L"Vulkan not found on this system.", L"ERROR", MB_ICONERROR | MB_OK | MB_SYSTEMMODAL);
		return 1;
	}

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
	window_class.lpszClassName = L"MainWindow";
	window_class.hIconSm = nullptr;

	if (RegisterClassExW(&window_class) == 0) {
		MessageBoxW(nullptr, L"Cannot register window class.", L"ERROR", MB_ICONERROR | MB_OK | MB_SYSTEMMODAL);
		return 1;
	}

	HWND window = CreateWindowW(L"MainWindow", L"Simulator", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, app_instance, nullptr);

	if (window == nullptr) {
		MessageBoxW(nullptr, L"Cannot create main window.", L"ERROR", MB_ICONERROR | MB_OK | MB_SYSTEMMODAL);
		return 1;
	}

	ShowWindow(window, cmd_show);

	if (!UpdateWindow(window)) {
		MessageBoxW(nullptr, L"Cannot update main window.", L"ERROR", MB_ICONERROR | MB_OK | MB_SYSTEMMODAL);
		return 1;
	}

	MSG message;
	while (GetMessage(&message, nullptr, 0, 0)) {
		TranslateMessage(&message);
		DispatchMessage(&message);
	}

	return (int)message.wParam;
}