
#include "values_list.h"


// Функция - обработчик сообщений диалога выбора типа параметра реестра
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
			// Заполняем список
			throwerr(ListBox_AddString(hwnd_list, "Двоичные данные") != LB_ERR, "ChooseParamTypeDlgProc: ListBox_AddString error");
			throwerr(ListBox_AddString(hwnd_list, "DWORD") != LB_ERR, "ChooseParamTypeDlgProc: ListBox_AddString error");
			throwerr(ListBox_AddString(hwnd_list, "QWORD") != LB_ERR, "ChooseParamTypeDlgProc: ListBox_AddString error");
			throwerr(ListBox_AddString(hwnd_list, "Строка") != LB_ERR, "ChooseParamTypeDlgProc: ListBox_AddString error");
			throwerr(ListBox_AddString(hwnd_list, "Многострочная строка") != LB_ERR, "ChooseParamTypeDlgProc: ListBox_AddString error");
			throwerr(ListBox_AddString(hwnd_list, "Расширяемая строка") != LB_ERR, "ChooseParamTypeDlgProc: ListBox_AddString error");
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
					// Помещаем в param_type_edit номер выбранной строки в списке
					param_type_edit = ListBox_GetCurSel(hwnd_list);
					if (param_type_edit == LB_ERR)
					{
						MessageBox(hwnd, "Не выбран тип значения", "Ошибка", MB_OK);
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
	// Логирование исключений
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
