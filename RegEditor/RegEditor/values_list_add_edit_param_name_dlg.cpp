
#include "values_list.h"


INT_PTR CALLBACK AddEditParamNameDlgProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	try
	{
		switch (msg)
		{
		case WM_INITDIALOG:
			// Установим текст из param_name_edit в поле редактирования
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
					// Запросим длину введенного имени
					int text_len = GetWindowTextLength(hwnd_param_name);
					// Пустым не должно быть, выдадим предупреждение
					if (text_len == 0)
					{
						MessageBox(hwnd, "Следует ввести имя параметра", "Ошибка", MB_OK);
						return TRUE;
					}
					// Сохраним в param_name_edit
					param_name_edit.assign(text_len, ' ');
					GetWindowText(hwnd_param_name, &param_name_edit[0], text_len + 1);
					// Выход из диалога
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
