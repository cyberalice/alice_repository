#pragma once

// Основной header, который подключается во все cpp-файлы.

#pragma warning (disable: 4996)	// Отключаем предупреждение 4996 (ругается на sprintf и другие небезопасные функции)

#include <Windows.h>
#include "resource.h"
#include <CommCtrl.h>
#include <exception>

using namespace std;


// Объявление общедоступных переменных, классов и функций модуля main.cpp

extern HINSTANCE hinstance;
extern HWND hwnd_main;
void throwerr(bool condition, const char* message, bool get_last_error = true, DWORD error_code = ERROR_SUCCESS);


// Объявление общедоступных переменных, классов и функций модуля exception.cpp

// Класс-исключение с возможностью логирования
class MyException : public std::exception
{
public:
	MyException(const char* s);
	void log();
};

void log(const char* s);	// Логирование ошибок


// Объявление общедоступных переменных, классов и функций модуля registry.cpp

// Класс-обертка над HKEY (раздел реестра) для безопасного перемещения (MoveConstructible) и освобождения ресурсов
class RegistryKey
{
public:
	RegistryKey(HKEY hk);
	HKEY get();
	~RegistryKey();
	RegistryKey(RegistryKey&&);	// Move constructor
	static const char *GetValueTypeString(DWORD t);	// Возвращает строковое представление для типа параметра (REG_SZ и т.п.)
private:
	HKEY _hkey;
};


// Объявление общедоступных переменных, классов и функций модуля tree_view.cpp

void TreeViewInit();	// Инициализация TreeView
HTREEITEM TreeViewAddItem(HTREEITEM parent, const char* text, HWND treeview);	// Добавление элемента в TreeView
INT_PTR TreeViewProcessNotification(NMHDR* nm);		// Обработка WM_NOTIFY
BOOL TreeViewProcessButton(WPARAM wp, LPARAM lp);	// Обработка WM_COMMAND (нажатия кнопок)
// Возвращает RegistryKey, соответствующий данному элементу TreeView
RegistryKey GetRegistryKey(HTREEITEM item, HWND treeview, REGSAM access = KEY_READ, bool throwerror = true, LSTATUS* pstatus = NULL);


// Объявление общедоступных переменных, классов и функций модуля values_list.cpp

void ValuesListInit();	// Инициализация ListView
void ValuesList_TreeViewSelChanged(RegistryKey key);	// Вызывается при изменении текущей строки в TreeView, чтобы ListView перерисовал свое содержимое
BOOL ValuesListProcessButton(WPARAM wp, LPARAM lp);		// Обработка WM_COMMAND (нажатия кнопок)
