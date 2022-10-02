#include <SDKDDKVer.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <cstdlib>
#include <Volk/volk.h>

LRESULT CALLBACK wndProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	if (message == WM_DESTROY) {
		DestroyWindow(window);
		PostQuitMessage(EXIT_SUCCESS);
	}

	return DefWindowProc(window, message, wparam, lparam);
}

int APIENTRY wWinMain(_In_ HINSTANCE app_instance, _In_opt_ HINSTANCE prev_app_instance,
	_In_ LPWSTR cmd_line, _In_ int cmd_show)
{
	UNREFERENCED_PARAMETER(prev_app_instance);
	UNREFERENCED_PARAMETER(cmd_line);

	if (volkInitialize() != VK_SUCCESS) {
		MessageBox(nullptr, L"Vulkan not found on this system.", L"ERROR", MB_ICONERROR | MB_OK | MB_SYSTEMMODAL);
		return EXIT_FAILURE;
	}

	uint32_t supported_vk_version = volkGetInstanceVersion();
	if (supported_vk_version == 0) {
		MessageBox(nullptr, L"Failed to read supported Vulkan version.", L"ERROR", MB_ICONERROR | MB_OK | MB_SYSTEMMODAL);
		return EXIT_FAILURE;
	}

	if ((VK_VERSION_MAJOR(supported_vk_version) != 1) || (VK_VERSION_MINOR(supported_vk_version) > 3)) {
		MessageBox(nullptr, L"Unsupported Vulkan version.", L"ERROR", MB_ICONERROR | MB_OK | MB_SYSTEMMODAL);
		return EXIT_FAILURE;
	}

	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pNext = NULL;
	appInfo.pApplicationName = "Simulator";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "none";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_MAKE_VERSION(1, 3, 0);

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
		MessageBox(nullptr, L"Cannot register window class.", L"ERROR", MB_ICONERROR | MB_OK | MB_SYSTEMMODAL);
		return EXIT_FAILURE;
	}

	HWND window = CreateWindowW(L"MainWindow", L"Simulator", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, app_instance, nullptr);

	if (window == nullptr) {
		MessageBox(nullptr, L"Cannot create main window.", L"ERROR", MB_ICONERROR | MB_OK | MB_SYSTEMMODAL);
		return EXIT_FAILURE;
	}

	ShowWindow(window, cmd_show);

	if (!UpdateWindow(window)) {
		MessageBox(nullptr, L"Cannot update main window.", L"ERROR", MB_ICONERROR | MB_OK | MB_SYSTEMMODAL);
		return EXIT_FAILURE;
	}

	MSG message;
	while (GetMessage(&message, nullptr, 0, 0)) {
		TranslateMessage(&message);
		DispatchMessage(&message);
	}

	return (int)message.wParam;
}