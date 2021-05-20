
#include "values_list.h"


// ������� - ���������� ��������� ������� �������������� �������������� ��������� �������
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
			// �������� �� ������ ������: ������ ��������� ������ ���� > 0, ��������� ���� ������ ���� 0
			throwerr(param_value_size_edit > 0, "AddEditMultistringValueDlgProc: internal error (param_value_size_edit must be > 0)", false);
			throwerr(param_value_edit[param_value_size_edit - 1] == 0, "AddEditMultistringValueDlgProc: internal error (param_value_edit[param_value_size_edit-1] must be 0)", false);
			
			// � ������������� ��������� ������ ��������� ������� ������, � � ����� ����� ��� ������� �����.
			// ����� ��� �� ��������� ������������ � ���������, ����� �������� ��� (����� ����������) ������� ����� �� ������ �������� ������, \r\n
			// ��� ��� ������ ������� ���� ���������� �� ���, ���������� �������� �������������� �����
			
			// ���������� ����� ������� ����, ����� ����������
			auto null_count = count(param_value_edit.begin(), param_value_edit.end(), 0) - 1;
			// ������� ���������� repr, ��������� ���� �������� ���������
			string repr(param_value_size_edit - 1, ' ');
			memcpy(&repr[0], (char*)&param_value_edit[0], param_value_size_edit);
			// ������������� ����������� �������������� ����� (��������� �����������, ����� ����� � �� ������)
			repr.reserve(param_value_size_edit - 1 + null_count);
			// ������� 0-����� �� \r\n
			size_t start_pos = 0;
			while ((start_pos = repr.find('\0', start_pos)) != string::npos) {
				repr.replace(start_pos, 1, "\r\n");
				start_pos += 2;
			}
			// ���������� ����� ��������� � ���� ��������������
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
					// �������� ������ ��������, ���������� ������������� � ���� ��������������
					int repr_len = GetWindowTextLength(hwnd_param_value);
					// ������� ���������� repr
					string repr(repr_len, ' ');
					// �������� �������� �� ���� �������������� � ���������� repr
					GetWindowText(hwnd_param_value, &repr[0], repr_len + 1);
					// ������� �������� ����� \r\n �� ������� �����
					size_t start_pos = 0;
					while ((start_pos = repr.find("\r\n", start_pos)) != string::npos) {
						repr.replace(start_pos, 2, 1, '\0');
						start_pos += 1;
					}
					// � ������������ �� ������ ���� ������ ����� (��� ������� ������������ WinAPI), �.�. �� ������ ���� ��� ������ ������� ����� (������ � ����� �����)
					string zz(2, 0);
					if (repr.find(zz) != string::npos)
					{
						MessageBox(hwnd, "������������ �� ������ ��������� ������ �����", "������", MB_OK);
						return TRUE;
					}
					// ���� ������������ � ����� ����� �� �������� ������� ������, � ��� ����� ��������� ������ �� ����� ������������ ����-�����. ����� ��� ��������
					if (repr.length() == 0 || repr[repr.length() - 1] != 0)
						repr.append(1, 0);
					// ��������������� �������� �������� � param_value_edit � ��� ������ �������������� � param_value_size_edit
					param_value_size_edit = repr.length() + 1;
					param_value_edit.assign(param_value_size_edit, 0);
					memcpy((char*)&param_value_edit[0], &repr[0], param_value_size_edit);
					// ��������� ������
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
	// ����������� ����������
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
