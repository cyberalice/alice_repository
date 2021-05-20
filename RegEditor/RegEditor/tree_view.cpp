
#include "main.h"
#include <string>

// ����� ��� �������� TreeView �� ��������� ���-�������� (����� ��� �������� ��������� ��� �������� �� ����������� ���� ������, ��� ����� �����),
// �� ��� �������� �������� ������������� ������ ITEM_NEVER_EXPANDED (������� ������� �� ����������� "��������") (��. TreeViewAddItem),
// � ��������� ���-�������� ����� ������ ����� ������������ ������� �������� ������� "��������" (��. ProcessItemExpanding) � ��������� ������ ITEM_HAS_BEEN_EXPANDED.
#define ITEM_NEVER_EXPANDED (0)
#define ITEM_HAS_BEEN_EXPANDED (1)

INT_PTR ProcessItemExpanding(NMTREEVIEW *data);	// ��������� TVN_ITEMEXPANDING (�������������� �������� "��������")
INT_PTR ProcessSelChanged(NMTREEVIEW* data);	// ��������� TVN_SELCHANGED (��������� ������� ������)
string GetItemName(HTREEITEM hitem, HWND treeview);	// ���������� ��� ������� ������� ��� ���������� �������� TreeView
RegistryKey GetRootRegistryKey(const char* name);	// ���������� ��� ��������� ������� ������� �� ���������� �����
INT_PTR CALLBACK AddEditKeyDlgProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);	// ������� - ���������� ��������� ������� "����������/�������������� �������"

// ���������� ����������, � ��� ����������� ��� ��������������/������������ ������� �������. ������������ � ������� "����������/�������������� �������"
char* keyname_edit = new char[256];


void TreeViewInit()
{
	// ������� ImageList � ��������� ��� TreeView
	auto hlist = ImageList_LoadBitmap(hinstance, "RegKeyBitmap", 16, 0, RGB(255, 255, 255));
	throwerr(hlist != NULL, "ImageList_LoadBitmap error");

	// ���������� ImageList ��� TreeView
	auto hwnd_tv = GetDlgItem(hwnd_main, IDC_TREEVIEW);
	throwerr(hwnd_tv != NULL, "GetDlgItem (IDC_TREEVIEW) error");
	TreeView_SetImageList(hwnd_tv, hlist, TVSIL_NORMAL);
	
	// �������� ���������������� (��������) ������� �������
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
	// ���������� ������ ����� ��������� ins
	ins.hParent = parent;			// ������������ �������
	ins.hInsertAfter = TVI_LAST;	// �������� ������� ���������
	ins.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;	// ������������ ������ ���� pszText, iImage, iSelectedImage, lParam
	ins.item.pszText = (char*)text;
	ins.item.iImage = 0;	// ������ �������� � ImageList
	ins.item.iSelectedImage = 0;	// ������ �������� � ImageList, ����� ������ �������� �������
	ins.item.lParam = ITEM_NEVER_EXPANDED;
	// ���� treeview �� �������, ������� ��� ����
	if (treeview == NULL)
		treeview = GetDlgItem(hwnd_main, IDC_TREEVIEW);
	// ������� ��������
	auto hitem = TreeView_InsertItem(treeview, &ins);
	throwerr(hitem != NULL, "TreeView_InsertItem error");
	// ������� "��������" ���-������� � ������ "Expanding...", ������ ��� ����, ����� � �������� �������� ����������� "������"
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


// ���������� WM_NOTIFY
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


// ���������� TVN_ITEMEXPANDING
INT_PTR ProcessItemExpanding(NMTREEVIEW *data)
{
	HWND treeview = data->hdr.hwndFrom;
	HTREEITEM item = data->itemNew.hItem;
	if (data->itemNew.lParam == ITEM_NEVER_EXPANDED)	// ���� ������� ������� ����� �� �������������� "��������"
	{
		// ������ ����������� "��������" ������� "Expanding..."
		auto child_item = TreeView_GetChild(treeview, item);
		throwerr(TreeView_DeleteItem(treeview, child_item), "ProcessItemExpanding: TreeView_DeleteItem error");
		
		// ������� ������ �������
		auto key = GetRegistryKey(item, treeview);
		
		bool has_children = false;	// ����� ��������, ���� �� �� ����� ���� ���������� � �������� �������
		DWORD name_length;
		static char* key_buffer = new char[256];	// ����� ��� ��������� ���� �����������
		LSTATUS status;
		DWORD index = 0;

		// ����� ����������� � �����
		name_length = 256;
		status = RegEnumKeyEx(key.get(), index, key_buffer, &name_length, NULL, NULL, NULL, NULL);
		while (status != ERROR_NO_MORE_ITEMS)	// ERROR_NO_MORE_ITEMS ��������, ��� ����� ����������� ��������
		{
			throwerr(status == ERROR_SUCCESS, "ProcessItemExpanding: RegEnumKeyEx error", false, status);
			index++;
			key_buffer[name_length] = 0;
			has_children = true;
			// ���������� ����������
			TreeViewAddItem(item, key_buffer, treeview);
			name_length = 256;
			status = RegEnumKeyEx(key.get(), index, key_buffer, &name_length, NULL, NULL, NULL, NULL);
		}
		// ��������� ������ ITEM_HAS_BEEN_EXPANDED, ����� ������ ��� ��������� ��������� �� ���������
		TVITEM set_item;
		set_item.hItem = item;
		set_item.mask = TVIF_PARAM;
		set_item.lParam = ITEM_HAS_BEEN_EXPANDED;
		throwerr(TreeView_SetItem(treeview, &set_item), "ProcessItemExpanding: TreeView_SetItem error");
		// ���� ��� �������� ���������, ������ TRUE (����� ��������� ����� ��������); ����� ������ FALSE
		return !has_children;
	}
	return FALSE;
}


// ���������� TVN_SELCHANGED
INT_PTR ProcessSelChanged(NMTREEVIEW *data)
{
	HWND treeview = data->hdr.hwndFrom;
	auto item = data->itemNew.hItem;
	// ������ ���������� � ������ values_list.cpp
	ValuesList_TreeViewSelChanged(GetRegistryKey(item, treeview));
	return FALSE;
}


// ���������� RegistryKey (������-������� ��� HKEY) ��� ������� �������, ���������������� ������� �������� TreeView.
// ������ ����������� � ���������� ������� (access).
// ��� ������������� ������: ���� ���������� ���� throwerror, ������� ����������; ����� �������� ��� ������ � pstatus
RegistryKey GetRegistryKey(HTREEITEM item, HWND treeview, REGSAM access, bool throwerror, LSTATUS* pstatus)
{
	auto item_name = GetItemName(item, treeview);	// �������� ����� � ������� ������
	auto parent = TreeView_GetParent(treeview, item);	// �������� ������������ �������
	if (parent == NULL)
		return GetRootRegistryKey(&item_name[0]);	// ���� ��� �������� ������� (�.�. �������� ���), ������ ��������������� �������� ������
	else
	{
		auto parent_hkey = GetRegistryKey(parent, treeview);	// ���������� ������� RegistryKey ������������� �������
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


// ���������� ����� � ��������� �������� TreeView
string GetItemName(HTREEITEM hitem, HWND treeview)
{
	string result(256, ' ');	// �������� ������ ������ 256 ����
	TVITEM item;
	ZeroMemory(&item, sizeof(TVITEM));
	item.hItem = hitem;
	item.mask = TVIF_TEXT;
	item.cchTextMax = 256;
	item.pszText = &result[0];
	// ����������� �����; ���� ������, ������� ����������
	throwerr(TreeView_GetItem(treeview, &item), "GetItemName: TreeView_GetItem error");
	return result;
}


// ���������� RegistryKey ����������������� ��������� �������
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


// ���������� WM_COMMAND (������� ������: ���������� �������, �������������� �������, �������� �������)
BOOL TreeViewProcessButton(WPARAM wp, LPARAM lp)
{
	HWND hwnd_tv = GetDlgItem(hwnd_main, IDC_TREEVIEW);
	throwerr(hwnd_tv != NULL, "TreeViewProcessButton: GetDlgItem error");
	if (HIWORD(wp) == BN_CLICKED)	// ������������ ������ BN_CLICKED (������� ������)
		switch (LOWORD(wp))
		{
		case IDC_ADD_KEY_BUTTON:	// ������ ������ ���������� �������
		{
			// �������� ������� ������� TreeView. ���� ������� �� ������, �������
			auto selected_item = TreeView_GetSelection(hwnd_tv);
			if (selected_item == NULL)
				return TRUE;
			// ���������� ���������� keyname_edit ������� ������ �������, ��� ����� � 0-� ���� ������� 0 (������� ����� ������)
			keyname_edit[0] = 0;
			// ������� ������ ����������/�������������� �������
			auto result = DialogBox(hinstance, "AddEditKeyDialog", hwnd_main, AddEditKeyDlgProc);
			if (result == IDOK)	// ������ ��� �������� �������� OK
			{
				// ������� ������� ������. � ������ ������ ������� ��������� ������������ (MessageBox), ��� ������������ ���� �������.
				LSTATUS open_key_status;
				auto parent_key = GetRegistryKey(selected_item, hwnd_tv, KEY_READ | KEY_WRITE, false, &open_key_status);
				if (open_key_status != ERROR_SUCCESS)
				{
					MessageBox(hwnd_main, "�� ������� ������� ������ ��� ���������. ��������, ��� ���� �������", "������", MB_OK);
					return TRUE;
				}
				// ����� ��������� ������� � ���������� ���������� keyname_edit ����� ���, ��������� �������������. ���������� ��� ��� �������� ����������.
				// �������� ���������. � ������ ������ ������� ��������� ������������ (MessageBox).
				HKEY created_key;
				auto result = RegCreateKey(parent_key.get(), keyname_edit, &created_key);
				if (result != ERROR_SUCCESS)
				{
					MessageBox(hwnd_main, "�� ������� ������� ������. ��������, ��� ���� �������", "������", MB_OK);
					return TRUE;
				}
				// ������� ��������� ������
				RegCloseKey(created_key);
				// ������� ������� � TreeView
				TreeViewAddItem(selected_item, keyname_edit, hwnd_tv);
			}
			return TRUE;
		}
		case IDC_EDIT_KEY_BUTTON:	// ������ ������ �������������� (��������������) �������
		{
			// �������� ������� ������� TreeView. ���� ������� �� ������, �������
			auto selected_item = TreeView_GetSelection(hwnd_tv);
			if (selected_item == NULL)
				return TRUE;
			// ������� ������������ �������
			auto parent_item = TreeView_GetParent(hwnd_tv, selected_item);
			// ���� �������� ������, �� ������������ �������� ������������� ���������������� �������� ������. ��� ����������, ������������.
			if (parent_item == NULL)
				return TRUE;
			// �������� ����� � ������� ��������
			string item_name = GetItemName(selected_item, hwnd_tv);
			// ����������� ��� � ���������� ���������� keyname_edit, ����� ������ �������������� ���� ������
			strcpy(keyname_edit, &item_name[0]);
			// ������� ������ ����������/�������������� �������
			auto result = DialogBox(hinstance, "AddEditKeyDialog", hwnd_main, AddEditKeyDlgProc);
			if (result == IDOK)	// ������ ��� �������� �������� OK
			{
				// ��������� ������������ ������. ��� ������ ������ ��������� ������������ (MessageBox)
				LSTATUS open_key_status;
				auto parent_key = GetRegistryKey(parent_item, hwnd_tv, KEY_READ | KEY_WRITE, false, &open_key_status);
				if (open_key_status != ERROR_SUCCESS)
				{
					MessageBox(hwnd_main, "�� ������� ������� ������ ��� ���������. ��������, ��� ���� �������", "������", MB_OK);
					return TRUE;
				}
				// ��������� � RegRenameKey ��� A-������, ���������� �������������� �� � W-������ (UTF-16) - ������ ��� ������� (item_name) � ����� ��� ������� (keyname_edit)
				// ������������ ������ ��� �������
				auto old_len = strlen(&item_name[0]) + 1;	// ���������� ����� ������
				wchar_t* old_name_wc_buffer = new wchar_t[old_len * 2];	// �������� ����� ��� W-������ ������
				throwerr(MultiByteToWideChar(CP_ACP, 0, &item_name[0], -1, old_name_wc_buffer, old_len) != 0, "TreeViewProcessButton, edit key button: MultiByteToWideChar (old name) error");	// ������������
				// ������������ ����� ��� �������
				auto new_len = strlen(keyname_edit) + 1;
				wchar_t* new_name_wc_buffer = new wchar_t[new_len * 2];
				throwerr(MultiByteToWideChar(CP_ACP, 0, keyname_edit, -1, new_name_wc_buffer, new_len) != 0, "TreeViewProcessButton, edit key button: MultiByteToWideChar (new name) error");
				// �������������� �������. � ������ ������ ������ ��������� ������������ (MessageBox)
				auto result = RegRenameKey(parent_key.get(), old_name_wc_buffer, new_name_wc_buffer);
				if (result != ERROR_SUCCESS)
				{
					MessageBox(hwnd_main, "�� ������� ������������� ������. ��������, ��� ���� �������", "������", MB_OK);
					return TRUE;
				}
				// ����������� � TreeView
				TVITEM set_item;
				set_item.hItem = selected_item;
				set_item.mask = TVIF_TEXT;
				set_item.pszText = keyname_edit;
				throwerr(TreeView_SetItem(hwnd_tv, &set_item), "TreeViewProcessButton, edit key button: TreeView_SetItem error");
			}
			return TRUE;
		}
		case IDC_DELETE_KEY_BUTTON:	//	������ ������ �������� �������
		{
			// �������� ������� ������� TreeView. ���� ������� �� ������, �������
			auto selected_item = TreeView_GetSelection(hwnd_tv);
			if (selected_item == NULL)
				return TRUE;
			// ������� ������������ �������
			auto parent_item = TreeView_GetParent(hwnd_tv, selected_item);
			if (parent_item == NULL)	// ������� ������� �������� ������ �������. ��� ����������, ������������.
				return TRUE;
			// ��������������
			auto ok_cancel = MessageBox(hwnd_main, "������� ������ �������?\r\n����� ������� ��� ���������� � ���������.\r\n�������������� ����������.", "��������������", MB_OKCANCEL | MB_ICONWARNING);
			if (ok_cancel != IDOK)	// ������������ ��������� - ������������
				return TRUE;
			// �������� ������������ ������, � ������ ������ ��������� ������������ (MessageBox)
			LSTATUS open_key_status;
			auto parent_key = GetRegistryKey(parent_item, hwnd_tv, KEY_READ | KEY_WRITE, false, &open_key_status);
			if (open_key_status != ERROR_SUCCESS)
			{
				MessageBox(hwnd_main, "�� ������� ������� ������ ��� ���������. ��������, ��� ���� �������", "������", MB_OK);
				return TRUE;
			}
			// �������� ��� �������� �������
			string item_name = GetItemName(selected_item, hwnd_tv);
			// �������. � ������ ������ - ��������� ������������ (MessageBox)
			auto result = RegDeleteTree(parent_key.get(), &item_name[0]);
			if (result != ERROR_SUCCESS)
			{
				MessageBox(hwnd_main, "�� ������� ������� ������. ��������, ��� ���� �������", "������", MB_OK);
				return TRUE;
			}
			// ������� �� TreeView
			throwerr(TreeView_DeleteItem(hwnd_tv, selected_item), "TreeViewProcessButton, delete key button: TreeView_DeleteItem error");
			return TRUE;
		}
		}
	return FALSE;
}


// ������� - ���������� ��������� ������� "����������/�������������� �������"
INT_PTR CALLBACK AddEditKeyDlgProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	try
	{
		switch (msg)
		{
		case WM_INITDIALOG:
			// �������� keyname_edit � ���� �������������� (�.�. ���������� ������������ �������)
			throwerr(SetDlgItemText(hwnd, IDC_KEY_NAME_EDIT, keyname_edit) != 0, "AddEditKeyDlgProc: SetDlgItemText error");
			return TRUE;
		case WM_CLOSE:	// ��� ������� �� "�������" ��������� ���� � ����� "������"
			EndDialog(hwnd, IDCANCEL);
			return TRUE;
		case WM_COMMAND:
			if (HIWORD(wp) == BN_CLICKED)	// ������ �����-�� ������
			{
				if (LOWORD(wp) == IDOK)	// ������ OK
				{
					// �������� ����� �� ���� �������������� � ���������� keyname_edit
					GetDlgItemText(hwnd, IDC_KEY_NAME_EDIT, keyname_edit, 256);
					if (strlen(keyname_edit) == 0)
					{
						MessageBox(hwnd, "������� ������ ��� �������", "������", MB_OK);
						return TRUE;
					}
					// ��������� ������
					EndDialog(hwnd, IDOK);
					return TRUE;
				}
				if (LOWORD(wp) == IDCANCEL)
				{
					// ��������� ������ � ����� "������"
					EndDialog(hwnd, IDCANCEL);
					return TRUE;
				}
			}
			return FALSE;
		}
		return FALSE;
	}
	// ����������� ������
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
