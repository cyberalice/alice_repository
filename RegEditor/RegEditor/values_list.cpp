
#include "values_list.h"

// ���������� ����������, � ��� �����������: ��� ������������ ��������� ������� (������������ � ������� ������ ���� ���������), ��� ������������/��������������
// ��������� (������������ � ������� ����� ����� ������������/�������������� ���������), ������ � �������� ������������/�������������� ���������
// (������������ � ������� ����� �������� ���������).
int param_type_edit = LB_ERR;	// � ������� ������ ���� ������������ ��������� ��������: 0=REG_BINARY, 1=REG_DWORD, 2=REG_QWORD, 3=REG_SZ, 4=REG_MULTI_SZ, 5=REG_EXPAND_SZ,
								// �������������� ��� ������ ������� WinAPI ����� ����� �������������� �� ����� "���������" � ��, ������� �������� Windows, � ��������
string param_name_edit;
int param_value_size_edit;
vector<BYTE> param_value_edit;


// ������������� ListView
void ValuesListInit()
{
	auto hwnd_vl = GetDlgItem(hwnd_main, IDC_VALUES_LIST);
	throwerr(hwnd_vl != NULL, "GetDlgItem (IDC_VALUES_LIST) error");

	// ���������� ������� � ListView
	LVCOLUMN lvc;
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;	// ������������ ������ ���� fmt, cx, pszText, iSubItem
	// ������� 0 - ��� ���������
	lvc.iSubItem = 0;
	lvc.fmt = LVCFMT_LEFT;
	lvc.cx = 100;
	lvc.pszText = (char*)"���";
	throwerr(ListView_InsertColumn(hwnd_vl, 0, &lvc) != -1, "ValuesListInit: ListView_InsertColumn(0) error");
	// ������� 1 - ��� ���������
	lvc.iSubItem = 1;
	lvc.cx = 100;
	lvc.pszText = (char*)"���";
	throwerr(ListView_InsertColumn(hwnd_vl, 1, &lvc) != -1, "ValuesListInit: ListView_InsertColumn(1) error");
	// ������� 2 - �������� ��������� � ������� ����
	lvc.iSubItem = 2;
	lvc.cx = 100;
	lvc.pszText = (char*)"��������";
	throwerr(ListView_InsertColumn(hwnd_vl, 2, &lvc) != -1, "ValuesListInit: ListView_InsertColumn(2) error");
}


// ���������� ��� ��������� ������� ������ � TreeView, ����� ListView ����������� ���� ����������
void ValuesList_TreeViewSelChanged(RegistryKey key)
{
	HWND hwnd_list = GetDlgItem(hwnd_main, IDC_VALUES_LIST);
	// �������� ���� ����� �� ListView
	throwerr(ListView_DeleteAllItems(hwnd_list), "ValuesList_TreeViewSelChanged: ListView_DeleteAllItems error");
	
	// �������� ������ ������ �������� ����� � ������ ������ �������� ��������, ����� �������� ������ ������������ �������
	LSTATUS status;
	DWORD max_name_len;
	DWORD max_value_len;
	status = RegQueryInfoKey(key.get(), 0, 0, 0, 0, 0, 0, 0, &max_name_len, &max_value_len, 0, 0);
	throwerr(status == ERROR_SUCCESS, "ValuesList_TreeViewSelChanged: RegQueryInfoKey error", false, status);
	// ������ ������ ��� ����� ������ ���� �� ������ 20, ����� �������� �� ��������� (� ������ ������) �� ����� ���������� ��� "(�� ���������)". �� ����� ���� 15 �������, �� � ���� � �������.
	if (max_name_len < 20)
		max_name_len = 20;
	// ��������� ����� ��� ����������� ������� ����
	max_name_len++;
	max_value_len++;

	// ��������� ������
	char *name_buf = new char[max_name_len];
	BYTE* value_buf = new BYTE[max_value_len];
	char value_repr_buf[100];	// ����� ��� �������� ���������� ������������� ��������

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
	//	strcpy(name_buf, "(�� ���������)");
	//	AddItem(item, 0, hwnd_list, value_type, name_buf, value_buf, value_len, value_repr_buf);
	//}

	// ����� ���������� ������� � �����
	DWORD index = 0;
	name_len = max_name_len;
	value_len = max_value_len;
	status = RegEnumValue(key.get(), index, name_buf, &name_len, 0, &value_type, value_buf, &value_len);	// ������ ������� ���������
	while (status != ERROR_NO_MORE_ITEMS)	// ERROR_NO_MORE_ITEMS ��������, ��� ����� ��������
	{
		throwerr(status == ERROR_SUCCESS, "ValuesList_TreeViewSelChanged: RegEnumValue error", false, status);
		if (name_len == 0)	// Default value
			strcpy(name_buf, "(�� ���������)");
		else
			name_buf[name_len] = 0;
		// ���������� �������� � ListView
		AddItem(item, index, hwnd_list, value_type, name_buf, value_buf, value_len, value_repr_buf);

		index++;
		name_len = max_name_len;
		value_len = max_value_len;
		status = RegEnumValue(key.get(), index, name_buf, &name_len, 0, &value_type, value_buf, &value_len);	// ������ ���������� ���������
	}
	// ������������ �������
	delete[] name_buf;
	delete[] value_buf;
}


// �������� � ListView ������� ��������� ������������� �������� ��������� �������
void SetItemValueRepr(HWND hwnd_list, DWORD index, DWORD value_type, BYTE *value_buf, DWORD value_len, char *value_repr_buf)
{
	switch (value_type)
	{
	case REG_BINARY:
		// ����� �� ����� 30 ����
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
		// ��� ������������ ����� ������ ������ ������ (� �� ����� 99 ��������)
		strncpy(value_repr_buf, (char*)value_buf, 99);
		break;
	case REG_EXPAND_SZ:
		// �� �� �����, ��� REG_SZ
		strncpy(value_repr_buf, (char*)value_buf, 99);
		break;
	case REG_NONE:
		value_repr_buf[0] = 0;
		break;
	default:
		value_repr_buf[0] = 0;
	}
	// �������� � ListView
	ListView_SetItemText(hwnd_list, index, 2, value_repr_buf);
}


// �������� �������� ������� � ListView
void AddItem(LVITEM& item, DWORD index, HWND hwnd_list, DWORD value_type, char* name_buf, BYTE* value_buf, DWORD value_len, char* value_repr_buf)
{
	// ������� ����� ���������
	item.pszText = name_buf;
	item.iItem = index;
	throwerr(ListView_InsertItem(hwnd_list, &item) != -1, "ValuesList_TreeViewSelChanged: ListView_InsertItem error");

	// ������� ���� ��������
	ListView_SetItemText(hwnd_list, index, 1, (char*)RegistryKey::GetValueTypeString(value_type));

	// ������� �������� �������������
	SetItemValueRepr(hwnd_list, index, value_type, value_buf, value_len, value_repr_buf);
}


// �������� ������� ������������� ��������� ��������� ������� � ����������������� ����
void GetBinaryValueRepr(BYTE *value, DWORD value_length, char *repr)
{
	static const char digits[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
	DWORD repr_len = 0;
	for (DWORD i = 0; i < value_length; ++i)
	{
		BYTE b = value[i];
		repr[repr_len++] = digits[b >> 4];	// ������� �������� �������� ������� �� 4 ���� ������
		repr[repr_len++] = digits[b & 0xF];	// ������� �������� �������� ��������� AND � ������ 00001111
	}
	repr[repr_len] = 0;	// ����������� ������� ������
}


// �������������� ���������� ������������� 16-����� ����� � ���� �����. ������� false � ������ �������.
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


// �������������� ���������� ������������� ��������� �������� � ���� �������� �������� (�������� ������). repr_len ������ ���� ������ 2.
// � ������ ������� ������������ false, � ������ ��������� ������� ������������ ����� bad_char_index
bool ValueReprToBinary(char *repr, int repr_len, BYTE *result, int *bad_char_index)
{
	int bytes_len = repr_len / 2;	// ����� ���� = ����� �������� / 2
	for (int pos = 0; pos < bytes_len; pos++)
	{
		// �������� ������� ��������
		BYTE b1;
		if (!GetHexFromChar(repr[pos * 2], &b1))
		{
			*bad_char_index = pos * 2;
			return false;
		}
		// �������� ������� ��������
		BYTE b2;
		if (!GetHexFromChar(repr[pos * 2 + 1], &b2))
		{
			*bad_char_index = pos * 2 + 1;
			return false;
		}
		// ���������� ��������� � �������� � result
		result[pos] = (b1 << 4) | b2;
	}
	return true;
}


// �������� ����� �������� ListView (�.�. ��� ��������� �������)
string GetListViewItemText(HWND hwnd, int idx)
{
	// ����� ����� ������� ����������.
	// ������� ����� ����������� ��� � ����� �� ��� ���,
	// ���� ������������ ����� returned_len �� ������ ������ ������� ������ len, �� ������ ���� ���������� ����� � 1,5 ����.
	// ������ ��� ����� ���������, ��� ��� ������� ������ ���, �� ����������.
	size_t len = 8;
	string name;
	size_t returned_len;
	LVITEM item;
	item.iSubItem = 0;
	do
	{
		// ����������� ����� � 1,5 ����
		len += len / 2;
		name.assign(len, ' ');
		// ����������� ���
		item.pszText = &name[0];
		item.cchTextMax = len + 1;
		returned_len = (size_t)SendMessage(hwnd, LVM_GETITEMTEXT, idx, (LPARAM)&item);
	} while (returned_len >= len);
	// ������ ������, ���������������� ����� � ������
	name.erase(returned_len, len - returned_len);
	return name;
}


// ��������� WM_COMMAND (������� ������ ����������, ��������������, �������������� � �������� ���������)
BOOL ValuesListProcessButton(WPARAM wp, LPARAM lp)
{
	HWND hwnd_vl = GetDlgItem(hwnd_main, IDC_VALUES_LIST);
	HWND hwnd_tv = GetDlgItem(hwnd_main, IDC_TREEVIEW);
	throwerr(hwnd_vl != NULL, "ValuesListProcessButton: GetDlgItem error");
	if (HIWORD(wp) == BN_CLICKED)	// ������������ ������ ��������� BN_CLICKED (������� ������)
		switch (LOWORD(wp))
		{
		case IDC_ADD_PARAM_BUTTON:	// ������ ������ ���������� ���������
		{
			// �������� ������� ������� TreeView. ���� ������� �� ������, �������
			auto selected_key = TreeView_GetSelection(hwnd_tv);
			if (selected_key == NULL)
				return TRUE;
			// ������� ������ ������ ���� ���������
			auto result = DialogBox(hinstance, "ChooseParamTypeDialog", hwnd_main, ChooseParamTypeDlgProc);
			if (result == IDOK)	// ������ ��
			{
				// ������� ����� ���������
				param_name_edit.clear();
				// ������� ������ ����� ����� ���������
				result = DialogBox(hinstance, "AddEditParamNameDialog", hwnd_main, AddEditParamNameDlgProc);
				if (result == IDOK)	// ������ ��
				{
					// ������� ������� ������. � ������ ������ ������� ��������� ������������ (MessageBox), ��� ������������ ���� �������.
					LSTATUS status;
					auto key = GetRegistryKey(selected_key, hwnd_tv, KEY_READ | KEY_WRITE, false, &status);
					if (status != ERROR_SUCCESS)
					{
						MessageBox(hwnd_main, "�� ������� ������� ������ ��� ���������. ��������, ��� ���� �������", "������", MB_OK);
						return TRUE;
					}
					// ���������, ��� ��������� � ��������� ������ � ������� ������� �� ����������
					status = RegQueryValueEx(key.get(), &param_name_edit[0], NULL, NULL, NULL, NULL);
					if (status != ERROR_FILE_NOT_FOUND)
					{
						throwerr(status == ERROR_SUCCESS, "ValuesListProcessButton, add param button: RegQueryValueEx error", false, status);
						// ���� ���������� - ������ MessageBox � �������
						MessageBox(hwnd_main, "����� �������� ��� ����������", "������", MB_OK);
						return TRUE;
					}
					if (param_type_edit != 4)	// ��� ����� 0 (REG_BINARY), 1 (REG_DWORD), 2 (REG_QWORD), 3 (REG_SZ), 5 (REG_EXPAND_SZ) ������������ ������ AddEditParamValueDialog,
												// � ��� ���� 4 (REG_MULTI_SZ) ������ ������
					{
						// ��������� "������" �������� ���������
						switch (param_type_edit)
						{
						case 0:		// ��� REG_BINARY ��� ������ ���� ������� �����
							param_value_size_edit = 0;
							param_value_edit.clear();
							break;
						case 1:		// ��� REG_DWORD ��� 4 ����� �� ��������� 0
							param_value_size_edit = 4;
							param_value_edit.assign(4, 0);
							break;
						case 2:		// ��� REG_QWORD ��� 8 ���� �� ��������� 0
							param_value_size_edit = 8;
							param_value_edit.assign(8, 0);
							break;
						case 3:		// ��� ����� REG_SZ � REG_EXPAND_SZ ��� ������ ������ (1 ���� - ��� ������� ����� ������)
						case 5:
							param_value_size_edit = 1;
							param_value_edit.assign(1, 0);
							break;
						}
						// �������� ������ ����� �������� ���������
						result = DialogBox(hinstance, "AddEditParamValueDialog", hwnd_main, AddEditParamValueDlgProc);
					}
					else	// ��� ���� 4 (REG_MULTI_SZ) ����� ������ AddEditMultistringValueDialog.
					{
						// ������ ������ (1 ���� - ��� ������� ����� ������)
						param_value_size_edit = 1;
						param_value_edit.assign(1, 0);
						result = DialogBox(hinstance, "AddEditMultistringValueDialog", hwnd_main, AddEditMultistringValueDlgProc);
					}

					if (result == IDOK)	// ������ ��
					{
						// ����������� �� "�����" ��������� � ��, ������� Windows ��������
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
							// ������ �������� �� ������ ����, ��� ������
							throwerr(true, "ValuesListProcessButton, add param button: internal error (param_type_edit)", false);
						}

						// ���� �������� REG_BINARY ������� �����, �� ������� ����� ����� &param_value_edit[0] ������� ����������,
						// ������� � ���� ������ ������ ����� ������� ���������.
						// ������� RegSetValueEx � AddItem ��������� �������� � ������� ����������.
						BYTE* data = param_value_size_edit == 0 ? NULL : &param_value_edit[0];
						// ��������� �������� � ������. � ������ ������ ���������� MessageBox � �������
						status = RegSetValueEx(key.get(), &param_name_edit[0], NULL, param_type, data, param_value_size_edit);
						if (status != ERROR_SUCCESS)
						{
							MessageBox(hwnd_main, "�� ������� �������� ��������. ��������, ��� ���� �������", "������", MB_OK);
							return TRUE;
						}
						
						// ��������� � ListView (� �����)
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
		case IDC_RENAME_PARAM_BUTTON:	// ������ ������ �������������� ���������
		{
			// ������� ������: � WinAPI ��� ������� �������������� ���������. ������� ��������� ���: ��������� �������� � ����� ������ � � ��� �� ���������, � ������ �������� �������.

			// �������� ������� ������� TreeView. ���� ������� �� ������, �������
			auto selected_key = TreeView_GetSelection(hwnd_tv);
			if (selected_key == NULL)
				return TRUE;
			// ����� ���������� �������� � ListView. ���� �� ������, �������
			auto selected_param_idx = ListView_GetNextItem(hwnd_vl, -1, LVNI_SELECTED);
			if (selected_param_idx == -1)
				return TRUE;
			// �������� ��� ��������� �� ListView
			string old_param_name = GetListViewItemText(hwnd_vl, selected_param_idx);
			param_name_edit = old_param_name;
			// �������� ������ ����� ����� ���������
			auto result = DialogBox(hinstance, "AddEditParamNameDialog", hwnd_main, AddEditParamNameDlgProc);
			if (result == IDOK)	// ������ ��
			{
				// ���� ��������� ��� ��������� �� ������, ������ ������ �� �����
				if (old_param_name == param_name_edit)
					return TRUE;
				// ������� ������� ������. � ������ ������ ������� ��������� ������������ (MessageBox), ��� ������������ ���� �������.
				LSTATUS status;
				auto key = GetRegistryKey(selected_key, hwnd_tv, KEY_READ | KEY_WRITE, false, &status);
				if (status != ERROR_SUCCESS)
				{
					MessageBox(hwnd_main, "�� ������� ������� ������ ��� ���������. ��������, ��� ���� �������", "������", MB_OK);
					return TRUE;
				}
				// ��������, ��� ��������� � ����� ������ �� ����������
				status = RegQueryValueEx(key.get(), &param_name_edit[0], NULL, NULL, NULL, NULL);
				if (status != ERROR_FILE_NOT_FOUND)
				{
					throwerr(status == ERROR_SUCCESS, "ValuesListProcessButton, rename param button: RegQueryValueEx error", false, status);
					MessageBox(hwnd_main, "����� �������� ��� ����������", "������", MB_OK);
					return TRUE;
				}
				// �������� ������ �������� ���������. ��� ������ ������� MessageBox � ������
				DWORD value_size;
				status = RegQueryValueEx(key.get(), &old_param_name[0], NULL, NULL, NULL, &value_size);
				if (status != ERROR_SUCCESS)
				{
					MessageBox(hwnd_main, "������ �������������� ���������. ��������, ��� ���� �������", "������", MB_OK);
					return TRUE;
				}
				param_value_edit.assign(value_size, 0);
				DWORD value_type;
				BYTE* data_ptr = value_size == 0 ? NULL : &param_value_edit[0];	// ���� �������� REG_BINARY ������� �����, �� ������� ����� ����� &param_value_edit[0] ������� ����������, ������� ����� ������� ���������.
				// �������� ���� �������� ���������. ��� ������ ������� MessageBox � ������
				status = RegQueryValueEx(key.get(), &old_param_name[0], NULL, &value_type, data_ptr, &value_size);
				if (status != ERROR_SUCCESS)
				{
					MessageBox(hwnd_main, "������ �������������� ���������. ��������, ��� ���� �������", "������", MB_OK);
					return TRUE;
				}
				// ������� ����� ��������. ��� ������ ������� MessageBox � ������
				status = RegSetValueEx(key.get(), &param_name_edit[0], NULL, value_type, data_ptr, value_size);
				if (status != ERROR_SUCCESS)
				{
					MessageBox(hwnd_main, "������ �������������� ���������. ��������, ��� ���� �������", "������", MB_OK);
					return TRUE;
				}
				// ������ ������ ��������. ��� ������ ������� MessageBox � ������
				status = RegDeleteValue(key.get(), &old_param_name[0]);
				if (status != ERROR_SUCCESS)
				{
					MessageBox(hwnd_main, "������ �������������� ���������. ��������, ��� ���� �������", "������", MB_OK);
					return TRUE;
				}
				// ����������� � ListView
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
		case IDC_EDIT_PARAM_BUTTON:	// ������ ������ �������������� ���������
		{
			// �������� ������� ������� TreeView. ���� ������� �� ������, �������
			auto selected_key = TreeView_GetSelection(hwnd_tv);
			if (selected_key == NULL)
				return TRUE;
			// ����� ���������� �������� � ListView. ���� �� ������, �������
			auto selected_param_idx = ListView_GetNextItem(hwnd_vl, -1, LVNI_SELECTED);
			if (selected_param_idx == -1)
				return TRUE;
			// �������� ��� ��������� �� ListView
			param_name_edit = GetListViewItemText(hwnd_vl, selected_param_idx);
			// ������� ������� ������. � ������ ������ ������� ��������� ������������ (MessageBox), ��� ������������ ���� �������.
			LSTATUS status;
			auto key = GetRegistryKey(selected_key, hwnd_tv, KEY_READ | KEY_WRITE, false, &status);
			if (status != ERROR_SUCCESS)
			{
				MessageBox(hwnd_main, "�� ������� ������� ������ ��� ���������. ��������, ��� ���� �������", "������", MB_OK);
				return TRUE;
			}
			// �������� ������ �������� ���������. � ������ ������ ������� MessageBox � ������.
			status = RegQueryValueEx(key.get(), &param_name_edit[0], NULL, NULL, NULL, (DWORD*)&param_value_size_edit);
			if (status != ERROR_SUCCESS)
			{
				MessageBox(hwnd_main, "������ ��������� ���������. ��������, ��� ���� �������", "������", MB_OK);
				return TRUE;
			}
			param_value_edit.assign(param_value_size_edit, 0);
			DWORD value_type;
			BYTE* data_ptr = param_value_size_edit == 0 ? NULL : &param_value_edit[0];	// ���� �������� REG_BINARY ������� �����, �� ������� ����� ����� &param_value_edit[0] ������� ����������, ������� ����� ������� ���������.
			// �������� ���� �������� ���������. � ������ ������ ������� MessageBox � ������.
			status = RegQueryValueEx(key.get(), &param_name_edit[0], NULL, &value_type, data_ptr, (DWORD*)&param_value_size_edit);
			if (status != ERROR_SUCCESS)
			{
				MessageBox(hwnd_main, "������ ��������� ���������. ��������, ��� ���� �������", "������", MB_OK);
				return TRUE;
			}
			// ����������� ��� ��������� �� "���������" Windows � ���� "���������".
			switch (value_type)
			{
			case REG_NONE:	// �������� REG_NONE �� �������������, �������
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
				// ������ �� ������ ����
				throwerr(false, "ValuesListProcessButton, edit param button: internal error (value_type)", false);
			}
			INT_PTR result;
			// �������� ������ ��������������. ��� ���� 4 (REG_MULTI_SZ) ��� ����� AddEditMultistringValueDialog, ��� ��������� ����� AddEditParamValueDialog
			if (param_type_edit != 4)
				result = DialogBox(hinstance, "AddEditParamValueDialog", hwnd_main, AddEditParamValueDlgProc);
			else
				result = DialogBox(hinstance, "AddEditMultistringValueDialog", hwnd_main, AddEditMultistringValueDlgProc);
			if (result == IDOK)	// ������ �������� �������� ��
			{
				BYTE* data_ptr = param_value_size_edit == 0 ? NULL : &param_value_edit[0];	// ���� �������� REG_BINARY ������� �����, �� ������� ����� ����� &param_value_edit[0] ������� ����������, ������� ����� ������� ���������.
				// �������� �������� ���������. � ������ ������ ������ MessageBox � �������.
				status = RegSetValueEx(key.get(), &param_name_edit[0], NULL, value_type, data_ptr, param_value_size_edit);
				if (status != ERROR_SUCCESS)
				{
					MessageBox(hwnd_main, "������ �������������� ���������. ��������, ��� ���� �������", "������", MB_OK);
					return TRUE;
				}
				// �������� ������� ������������� �������� � ListView
				char repr_buf[100];
				SetItemValueRepr(hwnd_vl, selected_param_idx, value_type, data_ptr, param_value_size_edit, repr_buf);
			}
			return TRUE;
		}
		case IDC_DELETE_PARAM_BUTTON:	// ������ ������ �������� ���������
		{
			// �������� ������� ������� TreeView. ���� ������� �� ������, �������
			auto selected_key = TreeView_GetSelection(hwnd_tv);
			if (selected_key == NULL)
				return TRUE;
			// ����� ���������� �������� � ListView. ���� �� ������, �������
			auto selected_param_idx = ListView_GetNextItem(hwnd_vl, -1, LVNI_SELECTED);
			if (selected_param_idx == -1)
				return TRUE;
			// �������� ��� ��������� �� ListView
			param_name_edit = GetListViewItemText(hwnd_vl, selected_param_idx);
			// ������� ������� ������. � ������ ������ ������� ��������� ������������ (MessageBox), ��� ������������ ���� �������.
			LSTATUS status;
			auto key = GetRegistryKey(selected_key, hwnd_tv, KEY_READ | KEY_WRITE, false, &status);
			if (status != ERROR_SUCCESS)
			{
				MessageBox(hwnd_main, "�� ������� ������� ������ ��� ���������. ��������, ��� ���� �������", "������", MB_OK);
				return TRUE;
			}
			// ������� ��������. � ������ ������ ������ MessageBox � �������.
			status = RegDeleteValue(key.get(), &param_name_edit[0]);
			if (status != ERROR_SUCCESS)
			{
				MessageBox(hwnd_main, "������ �������� ���������. ��������, ��� ���� �������", "������", MB_OK);
				return TRUE;
			}
			// ������� �� ListView
			throwerr(ListView_DeleteItem(hwnd_vl, selected_param_idx), "ValuesListProcessButton, delete param button: ListView_DeleteItem error");
			return TRUE;
		}
		}
	return FALSE;
}
