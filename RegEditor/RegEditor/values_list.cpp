
#include "values_list.h"

// Глобальные переменные, в них сохраняются: тип добавляемого параметра реестра (используется в диалоге выбора типа параметра), имя добавляемого/редактируемого
// параметра (используется в диалоге ввода имени добавляемого/редактируемого параметра), размер и значение добавляемого/редактируемого параметра
// (используется в диалоге ввода значения параметра).
int param_type_edit = LB_ERR;	// В диалоге выбора типа используются следующие значения: 0=REG_BINARY, 1=REG_DWORD, 2=REG_QWORD, 3=REG_SZ, 4=REG_MULTI_SZ, 5=REG_EXPAND_SZ,
								// Соответственно при вызове функций WinAPI нужно будет конвертировать из такой "кодировки" в ту, которую понимает Windows, и наоборот
string param_name_edit;
int param_value_size_edit;
vector<BYTE> param_value_edit;


// Инициализация ListView
void ValuesListInit()
{
	auto hwnd_vl = GetDlgItem(hwnd_main, IDC_VALUES_LIST);
	throwerr(hwnd_vl != NULL, "GetDlgItem (IDC_VALUES_LIST) error");

	// Добавление колонок в ListView
	LVCOLUMN lvc;
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;	// Используются только поля fmt, cx, pszText, iSubItem
	// Колонка 0 - имя параметра
	lvc.iSubItem = 0;
	lvc.fmt = LVCFMT_LEFT;
	lvc.cx = 100;
	lvc.pszText = (char*)"Имя";
	throwerr(ListView_InsertColumn(hwnd_vl, 0, &lvc) != -1, "ValuesListInit: ListView_InsertColumn(0) error");
	// Колонка 1 - тип параметра
	lvc.iSubItem = 1;
	lvc.cx = 100;
	lvc.pszText = (char*)"Тип";
	throwerr(ListView_InsertColumn(hwnd_vl, 1, &lvc) != -1, "ValuesListInit: ListView_InsertColumn(1) error");
	// Колонка 2 - значение параметра в кратком виде
	lvc.iSubItem = 2;
	lvc.cx = 100;
	lvc.pszText = (char*)"Значение";
	throwerr(ListView_InsertColumn(hwnd_vl, 2, &lvc) != -1, "ValuesListInit: ListView_InsertColumn(2) error");
}


// Вызывается при изменении текущей строки в TreeView, чтобы ListView перерисовал свое содержимое
void ValuesList_TreeViewSelChanged(RegistryKey key)
{
	HWND hwnd_list = GetDlgItem(hwnd_main, IDC_VALUES_LIST);
	// Удаление всех строк из ListView
	throwerr(ListView_DeleteAllItems(hwnd_list), "ValuesList_TreeViewSelChanged: ListView_DeleteAllItems error");
	
	// Получить размер самого длинного имени и размер самого большого значения, чтобы выделить буферы достаточного размера
	LSTATUS status;
	DWORD max_name_len;
	DWORD max_value_len;
	status = RegQueryInfoKey(key.get(), 0, 0, 0, 0, 0, 0, 0, &max_name_len, &max_value_len, 0, 0);
	throwerr(status == ERROR_SUCCESS, "ValuesList_TreeViewSelChanged: RegQueryInfoKey error", false, status);
	// Размер буфера для имени должен быть не меньше 20, чтобы значение по умолчанию (с пустым именем) мы могли отображать как "(По умолчанию)". На самом деле 15 хватает, но я беру с запасом.
	if (max_name_len < 20)
		max_name_len = 20;
	// Добавляем место под завершающий нулевой байт
	max_name_len++;
	max_value_len++;

	// Выделение памяти
	char *name_buf = new char[max_name_len];
	BYTE* value_buf = new BYTE[max_value_len];
	char value_repr_buf[100];	// Буфер для краткого строкового представления значения

	DWORD value_type;
	DWORD name_len;
	DWORD value_len;
	
	LVITEM item;
	item.iSubItem = 0;
	item.mask = LVIF_TEXT | LVIF_STATE;
	item.state = 0;
	item.stateMask = 0;

	//// Query default value
	//value_len = max_value_len;
	//status = RegQueryValueEx(key.get(), NULL, NULL, &value_type, value_buf, &value_len);
	//if (status != ERROR_FILE_NOT_FOUND)
	//{
	//	throwerr(status == ERROR_SUCCESS, "ValuesList_TreeViewSelChanged: RegQueryValueEx error", false, status);
	//	strcpy(name_buf, "(По умолчанию)");
	//	AddItem(item, 0, hwnd_list, value_type, name_buf, value_buf, value_len, value_repr_buf);
	//}

	// Обход параметров реестра в цикле
	DWORD index = 0;
	name_len = max_name_len;
	value_len = max_value_len;
	status = RegEnumValue(key.get(), index, name_buf, &name_len, 0, &value_type, value_buf, &value_len);	// Запрос первого параметра
	while (status != ERROR_NO_MORE_ITEMS)	// ERROR_NO_MORE_ITEMS означает, что обход завершен
	{
		throwerr(status == ERROR_SUCCESS, "ValuesList_TreeViewSelChanged: RegEnumValue error", false, status);
		if (name_len == 0)	// Default value
			strcpy(name_buf, "(По умолчанию)");
		else
			name_buf[name_len] = 0;
		// Добавление значения в ListView
		AddItem(item, index, hwnd_list, value_type, name_buf, value_buf, value_len, value_repr_buf);

		index++;
		name_len = max_name_len;
		value_len = max_value_len;
		status = RegEnumValue(key.get(), index, name_buf, &name_len, 0, &value_type, value_buf, &value_len);	// Запрос следующего параметра
	}
	// Освобождение буферов
	delete[] name_buf;
	delete[] value_buf;
}


// Добавить в ListView краткое строковое представление значения параметра реестра
void SetItemValueRepr(HWND hwnd_list, DWORD index, DWORD value_type, BYTE *value_buf, DWORD value_len, char *value_repr_buf)
{
	switch (value_type)
	{
	case REG_BINARY:
		// Берем не более 30 байт
		GetBinaryValueRepr(value_buf, value_len > 30 ? 30 : value_len, value_repr_buf);
		break;
	case REG_DWORD:
	{
		auto value = *((DWORD*)value_buf);
		sprintf(value_repr_buf, "0x%08X (%u)", value, value);
		break;
	}
	case REG_QWORD:
	{
		auto value = *((unsigned long long*)value_buf);
		sprintf(value_repr_buf, "0x%016llX (%llu)", value, value);
		break;
	}
	case REG_SZ:
		strncpy(value_repr_buf, (char*)value_buf, 99);
		break;
	case REG_MULTI_SZ:
		// Для мультистроки берем только первую строку (и не более 99 символов)
		strncpy(value_repr_buf, (char*)value_buf, 99);
		break;
	case REG_EXPAND_SZ:
		// То же самое, что REG_SZ
		strncpy(value_repr_buf, (char*)value_buf, 99);
		break;
	case REG_NONE:
		value_repr_buf[0] = 0;
		break;
	default:
		value_repr_buf[0] = 0;
	}
	// Добавить в ListView
	ListView_SetItemText(hwnd_list, index, 2, value_repr_buf);
}


// Добавить параметр реестра в ListView
void AddItem(LVITEM& item, DWORD index, HWND hwnd_list, DWORD value_type, char* name_buf, BYTE* value_buf, DWORD value_len, char* value_repr_buf)
{
	// Вставка имени параметра
	item.pszText = name_buf;
	item.iItem = index;
	throwerr(ListView_InsertItem(hwnd_list, &item) != -1, "ValuesList_TreeViewSelChanged: ListView_InsertItem error");

	// Вставка типа значения
	ListView_SetItemText(hwnd_list, index, 1, (char*)RegistryKey::GetValueTypeString(value_type));

	// Вставка краткого представления
	SetItemValueRepr(hwnd_list, index, value_type, value_buf, value_len, value_repr_buf);
}


// Получить краткое представление двоичного параметра реестра в шестнадцатеричном виде
void GetBinaryValueRepr(BYTE *value, DWORD value_length, char *repr)
{
	static const char digits[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
	DWORD repr_len = 0;
	for (DWORD i = 0; i < value_length; ++i)
	{
		BYTE b = value[i];
		repr[repr_len++] = digits[b >> 4];	// Старший полубайт получаем сдвигом на 4 бита вправо
		repr[repr_len++] = digits[b & 0xF];	// Младший полубайт получаем операцией AND с маской 00001111
	}
	repr[repr_len] = 0;	// Завершающий нулевой символ
}


// Преобразование строкового представления 16-ичной цифры в саму цифру. Вернуть false в случае неудачи.
bool GetHexFromChar(char c, BYTE *h)
{
	if (c >= '0' && c <= '9')
	{
		*h = c - '0';
		return true;
	}
	if (c >= 'A' && c <= 'F')
	{
		*h = c - 'A' + 10;
		return true;
	}
	if (c >= 'a' && c <= 'f')
	{
		*h = c - 'a' + 10;
		return true;
	}
	return false;
}


// Преобразование строкового представления двоичного значения в само двоичное значение (байтовый массив). repr_len должно быть кратно 2.
// В случае неудачи возвращается false, а индекс неверного символа возвращается через bad_char_index
bool ValueReprToBinary(char *repr, int repr_len, BYTE *result, int *bad_char_index)
{
	int bytes_len = repr_len / 2;	// Число байт = число символов / 2
	for (int pos = 0; pos < bytes_len; pos++)
	{
		// Получить старший полубайт
		BYTE b1;
		if (!GetHexFromChar(repr[pos * 2], &b1))
		{
			*bad_char_index = pos * 2;
			return false;
		}
		// Получить младший полубайт
		BYTE b2;
		if (!GetHexFromChar(repr[pos * 2 + 1], &b2))
		{
			*bad_char_index = pos * 2 + 1;
			return false;
		}
		// Объединить полубайты и положить в result
		result[pos] = (b1 << 4) | b2;
	}
	return true;
}


// Получить текст элемента ListView (т.е. имя параметра реестра)
string GetListViewItemText(HWND hwnd, int idx)
{
	// Длина имени заранее неизвестна.
	// Поэтому будем запрашивать имя в цикле до тех пор,
	// пока возвращенная длина returned_len не станет меньше размера буфера len, на каждом шагу увеличивая буфер в 1,5 раза.
	// Только так можно убедиться, что нам вернули полное имя, не обрезанное.
	size_t len = 8;
	string name;
	size_t returned_len;
	LVITEM item;
	item.iSubItem = 0;
	do
	{
		// Увеличиваем буфер в 1,5 раза
		len += len / 2;
		name.assign(len, ' ');
		// Запрашиваем имя
		item.pszText = &name[0];
		item.cchTextMax = len + 1;
		returned_len = (size_t)SendMessage(hwnd, LVM_GETITEMTEXT, idx, (LPARAM)&item);
	} while (returned_len >= len);
	// Удалим лишние, неиспользованные байты в буфере
	name.erase(returned_len, len - returned_len);
	return name;
}


// Обработка WM_COMMAND (нажатия кнопок добавления, переименования, редактирования и удаления параметра)
BOOL ValuesListProcessButton(WPARAM wp, LPARAM lp)
{
	HWND hwnd_vl = GetDlgItem(hwnd_main, IDC_VALUES_LIST);
	HWND hwnd_tv = GetDlgItem(hwnd_main, IDC_TREEVIEW);
	throwerr(hwnd_vl != NULL, "ValuesListProcessButton: GetDlgItem error");
	if (HIWORD(wp) == BN_CLICKED)	// Обрабатываем только сообщение BN_CLICKED (нажатие кнопки)
		switch (LOWORD(wp))
		{
		case IDC_ADD_PARAM_BUTTON:	// Нажата кнопка добавления параметра
		{
			// Получить текущий элемент TreeView. Если элемент не выбран, возврат
			auto selected_key = TreeView_GetSelection(hwnd_tv);
			if (selected_key == NULL)
				return TRUE;
			// Открыть диалог выбора типа параметра
			auto result = DialogBox(hinstance, "ChooseParamTypeDialog", hwnd_main, ChooseParamTypeDlgProc);
			if (result == IDOK)	// Нажато ОК
			{
				// Очистка имени параметра
				param_name_edit.clear();
				// Открыть диалог ввода имени параметра
				result = DialogBox(hinstance, "AddEditParamNameDialog", hwnd_main, AddEditParamNameDlgProc);
				if (result == IDOK)	// Нажато ОК
				{
					// Пробуем открыть раздел. В случае ошибок выдадим сообщение пользователю (MessageBox), что недостаточно прав доступа.
					LSTATUS status;
					auto key = GetRegistryKey(selected_key, hwnd_tv, KEY_READ | KEY_WRITE, false, &status);
					if (status != ERROR_SUCCESS)
					{
						MessageBox(hwnd_main, "Не удалось открыть раздел для изменения. Возможно, нет прав доступа", "Ошибка", MB_OK);
						return TRUE;
					}
					// Проверяем, что параметра с введенным именем в текущем разделе не существует
					status = RegQueryValueEx(key.get(), &param_name_edit[0], NULL, NULL, NULL, NULL);
					if (status != ERROR_FILE_NOT_FOUND)
					{
						throwerr(status == ERROR_SUCCESS, "ValuesListProcessButton, add param button: RegQueryValueEx error", false, status);
						// Если существует - выдаем MessageBox и выходим
						MessageBox(hwnd_main, "Такой параметр уже существует", "Ошибка", MB_OK);
						return TRUE;
					}
					if (param_type_edit != 4)	// Для типов 0 (REG_BINARY), 1 (REG_DWORD), 2 (REG_QWORD), 3 (REG_SZ), 5 (REG_EXPAND_SZ) используется диалог AddEditParamValueDialog,
												// а для типа 4 (REG_MULTI_SZ) другой диалог
					{
						// Заполняем "пустое" значение параметра
						switch (param_type_edit)
						{
						case 0:		// Для REG_BINARY это массив байт нулевой длины
							param_value_size_edit = 0;
							param_value_edit.clear();
							break;
						case 1:		// Для REG_DWORD это 4 байта со значением 0
							param_value_size_edit = 4;
							param_value_edit.assign(4, 0);
							break;
						case 2:		// Для REG_QWORD это 8 байт со значением 0
							param_value_size_edit = 8;
							param_value_edit.assign(8, 0);
							break;
						case 3:		// Для типов REG_SZ и REG_EXPAND_SZ это пустая строка (1 байт - это признак конца строки)
						case 5:
							param_value_size_edit = 1;
							param_value_edit.assign(1, 0);
							break;
						}
						// Вызываем диалог ввода значения параметра
						result = DialogBox(hinstance, "AddEditParamValueDialog", hwnd_main, AddEditParamValueDlgProc);
					}
					else	// Для типа 4 (REG_MULTI_SZ) будет диалог AddEditMultistringValueDialog.
					{
						// Пустая строка (1 байт - это признак конца строки)
						param_value_size_edit = 1;
						param_value_edit.assign(1, 0);
						result = DialogBox(hinstance, "AddEditMultistringValueDialog", hwnd_main, AddEditMultistringValueDlgProc);
					}

					if (result == IDOK)	// Нажато ОК
					{
						// Преобразуем из "нашей" кодировки в ту, которую Windows понимает
						DWORD param_type;
						switch (param_type_edit)
						{
						case 0:
							param_type = REG_BINARY;
							break;
						case 1:
							param_type = REG_DWORD;
							break;
						case 2:
							param_type = REG_QWORD;
							break;
						case 3:
							param_type = REG_SZ;
							break;
						case 4:
							param_type = REG_MULTI_SZ;
							break;
						case 5:
							param_type = REG_EXPAND_SZ;
							break;
						default:
							// Других значений не должно быть, это ошибка
							throwerr(true, "ValuesListProcessButton, add param button: internal error (param_type_edit)", false);
						}

						// Если параметр REG_BINARY нулевой длины, то попытка взять адрес &param_value_edit[0] вызовет исключение,
						// поэтому в этом случае просто берем нулевой указатель.
						// Функции RegSetValueEx и AddItem корректно работают с нулевым указателем.
						BYTE* data = param_value_size_edit == 0 ? NULL : &param_value_edit[0];
						// Добавляем параметр в реестр. В случае ошибки показываем MessageBox и выходим
						status = RegSetValueEx(key.get(), &param_name_edit[0], NULL, param_type, data, param_value_size_edit);
						if (status != ERROR_SUCCESS)
						{
							MessageBox(hwnd_main, "Не удалось добавить параметр. Возможно, нет прав доступа", "Ошибка", MB_OK);
							return TRUE;
						}
						
						// Добавляем в ListView (в конец)
						char value_repr_buf[100];
						LVITEM item;
						item.iSubItem = 0;
						item.mask = LVIF_TEXT | LVIF_STATE;
						item.state = 0;
						item.stateMask = 0;
						auto index = ListView_GetItemCount(hwnd_vl);
						AddItem(item, index, hwnd_vl, param_type, &param_name_edit[0], data, param_value_size_edit, value_repr_buf);
					}
				}
			}
			return TRUE;
		}
		case IDC_RENAME_PARAM_BUTTON:	// Нажата кнопка переименования параметра
		{
			// ПРИНЦИП РАБОТЫ: в WinAPI нет функции переименования параметра. Поэтому действуем так: добавляем параметр с новым именем и с тем же значением, а старый параметр удаляем.

			// Получить текущий элемент TreeView. Если элемент не выбран, возврат
			auto selected_key = TreeView_GetSelection(hwnd_tv);
			if (selected_key == NULL)
				return TRUE;
			// Номер выбранного элемента в ListView. Если не выбран, возврат
			auto selected_param_idx = ListView_GetNextItem(hwnd_vl, -1, LVNI_SELECTED);
			if (selected_param_idx == -1)
				return TRUE;
			// Получить имя параметра из ListView
			string old_param_name = GetListViewItemText(hwnd_vl, selected_param_idx);
			param_name_edit = old_param_name;
			// Вызываем диалог ввода имени параметра
			auto result = DialogBox(hinstance, "AddEditParamNameDialog", hwnd_main, AddEditParamNameDlgProc);
			if (result == IDOK)	// Нажато ОК
			{
				// Если введенное имя совпадает со старым, ничего делать не нужно
				if (old_param_name == param_name_edit)
					return TRUE;
				// Пробуем открыть раздел. В случае ошибок выдадим сообщение пользователю (MessageBox), что недостаточно прав доступа.
				LSTATUS status;
				auto key = GetRegistryKey(selected_key, hwnd_tv, KEY_READ | KEY_WRITE, false, &status);
				if (status != ERROR_SUCCESS)
				{
					MessageBox(hwnd_main, "Не удалось открыть раздел для изменения. Возможно, нет прав доступа", "Ошибка", MB_OK);
					return TRUE;
				}
				// Проверим, что параметра с таким именем не существует
				status = RegQueryValueEx(key.get(), &param_name_edit[0], NULL, NULL, NULL, NULL);
				if (status != ERROR_FILE_NOT_FOUND)
				{
					throwerr(status == ERROR_SUCCESS, "ValuesListProcessButton, rename param button: RegQueryValueEx error", false, status);
					MessageBox(hwnd_main, "Такой параметр уже существует", "Ошибка", MB_OK);
					return TRUE;
				}
				// Запросим размер значения параметра. При ошибке выдадим MessageBox и выйдем
				DWORD value_size;
				status = RegQueryValueEx(key.get(), &old_param_name[0], NULL, NULL, NULL, &value_size);
				if (status != ERROR_SUCCESS)
				{
					MessageBox(hwnd_main, "Ошибка переименования параметра. Возможно, нет прав доступа", "Ошибка", MB_OK);
					return TRUE;
				}
				param_value_edit.assign(value_size, 0);
				DWORD value_type;
				BYTE* data_ptr = value_size == 0 ? NULL : &param_value_edit[0];	// Если параметр REG_BINARY нулевой длины, то попытка взять адрес &param_value_edit[0] вызовет исключение, поэтому берем нулевой указатель.
				// Запросим само значение параметра. При ошибке выдадим MessageBox и выйдем
				status = RegQueryValueEx(key.get(), &old_param_name[0], NULL, &value_type, data_ptr, &value_size);
				if (status != ERROR_SUCCESS)
				{
					MessageBox(hwnd_main, "Ошибка переименования параметра. Возможно, нет прав доступа", "Ошибка", MB_OK);
					return TRUE;
				}
				// Добавим новый параметр. При ошибке выдадим MessageBox и выйдем
				status = RegSetValueEx(key.get(), &param_name_edit[0], NULL, value_type, data_ptr, value_size);
				if (status != ERROR_SUCCESS)
				{
					MessageBox(hwnd_main, "Ошибка переименования параметра. Возможно, нет прав доступа", "Ошибка", MB_OK);
					return TRUE;
				}
				// Удалим старый параметр. При ошибке выдадим MessageBox и выйдем
				status = RegDeleteValue(key.get(), &old_param_name[0]);
				if (status != ERROR_SUCCESS)
				{
					MessageBox(hwnd_main, "Ошибка переименования параметра. Возможно, нет прав доступа", "Ошибка", MB_OK);
					return TRUE;
				}
				// Переименуем в ListView
				LVITEM item;
				item.iItem = selected_param_idx;
				item.iSubItem = 0;
				item.mask = LVIF_TEXT;
				item.stateMask = 0;
				item.pszText = &param_name_edit[0];
				throwerr(ListView_SetItem(hwnd_vl, &item), "ValuesListProcessButton, rename param button: ListView_SetItem error");
			}
			return TRUE;
		}
		case IDC_EDIT_PARAM_BUTTON:	// Нажата кнопка редактирования параметра
		{
			// Получить текущий элемент TreeView. Если элемент не выбран, возврат
			auto selected_key = TreeView_GetSelection(hwnd_tv);
			if (selected_key == NULL)
				return TRUE;
			// Номер выбранного элемента в ListView. Если не выбран, возврат
			auto selected_param_idx = ListView_GetNextItem(hwnd_vl, -1, LVNI_SELECTED);
			if (selected_param_idx == -1)
				return TRUE;
			// Получить имя параметра из ListView
			param_name_edit = GetListViewItemText(hwnd_vl, selected_param_idx);
			// Пробуем открыть раздел. В случае ошибок выдадим сообщение пользователю (MessageBox), что недостаточно прав доступа.
			LSTATUS status;
			auto key = GetRegistryKey(selected_key, hwnd_tv, KEY_READ | KEY_WRITE, false, &status);
			if (status != ERROR_SUCCESS)
			{
				MessageBox(hwnd_main, "Не удалось открыть раздел для изменения. Возможно, нет прав доступа", "Ошибка", MB_OK);
				return TRUE;
			}
			// Запросим размер значения параметра. В случае ошибок выдадим MessageBox и выйдем.
			status = RegQueryValueEx(key.get(), &param_name_edit[0], NULL, NULL, NULL, (DWORD*)&param_value_size_edit);
			if (status != ERROR_SUCCESS)
			{
				MessageBox(hwnd_main, "Ошибка изменения параметра. Возможно, нет прав доступа", "Ошибка", MB_OK);
				return TRUE;
			}
			param_value_edit.assign(param_value_size_edit, 0);
			DWORD value_type;
			BYTE* data_ptr = param_value_size_edit == 0 ? NULL : &param_value_edit[0];	// Если параметр REG_BINARY нулевой длины, то попытка взять адрес &param_value_edit[0] вызовет исключение, поэтому берем нулевой указатель.
			// Запросим само значение параметра. В случае ошибок выдадим MessageBox и выйдем.
			status = RegQueryValueEx(key.get(), &param_name_edit[0], NULL, &value_type, data_ptr, (DWORD*)&param_value_size_edit);
			if (status != ERROR_SUCCESS)
			{
				MessageBox(hwnd_main, "Ошибка изменения параметра. Возможно, нет прав доступа", "Ошибка", MB_OK);
				return TRUE;
			}
			// Преобразуем тип параметра из "кодировки" Windows в нашу "кодировку".
			switch (value_type)
			{
			case REG_NONE:	// Значение REG_NONE не редактируется, выходим
				return TRUE;
			case REG_BINARY:
				param_type_edit = 0;
				break;
			case REG_DWORD:
				param_type_edit = 1;
				break;
			case REG_QWORD:
				param_type_edit = 2;
				break;
			case REG_SZ:
				param_type_edit = 3;
				break;
			case REG_MULTI_SZ:
				param_type_edit = 4;
				break;
			case REG_EXPAND_SZ:
				param_type_edit = 5;
				break;
			default:
				// Такого не должно быть
				throwerr(false, "ValuesListProcessButton, edit param button: internal error (value_type)", false);
			}
			INT_PTR result;
			// Вызываем диалог редактирования. Для типа 4 (REG_MULTI_SZ) это будет AddEditMultistringValueDialog, для остальных типов AddEditParamValueDialog
			if (param_type_edit != 4)
				result = DialogBox(hinstance, "AddEditParamValueDialog", hwnd_main, AddEditParamValueDlgProc);
			else
				result = DialogBox(hinstance, "AddEditMultistringValueDialog", hwnd_main, AddEditMultistringValueDlgProc);
			if (result == IDOK)	// Диалог завершен нажатием ОК
			{
				BYTE* data_ptr = param_value_size_edit == 0 ? NULL : &param_value_edit[0];	// Если параметр REG_BINARY нулевой длины, то попытка взять адрес &param_value_edit[0] вызовет исключение, поэтому берем нулевой указатель.
				// Изменяем значение параметра. В случае ошибок выдаем MessageBox и выходим.
				status = RegSetValueEx(key.get(), &param_name_edit[0], NULL, value_type, data_ptr, param_value_size_edit);
				if (status != ERROR_SUCCESS)
				{
					MessageBox(hwnd_main, "Ошибка переименования параметра. Возможно, нет прав доступа", "Ошибка", MB_OK);
					return TRUE;
				}
				// Изменяем краткое представление значения в ListView
				char repr_buf[100];
				SetItemValueRepr(hwnd_vl, selected_param_idx, value_type, data_ptr, param_value_size_edit, repr_buf);
			}
			return TRUE;
		}
		case IDC_DELETE_PARAM_BUTTON:	// Нажата кнопка удаления параметра
		{
			// Получить текущий элемент TreeView. Если элемент не выбран, возврат
			auto selected_key = TreeView_GetSelection(hwnd_tv);
			if (selected_key == NULL)
				return TRUE;
			// Номер выбранного элемента в ListView. Если не выбран, возврат
			auto selected_param_idx = ListView_GetNextItem(hwnd_vl, -1, LVNI_SELECTED);
			if (selected_param_idx == -1)
				return TRUE;
			// Получить имя параметра из ListView
			param_name_edit = GetListViewItemText(hwnd_vl, selected_param_idx);
			// Пробуем открыть раздел. В случае ошибок выдадим сообщение пользователю (MessageBox), что недостаточно прав доступа.
			LSTATUS status;
			auto key = GetRegistryKey(selected_key, hwnd_tv, KEY_READ | KEY_WRITE, false, &status);
			if (status != ERROR_SUCCESS)
			{
				MessageBox(hwnd_main, "Не удалось открыть раздел для изменения. Возможно, нет прав доступа", "Ошибка", MB_OK);
				return TRUE;
			}
			// Удаляем параметр. В случае ошибок выдаем MessageBox и выходим.
			status = RegDeleteValue(key.get(), &param_name_edit[0]);
			if (status != ERROR_SUCCESS)
			{
				MessageBox(hwnd_main, "Ошибка удаления параметра. Возможно, нет прав доступа", "Ошибка", MB_OK);
				return TRUE;
			}
			// Удаляем из ListView
			throwerr(ListView_DeleteItem(hwnd_vl, selected_param_idx), "ValuesListProcessButton, delete param button: ListView_DeleteItem error");
			return TRUE;
		}
		}
	return FALSE;
}
