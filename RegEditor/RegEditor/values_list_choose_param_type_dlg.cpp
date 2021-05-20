
#include "values_list.h"


// ������� - ���������� ��������� ������� ������ ���� ��������� �������
INT_PTR CALLBACK ChooseParamTypeDlgProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	try
	{
		switch (msg)
		{
		case WM_INITDIALOG:
		{
			HWND hwnd_list = GetDlgItem(hwnd, IDC_VALUE_TYPE_LIST);
			throwerr(hwnd_list != NULL, "ChooseParamTypeDlgProc: GetDlgItem error");
			// ��������� ������
			throwerr(ListBox_AddString(hwnd_list, "�������� ������") != LB_ERR, "ChooseParamTypeDlgProc: ListBox_AddString error");
			throwerr(ListBox_AddString(hwnd_list, "DWORD") != LB_ERR, "ChooseParamTypeDlgProc: ListBox_AddString error");
			throwerr(ListBox_AddString(hwnd_list, "QWORD") != LB_ERR, "ChooseParamTypeDlgProc: ListBox_AddString error");
			throwerr(ListBox_AddString(hwnd_list, "������") != LB_ERR, "ChooseParamTypeDlgProc: ListBox_AddString error");
			throwerr(ListBox_AddString(hwnd_list, "������������� ������") != LB_ERR, "ChooseParamTypeDlgProc: ListBox_AddString error");
			throwerr(ListBox_AddString(hwnd_list, "����������� ������") != LB_ERR, "ChooseParamTypeDlgProc: ListBox_AddString error");
			return TRUE;
		}
		case WM_CLOSE:
			EndDialog(hwnd, IDCANCEL);
			return TRUE;
		case WM_COMMAND:
			if (HIWORD(wp) == BN_CLICKED)
			{
				if (LOWORD(wp) == IDOK)
				{
					HWND hwnd_list = GetDlgItem(hwnd, IDC_VALUE_TYPE_LIST);
					throwerr(hwnd_list != NULL, "ChooseParamTypeDlgProc: GetDlgItem error");
					// �������� � param_type_edit ����� ��������� ������ � ������
					param_type_edit = ListBox_GetCurSel(hwnd_list);
					if (param_type_edit == LB_ERR)
					{
						MessageBox(hwnd, "�� ������ ��� ��������", "������", MB_OK);
						return TRUE;
					}
					EndDialog(hwnd, IDOK);
					return TRUE;
				}
				if (LOWORD(wp) == IDCANCEL)
				{
					param_type_edit = LB_ERR;
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
