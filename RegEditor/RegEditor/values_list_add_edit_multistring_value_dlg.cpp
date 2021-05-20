
#include "values_list.h"


// ‘ункци€ - обработчик сообщений диалога редактировани€ многострочного параметра реестра
INT_PTR CALLBACK AddEditMultistringValueDlgProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	try
	{
		switch (msg)
		{
		case WM_INITDIALOG:
		{
			HWND hwnd_param_value = GetDlgItem(hwnd, IDC_PARAM_VALUE_EDIT);
			throwerr(hwnd_param_value != NULL, "AddEditMultistringValueDlgProc: GetDlgItem error");
			// ѕроверки на вс€кий случай: размер параметра должен быть > 0, последний байт должен быть 0
			throwerr(param_value_size_edit > 0, "AddEditMultistringValueDlgProc: internal error (param_value_size_edit must be > 0)", false);
			throwerr(param_value_edit[param_value_size_edit - 1] == 0, "AddEditMultistringValueDlgProc: internal error (param_value_edit[param_value_size_edit-1] must be 0)", false);
			
			// ¬ многострочном параметре строки разделены нулевым байтом, а в самом конце два нулевых байта.
			// „тобы это всЄ корректно отображалось в редакторе, нужно заменить все (кроме последнего) нулевые байты на символ переноса строки, \r\n
			// “ак как каждый нулевой байт замен€етс€ на два, необходимо выделить дополнительное место
			
			// ѕодсчитаем число нулевых байт, кроме последнего
			auto null_count = count(param_value_edit.begin(), param_value_edit.end(), 0) - 1;
			// ¬ыделим переменную repr, скопируем туда значение параметра
			string repr(param_value_size_edit - 1, ' ');
			memcpy(&repr[0], (char*)&param_value_edit[0], param_value_size_edit);
			// «арезервируем необходимое дополнительное место (небольша€ оптимизаци€, этого можно и не делать)
			repr.reserve(param_value_size_edit - 1 + null_count);
			// «аменим 0-байты на \r\n
			size_t start_pos = 0;
			while ((start_pos = repr.find('\0', start_pos)) != string::npos) {
				repr.replace(start_pos, 1, "\r\n");
				start_pos += 2;
			}
			// ѕолученный текст установим в поле редактировани€
			throwerr(SetWindowText(hwnd_param_value, &repr[0]) != 0, "AddEditMultistringValueDlgProc: SetWindowText error");
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
					HWND hwnd_param_value = GetDlgItem(hwnd, IDC_PARAM_VALUE_EDIT);
					throwerr(hwnd_param_value != NULL, "AddEditParamValueDlgProc: GetDlgItem error");
					// «апросим размер значени€, введенного пользователем в поле редактировани€
					int repr_len = GetWindowTextLength(hwnd_param_value);
					// ¬ыделим переменную repr
					string repr(repr_len, ' ');
					// «апросим значение из пол€ редактировани€ в переменную repr
					GetWindowText(hwnd_param_value, &repr[0], repr_len + 1);
					// «аменим переносы строк \r\n на нулевые байты
					size_t start_pos = 0;
					while ((start_pos = repr.find("\r\n", start_pos)) != string::npos) {
						repr.replace(start_pos, 2, 1, '\0');
						start_pos += 1;
					}
					// ¬ мультистроке не должно быть пустых строк (так требует документаци€ WinAPI), т.е. не должно быть два подр€д нулевых байта (только в самом конце)
					string zz(2, 0);
					if (repr.find(zz) != string::npos)
					{
						MessageBox(hwnd, "ћультистрока не должна содержать пустых строк", "ќшибка", MB_OK);
						return TRUE;
					}
					// ≈сли пользователь в самом конце не поставил перенос строки, у нас после последней строки не будет закрывающего нуль-байта. Ќужно его добавить
					if (repr.length() == 0 || repr[repr.length() - 1] != 0)
						repr.append(1, 0);
					// ѕреобразованное значение помещаем в param_value_edit и его размер соответственно в param_value_size_edit
					param_value_size_edit = repr.length() + 1;
					param_value_edit.assign(param_value_size_edit, 0);
					memcpy((char*)&param_value_edit[0], &repr[0], param_value_size_edit);
					// «авершаем диалог
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
	// Ћогирование исключений
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
