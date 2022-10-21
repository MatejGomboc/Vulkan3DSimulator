#include <SDKDDKVer.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <CommCtrl.h>
#include <cstdlib>
#include "renderer.h"

static Simulator::Renderer renderer;

static LRESULT CALLBACK wndProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	switch (message) {
	case WM_CREATE: {
		auto create_info = reinterpret_cast<CREATESTRUCTA*>(lparam);

		HWND devices_combobox = CreateWindow(WC_COMBOBOX, TEXT("DevicesCombobox"),
			CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE,
			0, 0, 1000, 1000, window, nullptr, create_info->hInstance, nullptr);

		if (devices_combobox == nullptr) {
			MessageBoxA(nullptr, "Cannot create devices combo box.", "ERROR", MB_ICONERROR | MB_OK | MB_APPLMODAL);
			return -1;
		}

		LRESULT error = SendMessage(devices_combobox, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(TEXT("1")));
		if ((error == CB_ERR) || (error == CB_ERRSPACE)) {
			MessageBoxA(nullptr, "Cannot create devices combo box.", "ERROR", MB_ICONERROR | MB_OK | MB_APPLMODAL);
			return -1;
		}

		error = SendMessage(devices_combobox, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(TEXT("2")));
		if ((error == CB_ERR) || (error == CB_ERRSPACE)) {
			MessageBoxA(nullptr, "Cannot create devices combo box.", "ERROR", MB_ICONERROR | MB_OK | MB_APPLMODAL);
			return -1;
		}

		/*error = SendMessage(devices_combobox, CB_SETCURSEL, 0, 0);
		if ((error == CB_ERR) || (error == CB_ERRSPACE)) {
			MessageBoxA(nullptr, "Cannot create devices combo box.", "ERROR", MB_ICONERROR | MB_OK | MB_APPLMODAL);
			return -1;
		}*/

		return 0;
	}
	case WM_CLOSE: {
		DestroyWindow(window);
		return 0;
	}
	case WM_DESTROY: {
		PostQuitMessage(EXIT_SUCCESS);
		return 0;
	}
	default:
		return DefWindowProc(window, message, wparam, lparam);
	}
}

int APIENTRY wWinMain(_In_ HINSTANCE app_instance, _In_opt_ HINSTANCE prev_app_instance,
	_In_ LPWSTR cmd_line, _In_ int cmd_show)
{
	UNREFERENCED_PARAMETER(prev_app_instance);
	UNREFERENCED_PARAMETER(cmd_line);

	std::string out_error_message;
	if (!renderer.init(out_error_message)) {
		MessageBoxA(nullptr, out_error_message.c_str(), "ERROR", MB_ICONERROR | MB_OK | MB_APPLMODAL);
		return EXIT_FAILURE;
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
	window_class.lpszClassName = TEXT("MainWindow");
	window_class.hIconSm = nullptr;

	if (RegisterClassEx(&window_class) == 0) {
		MessageBoxA(nullptr, "Cannot register window class.", "ERROR", MB_ICONERROR | MB_OK | MB_APPLMODAL);
		return EXIT_FAILURE;
	}

	HWND window = CreateWindow(TEXT("MainWindow"), TEXT("Simulator"), WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, nullptr, nullptr, app_instance, nullptr);

	if (window == nullptr) {
		MessageBoxA(nullptr, "Cannot create main window.", "ERROR", MB_ICONERROR | MB_OK | MB_APPLMODAL);
		return EXIT_FAILURE;
	}

	ShowWindow(window, cmd_show);

	MSG message;
	while (GetMessage(&message, nullptr, 0, 0)) {
		TranslateMessage(&message);
		DispatchMessage(&message);
	}

	renderer.destroy();
	return (int)message.wParam;
}