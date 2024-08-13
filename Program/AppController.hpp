#include <Windows.h>
#include "ActionDialog.h"
#include "MainDialog.h"

class AppController
{
private:
	HINSTANCE _appInstance{ 0 };
	MainDialog _mainDialog;
	ActionDialog _actionDialog;

public:
	AppController(HINSTANCE hInstance) noexcept :
		_appInstance(hInstance), _mainDialog(hInstance), _actionDialog(hInstance) {}
	AppController(const AppController&) = delete;
	AppController& operator=(const AppController&) = delete;
	AppController(AppController&&) = default;
	AppController& operator=(AppController&&) = default;
	~AppController() = default;

	BOOL Run() noexcept;
};
