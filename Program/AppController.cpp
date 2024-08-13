#include "AppController.hpp"
#include "MainDialog.h"

BOOL AppController::Run() noexcept
{
	MainDialog Main(_appInstance);
	HWND MainDialogHandle{ Main.Show(SW_SHOWDEFAULT) };
	MSG msg;
	BOOL GetMessageResult;
	while ((GetMessageResult = GetMessage(&msg, 0, 0, 0)) > 0)
	{
		if (!IsDialogMessage(MainDialogHandle, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	return GetMessageResult;
}
