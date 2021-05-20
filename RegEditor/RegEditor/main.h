#pragma once

// �������� header, ������� ������������ �� ��� cpp-�����.

#pragma warning (disable: 4996)	// ��������� �������������� 4996 (�������� �� sprintf � ������ ������������ �������)

#include <Windows.h>
#include "resource.h"
#include <CommCtrl.h>
#include <exception>

using namespace std;


// ���������� ������������� ����������, ������� � ������� ������ main.cpp

extern HINSTANCE hinstance;
extern HWND hwnd_main;
void throwerr(bool condition, const char* message, bool get_last_error = true, DWORD error_code = ERROR_SUCCESS);


// ���������� ������������� ����������, ������� � ������� ������ exception.cpp

// �����-���������� � ������������ �����������
class MyException : public std::exception
{
public:
	MyException(const char* s);
	void log();
};

void log(const char* s);	// ����������� ������


// ���������� ������������� ����������, ������� � ������� ������ registry.cpp

// �����-������� ��� HKEY (������ �������) ��� ����������� ����������� (MoveConstructible) � ������������ ��������
class RegistryKey
{
public:
	RegistryKey(HKEY hk);
	HKEY get();
	~RegistryKey();
	RegistryKey(RegistryKey&&);	// Move constructor
	static const char *GetValueTypeString(DWORD t);	// ���������� ��������� ������������� ��� ���� ��������� (REG_SZ � �.�.)
private:
	HKEY _hkey;
};


// ���������� ������������� ����������, ������� � ������� ������ tree_view.cpp

void TreeViewInit();	// ������������� TreeView
HTREEITEM TreeViewAddItem(HTREEITEM parent, const char* text, HWND treeview);	// ���������� �������� � TreeView
INT_PTR TreeViewProcessNotification(NMHDR* nm);		// ��������� WM_NOTIFY
BOOL TreeViewProcessButton(WPARAM wp, LPARAM lp);	// ��������� WM_COMMAND (������� ������)
// ���������� RegistryKey, ��������������� ������� �������� TreeView
RegistryKey GetRegistryKey(HTREEITEM item, HWND treeview, REGSAM access = KEY_READ, bool throwerror = true, LSTATUS* pstatus = NULL);


// ���������� ������������� ����������, ������� � ������� ������ values_list.cpp

void ValuesListInit();	// ������������� ListView
void ValuesList_TreeViewSelChanged(RegistryKey key);	// ���������� ��� ��������� ������� ������ � TreeView, ����� ListView ����������� ���� ����������
BOOL ValuesListProcessButton(WPARAM wp, LPARAM lp);		// ��������� WM_COMMAND (������� ������)
