
#include "main.h"
#include <string>

using namespace std;


// Глобальные переменные
HINSTANCE hinstance;	// HINSTANCE приложения
HWND hwnd_main;			// HWND главного окна

INT_PTR CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);	// Функция - обработчик сообщений главного окна


int WINAPI WinMain(HINSTANCE hinst, HINSTANCE hprevinst, LPSTR cmd, int cmdShow)
{
	hinstance = hinst;
	hwnd_main = NULL;

	try
	{
		// Инициализация Common Controls
		INITCOMMONCONTROLSEX comm;
		comm.dwSize = sizeof(comm);
		comm.dwICC = ICC_TREEVIEW_CLASSES;	// TreeView
		throwerr(InitCommonControlsEx(&comm), "InitCommonControlsEx error");

		// Запуск главного окна
		DialogBox(hinstance, "MainDialog", NULL, DlgProc);
	}
	// Логирование исключений
	catch (MyException ex)
	{
		ex.log();
	}
	catch (exception ex)
	{
		log(ex.what());
	}
	catch (...)
	{
		log("Unknown exception");
	}
	
	return 0;
}


// Бросает исключение с переданным текстом, если условие condition не выполняется.
// Дополнительно может форматировать строку с описанием ошибки из переданного error_code либо из GetLastError
void throwerr(bool condition, const char* message, bool get_last_error, DWORD error_code)
{
	if (!condition)
	{
		if (!get_last_error && error_code == ERROR_SUCCESS)
			throw MyException(message);
		// Форматирование ошибки из error_code либо GetLastError
		DWORD error = get_last_error ? GetLastError() : error_code;
		void* msgbuf;
		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			error,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR)&msgbuf,
			0, NULL);
		char* buffer = new char[10000];
		sprintf(buffer, "%s; GetLastError = %d; Error = %s", message, error, (char*)msgbuf);
		throw MyException(buffer);
	}
}


// Функция - обработчик сообщений главного окна
INT_PTR CALLBACK DlgProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	try
	{
		switch (msg)
		{
		case WM_INITDIALOG:
			hwnd_main = hwnd;	// Сохраняем hwnd_main
			TreeViewInit();		// Инициализациия TreeView
			ValuesListInit();	// Инициализация ListView
			return TRUE;
		case WM_CLOSE:			// При нажатии на "крестик" закрываем окно
			EndDialog(hwnd, 0);
			return TRUE;
		case WM_NOTIFY:
		{
			NMHDR* nmhdr = (NMHDR*)lp;
			if (nmhdr->idFrom == IDC_TREEVIEW)
				// Перенаправляем сообщение WM_NOTIFY в соответствующий обработчик TreeView
				return TreeViewProcessNotification(nmhdr);
			return FALSE;
		}
		case WM_COMMAND:
			// Перенаправляем сообщения WM_COMMAND (в основном это BN_CLICKED) в соответствующие обработчики TreeView и ListView
			if (TreeViewProcessButton(wp, lp))
				return TRUE;
			if (ValuesListProcessButton(wp, lp))
				return TRUE;
			return FALSE;
		}
		return FALSE;
	}
	// Логирование исключений и завершение процесса
	catch (MyException ex)
	{
		ex.log();
		ExitProcess(-1);
	}
	catch (exception ex)
	{
		log(ex.what());
		ExitProcess(-1);
	}
	catch (...)
	{
		log("Unknown exception");
		ExitProcess(-1);
	}
}
