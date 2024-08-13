#include <Windows.h>
#include "AppController.hpp"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	AppController& theApp{ AppController::GetAppController(hInstance) };
	return theApp.Run();
}
