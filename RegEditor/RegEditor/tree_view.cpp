
#include "main.h"
#include <string>

// Чтобы для элемента TreeView не заполнять суб-элементы (иначе при открытии программы нам пришлось бы сканировать весь реестр, что очень долго),
// мы при создании элемента устанавливаем статус ITEM_NEVER_EXPANDED (элемент никогда не раскрывался "плюсиком") (см. TreeViewAddItem),
// а заполнять суб-элементы будем только когда пользователь впервые раскроет элемент "плюсиком" (см. ProcessItemExpanding) и установим статус ITEM_HAS_BEEN_EXPANDED.
#define ITEM_NEVER_EXPANDED (0)
#define ITEM_HAS_BEEN_EXPANDED (1)

INT_PTR ProcessItemExpanding(NMTREEVIEW *data);	// Обработка TVN_ITEMEXPANDING (разворачивание элемента "плюсиком")
INT_PTR ProcessSelChanged(NMTREEVIEW* data);	// Обработка TVN_SELCHANGED (изменение текущей строки)
string GetItemName(HTREEITEM hitem, HWND treeview);	// Возвращает имя раздела реестра для указанного элемента TreeView
RegistryKey GetRootRegistryKey(const char* name);	// Возвращает имя корневого раздела реестра по указанному имени
INT_PTR CALLBACK AddEditKeyDlgProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);	// Функция - обработчик сообщений диалога "Добавление/переименование раздела"

// Глобальная переменная, в ней сохраняется имя редактируемого/добавляемого раздела реестра. Используется в диалоге "Добавление/переименование раздела"
char* keyname_edit = new char[256];


void TreeViewInit()
{
	// Создать ImageList с картинкой для TreeView
	auto hlist = ImageList_LoadBitmap(hinstance, "RegKeyBitmap", 16, 0, RGB(255, 255, 255));
	throwerr(hlist != NULL, "ImageList_LoadBitmap error");

	// Установить ImageList для TreeView
	auto hwnd_tv = GetDlgItem(hwnd_main, IDC_TREEVIEW);
	throwerr(hwnd_tv != NULL, "GetDlgItem (IDC_TREEVIEW) error");
	TreeView_SetImageList(hwnd_tv, hlist, TVSIL_NORMAL);
	
	// Добавить предопределенные (корневые) разделы реестра
	TreeViewAddItem(TVI_ROOT, "HKEY_CLASSES_ROOT", hwnd_tv);
	TreeViewAddItem(TVI_ROOT, "HKEY_CURRENT_CONFIG", hwnd_tv);
	TreeViewAddItem(TVI_ROOT, "HKEY_CURRENT_USER", hwnd_tv);
	TreeViewAddItem(TVI_ROOT, "HKEY_LOCAL_MACHINE", hwnd_tv);
	//TreeViewAddItem(TVI_ROOT, "HKEY_PERFORMANCE_DATA", hwnd_tv);
	TreeViewAddItem(TVI_ROOT, "HKEY_USERS", hwnd_tv);
}


HTREEITEM TreeViewAddItem(HTREEITEM parent, const char *text, HWND treeview = NULL)
{
	TVINSERTSTRUCT ins;
	// Заполнение нужных полей структуры ins
	ins.hParent = parent;			// Родительский элемент
	ins.hInsertAfter = TVI_LAST;	// Вставить элемент последним
	ins.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;	// Использовать только поля pszText, iImage, iSelectedImage, lParam
	ins.item.pszText = (char*)text;
	ins.item.iImage = 0;	// Индекс картинки в ImageList
	ins.item.iSelectedImage = 0;	// Индекс картинки в ImageList, когда строка является текущей
	ins.item.lParam = ITEM_NEVER_EXPANDED;
	// Если treeview не передан, получим его сами
	if (treeview == NULL)
		treeview = GetDlgItem(hwnd_main, IDC_TREEVIEW);
	// Вставка элемента
	auto hitem = TreeView_InsertItem(treeview, &ins);
	throwerr(hitem != NULL, "TreeView_InsertItem error");
	// Вставим "фейковый" суб-элемент с именем "Expanding...", просто для того, чтобы у текущего элемента отображался "плюсик"
	ins.hParent = hitem;
	ins.hInsertAfter = TVI_LAST;
	ins.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
	ins.item.pszText = (char*)"Expanding...";
	ins.item.iImage = 0;
	ins.item.iSelectedImage = 0;
	auto hsubitem = TreeView_InsertItem(treeview, &ins);
	throwerr(hsubitem != NULL, "TreeView_InsertItem (subitem) error");
	return hitem;
}


// Обработчик WM_NOTIFY
INT_PTR TreeViewProcessNotification(NMHDR *nm)
{
	switch (nm->code)
	{
	case TVN_ITEMEXPANDING:
		return ProcessItemExpanding((NMTREEVIEW*)nm);
	case TVN_SELCHANGED:
		return ProcessSelChanged((NMTREEVIEW*)nm);
	}
	return FALSE;
}


// Обработчик TVN_ITEMEXPANDING
INT_PTR ProcessItemExpanding(NMTREEVIEW *data)
{
	HWND treeview = data->hdr.hwndFrom;
	HTREEITEM item = data->itemNew.hItem;
	if (data->itemNew.lParam == ITEM_NEVER_EXPANDED)	// Если элемент никогда ранее не разворачивался "плюсиком"
	{
		// Удалим подчиненный "фейковый" элемент "Expanding..."
		auto child_item = TreeView_GetChild(treeview, item);
		throwerr(TreeView_DeleteItem(treeview, child_item), "ProcessItemExpanding: TreeView_DeleteItem error");
		
		// Получим раздел реестра
		auto key = GetRegistryKey(item, treeview);
		
		bool has_children = false;	// Будем выяснять, есть ли на самом деле подразделы у текущего раздела
		DWORD name_length;
		static char* key_buffer = new char[256];	// Буфер для получения имен подразделов
		LSTATUS status;
		DWORD index = 0;

		// Обход подразделов в цикле
		name_length = 256;
		status = RegEnumKeyEx(key.get(), index, key_buffer, &name_length, NULL, NULL, NULL, NULL);
		while (status != ERROR_NO_MORE_ITEMS)	// ERROR_NO_MORE_ITEMS означает, что обход подразделов завершен
		{
			throwerr(status == ERROR_SUCCESS, "ProcessItemExpanding: RegEnumKeyEx error", false, status);
			index++;
			key_buffer[name_length] = 0;
			has_children = true;
			// Добавление подраздела
			TreeViewAddItem(item, key_buffer, treeview);
			name_length = 256;
			status = RegEnumKeyEx(key.get(), index, key_buffer, &name_length, NULL, NULL, NULL, NULL);
		}
		// Установим статус ITEM_HAS_BEEN_EXPANDED, чтобы больше эту процедуру раскрытия не выполнять
		TVITEM set_item;
		set_item.hItem = item;
		set_item.mask = TVIF_PARAM;
		set_item.lParam = ITEM_HAS_BEEN_EXPANDED;
		throwerr(TreeView_SetItem(treeview, &set_item), "ProcessItemExpanding: TreeView_SetItem error");
		// Если нет дочерних элементов, вернем TRUE (тогда раскрытие будет отменено); иначе вернем FALSE
		return !has_children;
	}
	return FALSE;
}


// Обработчик TVN_SELCHANGED
INT_PTR ProcessSelChanged(NMTREEVIEW *data)
{
	HWND treeview = data->hdr.hwndFrom;
	auto item = data->itemNew.hItem;
	// Просто делегируем в модуль values_list.cpp
	ValuesList_TreeViewSelChanged(GetRegistryKey(item, treeview));
	return FALSE;
}


// Возвращает RegistryKey (объект-обертку над HKEY) для раздела реестра, соответствующего данному элементу TreeView.
// Раздел открывается с указанными правами (access).
// При возникновении ошибок: если установлен флаг throwerror, бросает исключение; иначе помещает код ошибки в pstatus
RegistryKey GetRegistryKey(HTREEITEM item, HWND treeview, REGSAM access, bool throwerror, LSTATUS* pstatus)
{
	auto item_name = GetItemName(item, treeview);	// Получить текст в текущей строке
	auto parent = TreeView_GetParent(treeview, item);	// Получить родительский элемент
	if (parent == NULL)
		return GetRootRegistryKey(&item_name[0]);	// Если это корневой элемент (т.е. родителя нет), вернем соответствующий корневой раздел
	else
	{
		auto parent_hkey = GetRegistryKey(parent, treeview);	// Рекурсивно получим RegistryKey родительского раздела
		HKEY result;
		LSTATUS status = RegOpenKeyEx(parent_hkey.get(), &item_name[0], 0, access, &result);
		if (throwerror)
		{
			throwerr(status == ERROR_SUCCESS, "GetRegistryKey: RegOpenKeyEx error", false, status);
			return RegistryKey(result);
		}
		else
		{
			if (pstatus == NULL)
				throwerr(false, "GetRegistryKey: if throwerror flag is not set then pstatus must be not NULL", false);
			*pstatus = status;
			if (status == ERROR_SUCCESS)
				return RegistryKey(result);
			return RegistryKey(NULL);
		}
	}
}


// Возвращает текст в указанном элементе TreeView
string GetItemName(HTREEITEM hitem, HWND treeview)
{
	string result(256, ' ');	// Выделяем пустую строку 256 байт
	TVITEM item;
	ZeroMemory(&item, sizeof(TVITEM));
	item.hItem = hitem;
	item.mask = TVIF_TEXT;
	item.cchTextMax = 256;
	item.pszText = &result[0];
	// Запрашиваем текст; если ошибка, бросаем исключение
	throwerr(TreeView_GetItem(treeview, &item), "GetItemName: TreeView_GetItem error");
	return result;
}


// Возвращает RegistryKey предопределенного корневого раздела
RegistryKey GetRootRegistryKey(const char *name)
{
	if (strcmp(name, "HKEY_CLASSES_ROOT") == 0)
		return HKEY_CLASSES_ROOT;
	if (strcmp(name, "HKEY_CURRENT_CONFIG") == 0)
		return HKEY_CURRENT_CONFIG;
	if (strcmp(name, "HKEY_CURRENT_USER") == 0)
		return HKEY_CURRENT_USER;
	if (strcmp(name, "HKEY_LOCAL_MACHINE") == 0)
		return HKEY_LOCAL_MACHINE;
	//if (strcmp(name, "HKEY_PERFORMANCE_DATA") == 0)
	//	return HKEY_PERFORMANCE_DATA;
	if (strcmp(name, "HKEY_USERS") == 0)
		return HKEY_USERS;
	throwerr(false, "GetRootRegistryKey: wrong root registry key name", false);
	return NULL;
}


// Обработчик WM_COMMAND (нажатия кнопок: добавление раздела, переименование раздела, удаление раздела)
BOOL TreeViewProcessButton(WPARAM wp, LPARAM lp)
{
	HWND hwnd_tv = GetDlgItem(hwnd_main, IDC_TREEVIEW);
	throwerr(hwnd_tv != NULL, "TreeViewProcessButton: GetDlgItem error");
	if (HIWORD(wp) == BN_CLICKED)	// Обрабатываем только BN_CLICKED (нажатия кнопок)
		switch (LOWORD(wp))
		{
		case IDC_ADD_KEY_BUTTON:	// Нажата кнопка добавления раздела
		{
			// Получить текущий элемент TreeView. Если элемент не выбран, возврат
			auto selected_item = TreeView_GetSelection(hwnd_tv);
			if (selected_item == NULL)
				return TRUE;
			// Глобальную переменную keyname_edit сделаем пустой строкой, для этого в 0-й байт запишем 0 (признак конца строки)
			keyname_edit[0] = 0;
			// Откроем диалог добавления/редактирования раздела
			auto result = DialogBox(hinstance, "AddEditKeyDialog", hwnd_main, AddEditKeyDlgProc);
			if (result == IDOK)	// Диалог был завершен нажатием OK
			{
				// Пробуем открыть раздел. В случае ошибок выдадим сообщение пользователю (MessageBox), что недостаточно прав доступа.
				LSTATUS open_key_status;
				auto parent_key = GetRegistryKey(selected_item, hwnd_tv, KEY_READ | KEY_WRITE, false, &open_key_status);
				if (open_key_status != ERROR_SUCCESS)
				{
					MessageBox(hwnd_main, "Не удалось открыть раздел для изменения. Возможно, нет прав доступа", "Ошибка", MB_OK);
					return TRUE;
				}
				// После отработки диалога в глобальной переменной keyname_edit лежит имя, введенное пользователем. Используем его для создания подраздела.
				// Создадим подраздел. В случае ошибок выдадим сообщение пользователю (MessageBox).
				HKEY created_key;
				auto result = RegCreateKey(parent_key.get(), keyname_edit, &created_key);
				if (result != ERROR_SUCCESS)
				{
					MessageBox(hwnd_main, "Не удалось создать раздел. Возможно, нет прав доступа", "Ошибка", MB_OK);
					return TRUE;
				}
				// Закроем созданный раздел
				RegCloseKey(created_key);
				// Добавим элемент в TreeView
				TreeViewAddItem(selected_item, keyname_edit, hwnd_tv);
			}
			return TRUE;
		}
		case IDC_EDIT_KEY_BUTTON:	// Нажата кнопка редактирования (переименования) раздела
		{
			// Получить текущий элемент TreeView. Если элемент не выбран, возврат
			auto selected_item = TreeView_GetSelection(hwnd_tv);
			if (selected_item == NULL)
				return TRUE;
			// Получим родительский элемент
			auto parent_item = TreeView_GetParent(hwnd_tv, selected_item);
			// Если родитель пустой, то пользователь пытается редактировать предопределенный корневой раздел. Это невозможно, возвращаемся.
			if (parent_item == NULL)
				return TRUE;
			// Получить текст в текущем элементе
			string item_name = GetItemName(selected_item, hwnd_tv);
			// Скопировать его в глобальную переменную keyname_edit, чтобы диалог редактирования взял оттуда
			strcpy(keyname_edit, &item_name[0]);
			// Открыть диалог добавления/редактирования раздела
			auto result = DialogBox(hinstance, "AddEditKeyDialog", hwnd_main, AddEditKeyDlgProc);
			if (result == IDOK)	// Диалог был завершен нажатием OK
			{
				// Открываем родительский раздел. При ошибке выдаем сообщение пользователю (MessageBox)
				LSTATUS open_key_status;
				auto parent_key = GetRegistryKey(parent_item, hwnd_tv, KEY_READ | KEY_WRITE, false, &open_key_status);
				if (open_key_status != ERROR_SUCCESS)
				{
					MessageBox(hwnd_main, "Не удалось открыть раздел для изменения. Возможно, нет прав доступа", "Ошибка", MB_OK);
					return TRUE;
				}
				// Поскольку у RegRenameKey нет A-версии, приходится конвертировать всё в W-строки (UTF-16) - старое имя раздела (item_name) и новое имя раздела (keyname_edit)
				// Конвертируем старое имя раздела
				auto old_len = strlen(&item_name[0]) + 1;	// Определяем длину строки
				wchar_t* old_name_wc_buffer = new wchar_t[old_len * 2];	// Выделяем буфер для W-версии строки
				throwerr(MultiByteToWideChar(CP_ACP, 0, &item_name[0], -1, old_name_wc_buffer, old_len) != 0, "TreeViewProcessButton, edit key button: MultiByteToWideChar (old name) error");	// Конвертируем
				// Конвертируем новое имя раздела
				auto new_len = strlen(keyname_edit) + 1;
				wchar_t* new_name_wc_buffer = new wchar_t[new_len * 2];
				throwerr(MultiByteToWideChar(CP_ACP, 0, keyname_edit, -1, new_name_wc_buffer, new_len) != 0, "TreeViewProcessButton, edit key button: MultiByteToWideChar (new name) error");
				// Переименование раздела. В случае ошибок выдаем сообщение пользователю (MessageBox)
				auto result = RegRenameKey(parent_key.get(), old_name_wc_buffer, new_name_wc_buffer);
				if (result != ERROR_SUCCESS)
				{
					MessageBox(hwnd_main, "Не удалось переименовать раздел. Возможно, нет прав доступа", "Ошибка", MB_OK);
					return TRUE;
				}
				// Переименуем в TreeView
				TVITEM set_item;
				set_item.hItem = selected_item;
				set_item.mask = TVIF_TEXT;
				set_item.pszText = keyname_edit;
				throwerr(TreeView_SetItem(hwnd_tv, &set_item), "TreeViewProcessButton, edit key button: TreeView_SetItem error");
			}
			return TRUE;
		}
		case IDC_DELETE_KEY_BUTTON:	//	Нажата кнопка удаления раздела
		{
			// Получить текущий элемент TreeView. Если элемент не выбран, возврат
			auto selected_item = TreeView_GetSelection(hwnd_tv);
			if (selected_item == NULL)
				return TRUE;
			// Получим родительский элемент
			auto parent_item = TreeView_GetParent(hwnd_tv, selected_item);
			if (parent_item == NULL)	// Попытка удалить корневой раздел реестра. Это невозможно, возвращаемся.
				return TRUE;
			// Предупреждение
			auto ok_cancel = MessageBox(hwnd_main, "Удалить раздел реестра?\r\nБудут удалены ВСЕ ПОДРАЗДЕЛЫ И ПАРАМЕТРЫ.\r\nВОССТАНОВЛЕНИЕ НЕВОЗМОЖНО.", "Предупреждение", MB_OKCANCEL | MB_ICONWARNING);
			if (ok_cancel != IDOK)	// Пользователь передумал - возвращаемся
				return TRUE;
			// Получаем родительский раздел, в случае ошибок сообщение пользователю (MessageBox)
			LSTATUS open_key_status;
			auto parent_key = GetRegistryKey(parent_item, hwnd_tv, KEY_READ | KEY_WRITE, false, &open_key_status);
			if (open_key_status != ERROR_SUCCESS)
			{
				MessageBox(hwnd_main, "Не удалось открыть раздел для изменения. Возможно, нет прав доступа", "Ошибка", MB_OK);
				return TRUE;
			}
			// Получаем имя текущего раздела
			string item_name = GetItemName(selected_item, hwnd_tv);
			// Удаляем. В случае ошибок - сообщение пользователю (MessageBox)
			auto result = RegDeleteTree(parent_key.get(), &item_name[0]);
			if (result != ERROR_SUCCESS)
			{
				MessageBox(hwnd_main, "Не удалось удалить раздел. Возможно, нет прав доступа", "Ошибка", MB_OK);
				return TRUE;
			}
			// Удаляем из TreeView
			throwerr(TreeView_DeleteItem(hwnd_tv, selected_item), "TreeViewProcessButton, delete key button: TreeView_DeleteItem error");
			return TRUE;
		}
		}
	return FALSE;
}


// Функция - обработчик сообщений диалога "Добавление/переименование раздела"
INT_PTR CALLBACK AddEditKeyDlgProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	try
	{
		switch (msg)
		{
		case WM_INITDIALOG:
			// Помещаем keyname_edit в поле редактирования (т.е. предыдущее наименование раздела)
			throwerr(SetDlgItemText(hwnd, IDC_KEY_NAME_EDIT, keyname_edit) != 0, "AddEditKeyDlgProc: SetDlgItemText error");
			return TRUE;
		case WM_CLOSE:	// При нажатии на "крестик" закрываем окно с кодом "отмена"
			EndDialog(hwnd, IDCANCEL);
			return TRUE;
		case WM_COMMAND:
			if (HIWORD(wp) == BN_CLICKED)	// Нажата какая-то кнопка
			{
				if (LOWORD(wp) == IDOK)	// Нажато OK
				{
					// Получаем текст из поля редактирования в переменную keyname_edit
					GetDlgItemText(hwnd, IDC_KEY_NAME_EDIT, keyname_edit, 256);
					if (strlen(keyname_edit) == 0)
					{
						MessageBox(hwnd, "Следует ввести имя раздела", "Ошибка", MB_OK);
						return TRUE;
					}
					// Закрываем диалог
					EndDialog(hwnd, IDOK);
					return TRUE;
				}
				if (LOWORD(wp) == IDCANCEL)
				{
					// Закрываем диалог с кодом "отмена"
					EndDialog(hwnd, IDCANCEL);
					return TRUE;
				}
			}
			return FALSE;
		}
		return FALSE;
	}
	// Логирование ошибок
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
