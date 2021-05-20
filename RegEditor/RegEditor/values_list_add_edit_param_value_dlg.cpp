
#include "values_list.h"


INT_PTR CALLBACK AddEditParamValueDlgProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	try
	{
		switch (msg)
		{
		case WM_INITDIALOG:
		{
			HWND hwnd_param_value = GetDlgItem(hwnd, IDC_PARAM_VALUE_EDIT);
			throwerr(hwnd_param_value != NULL, "AddEditParamValueDlgProc: GetDlgItem error");
			int repr_len;
			string repr;
			// 0=bin, 1=DWORD, 2=QWORD, 3=SZ, 4=MULTI_SZ, 5=EXPAND_SZ
			switch (param_type_edit)
			{
			case 0:
			{
				BYTE* param_value_ptr = param_value_edit.size() == 0 ? NULL : &param_value_edit[0];
				repr_len = 2 * param_value_size_edit;
				repr.assign(repr_len, ' ');
				// Строковое представление двоичного параметра поместим в repr
				GetBinaryValueRepr(param_value_ptr, param_value_size_edit, &repr[0]);
				break;
			}
			case 1:
			{
				// В поле редактирования установим стиль "number", т.е. разрешен ввод только цифр
				auto style = GetWindowLong(hwnd_param_value, GWL_STYLE);
				SetWindowLong(hwnd_param_value, GWL_STYLE, style | ES_NUMBER);
				// Выделяем место с запасом
				repr_len = 15;
				repr.assign(repr_len, ' ');
				// В repr поместим строковое представление DWORD
				sprintf(&repr[0], "%u", *((DWORD*)&param_value_edit[0]));
				break;
			}
			case 2:
			{
				// В поле редактирования установим стиль "number", т.е. разрешен ввод только цифр
				auto style = GetWindowLong(hwnd_param_value, GWL_STYLE);
				SetWindowLong(hwnd_param_value, GWL_STYLE, style | ES_NUMBER);
				// Выделяем место с запасом
				repr_len = 35;
				repr.assign(repr_len, ' ');
				// В repr поместим строковое представление QWORD
				sprintf(&repr[0], "%llu", *((unsigned long long*) & param_value_edit[0]));
				break;
			}
			case 3:
			case 5:
				// Просто скопируем из param_value_edit в repr
				repr_len = param_value_size_edit - 1;
				if (repr_len > 0)
				{
					repr.assign(repr_len, ' ');
					strcpy(&repr[0], (char*)&param_value_edit[0]);
				}
				break;
			default:
				// Этого не должно быть
				throwerr(false, "AddEditParamValueDlgProc: wrong parameter type", false);
			}
			// Установим полученное строковое представление значения из repr в поле ввода
			throwerr(SetWindowText(hwnd_param_value, &repr[0]) != 0, "AddEditParamValueDlgProc: SetWindowText error");
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
					// Запросим размер введенного значения
					int repr_len = GetWindowTextLength(hwnd_param_value);
					string repr(repr_len, ' ');
					GetWindowText(hwnd_param_value, &repr[0], repr_len + 1);
					// Проверка корректности ввода, конвертация repr, установка сконвертированного значения из repr в param_value_edit и param_value_size_edit
					switch (param_type_edit)
					{
					case 0:	// REG_BINARY
						if (repr_len % 2 != 0)
						{
							MessageBox(hwnd, "Неверный формат двоичного значения: нечетное число символов", "Ошибка", MB_OK);
							return TRUE;
						}
						// Выделение места
						param_value_size_edit = repr_len / 2;
						param_value_edit.assign(param_value_size_edit, 0);
						if (param_value_size_edit > 0)
						{
							// Конвертация из строкового представления в двоичное
							int bad_char_index;
							if (!ValueReprToBinary(&repr[0], repr_len, &param_value_edit[0], &bad_char_index))
							{
								char message_buf[150];
								sprintf(message_buf, "Неверный формат двоичного значения: неверный символ %c в позиции %d", repr[bad_char_index], bad_char_index + 1);
								MessageBox(hwnd, message_buf, "Ошибка", MB_OK);
								return TRUE;
							}
						}
						break;
					case 1:	// REG_DWORD
					{
						// Выделение места
						param_value_size_edit = 4;
						param_value_edit.assign(4, 0);
						size_t end_pos;
						// Конвертирование из строкового представления в DWORD
						try
						{
							*((DWORD*)&param_value_edit[0]) = stoul(repr, &end_pos);
						}
						catch (invalid_argument)
						{
							MessageBox(hwnd, "Неверный формат значения", "Ошибка", MB_OK);
							return TRUE;
						}
						catch (out_of_range)
						{
							MessageBox(hwnd, "Слишком большое значение", "Ошибка", MB_OK);
							return TRUE;
						}
						// Не вся строка сконвертировалась, т.е. в строке есть посторонние нецифровые символы
						if (end_pos != repr.length())
						{
							MessageBox(hwnd, "Неверный формат", "Ошибка", MB_OK);
							return TRUE;
						}
						break;
					}
					case 2:	// REG_QWORD
					{
						// Аналогично DWORD, только 8 байт
						param_value_size_edit = 8;
						param_value_edit.assign(8, 0);
						size_t end_pos;
						try
						{
							*((unsigned long long*) & param_value_edit[0]) = stoull(repr, &end_pos);
						}
						catch (invalid_argument)
						{
							MessageBox(hwnd, "Неверный формат значения", "Ошибка", MB_OK);
							return TRUE;
						}
						catch (out_of_range)
						{
							MessageBox(hwnd, "Слишком большое значение", "Ошибка", MB_OK);
							return TRUE;
						}
						if (end_pos != repr.length())
						{
							MessageBox(hwnd, "Неверный формат", "Ошибка", MB_OK);
							return TRUE;
						}
						break;
					}
					case 3:	// REG_SZ
					case 5:	// REG_MULTI_SZ
						// Просто копируем
						param_value_size_edit = repr_len + 1;
						param_value_edit.assign(param_value_size_edit, 0);
						strcpy((char*)&param_value_edit[0], &repr[0]);
						break;
					}
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
