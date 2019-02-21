#include <Windows.h>
#include <tchar.h>
#include <string>
#include <utility>
#include "resource.h"
#include "ComputerCertificate.h"
#include "InstallInfo.h"
#include "ErrorRecord.h"

#include "MainDialog.h"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	auto[RegistryError, MSICmdline] = GetMSIModifyPath();
	auto r = RegistryError.GetErrorCode();
	//if (RegistryError.GetErrorCode() > 0)
	//{
	//	MessageBox(NULL, FormatErrorForDialog(
	//		RegistryError.GetErrorCode(), RegistryError.GetErrorMessage(), RegistryError.GetActivity()).c_str(),
	//		L"Installation Detection Error", MB_OK);
	//}

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
