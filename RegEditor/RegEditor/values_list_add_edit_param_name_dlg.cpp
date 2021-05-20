
#include "values_list.h"


INT_PTR CALLBACK AddEditParamNameDlgProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	try
	{
		switch (msg)
		{
		case WM_INITDIALOG:
			// ��������� ����� �� param_name_edit � ���� ��������������
			throwerr(SetDlgItemText(hwnd, IDC_PARAM_NAME_EDIT, &param_name_edit[0]) != 0, "AddEditParamNameDlgProc: SetDlgItemText error");
			return TRUE;
		case WM_CLOSE:
			EndDialog(hwnd, IDCANCEL);
			return TRUE;
		case WM_COMMAND:
			if (HIWORD(wp) == BN_CLICKED)
			{
				if (LOWORD(wp) == IDOK)
				{
					HWND hwnd_param_name = GetDlgItem(hwnd, IDC_PARAM_NAME_EDIT);
					throwerr(hwnd_param_name != NULL, "AddEditParamNameDlgProc: GetDlgItem error");
					// �������� ����� ���������� �����
					int text_len = GetWindowTextLength(hwnd_param_name);
					// ������ �� ������ ����, ������� ��������������
					if (text_len == 0)
					{
						MessageBox(hwnd, "������� ������ ��� ���������", "������", MB_OK);
						return TRUE;
					}
					// �������� � param_name_edit
					param_name_edit.assign(text_len, ' ');
					GetWindowText(hwnd_param_name, &param_name_edit[0], text_len + 1);
					// ����� �� �������
					EndDialog(hwnd, IDOK);
					return TRUE;
				}
				if (LOWORD(wp) == IDCANCEL)
				{
					EndDialog(hwnd, IDCANCEL);
					return TRUE;
				}
			}
			return FALSE;
		}
		return FALSE;
	}
	// ����������� ����������
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
