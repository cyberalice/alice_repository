
#include "main.h"
#include <string>

using namespace std;


// ���������� ����������
HINSTANCE hinstance;	// HINSTANCE ����������
HWND hwnd_main;			// HWND �������� ����

INT_PTR CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);	// ������� - ���������� ��������� �������� ����


int WINAPI WinMain(HINSTANCE hinst, HINSTANCE hprevinst, LPSTR cmd, int cmdShow)
{
	hinstance = hinst;
	hwnd_main = NULL;

	try
	{
		// ������������� Common Controls
		INITCOMMONCONTROLSEX comm;
		comm.dwSize = sizeof(comm);
		comm.dwICC = ICC_TREEVIEW_CLASSES;	// TreeView
		throwerr(InitCommonControlsEx(&comm), "InitCommonControlsEx error");

		// ������ �������� ����
		DialogBox(hinstance, "MainDialog", NULL, DlgProc);
	}
	// ����������� ����������
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


// ������� ���������� � ���������� �������, ���� ������� condition �� �����������.
// ������������� ����� ������������� ������ � ��������� ������ �� ����������� error_code ���� �� GetLastError
void throwerr(bool condition, const char* message, bool get_last_error, DWORD error_code)
{
	if (!condition)
	{
		if (!get_last_error && error_code == ERROR_SUCCESS)
			throw MyException(message);
		// �������������� ������ �� error_code ���� GetLastError
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


// ������� - ���������� ��������� �������� ����
INT_PTR CALLBACK DlgProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	try
	{
		switch (msg)
		{
		case WM_INITDIALOG:
			hwnd_main = hwnd;	// ��������� hwnd_main
			TreeViewInit();		// �������������� TreeView
			ValuesListInit();	// ������������� ListView
			return TRUE;
		case WM_CLOSE:			// ��� ������� �� "�������" ��������� ����
			EndDialog(hwnd, 0);
			return TRUE;
		case WM_NOTIFY:
		{
			NMHDR* nmhdr = (NMHDR*)lp;
			if (nmhdr->idFrom == IDC_TREEVIEW)
				// �������������� ��������� WM_NOTIFY � ��������������� ���������� TreeView
				return TreeViewProcessNotification(nmhdr);
			return FALSE;
		}
		case WM_COMMAND:
			// �������������� ��������� WM_COMMAND (� �������� ��� BN_CLICKED) � ��������������� ����������� TreeView � ListView
			if (TreeViewProcessButton(wp, lp))
				return TRUE;
			if (ValuesListProcessButton(wp, lp))
				return TRUE;
			return FALSE;
		}
		return FALSE;
	}
	// ����������� ���������� � ���������� ��������
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
