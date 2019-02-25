#include "WindowsTarget.h"
#include <tchar.h>
#include "MainDialog.h"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	MainDialog Main(hInstance);
	Main.Show(nCmdShow);
	
	MSG msg;
	BOOL GetMessageResult;
	while ((GetMessageResult = GetMessage(&msg, 0, 0, 0)) > 0)
	{
		if (!IsDialogMessage(Main.GetWindowHandle(), &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	return GetMessageResult;
}
