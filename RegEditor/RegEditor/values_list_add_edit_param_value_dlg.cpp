
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
				// ��������� ������������� ��������� ��������� �������� � repr
				GetBinaryValueRepr(param_value_ptr, param_value_size_edit, &repr[0]);
				break;
			}
			case 1:
			{
				// � ���� �������������� ��������� ����� "number", �.�. �������� ���� ������ ����
				auto style = GetWindowLong(hwnd_param_value, GWL_STYLE);
				SetWindowLong(hwnd_param_value, GWL_STYLE, style | ES_NUMBER);
				// �������� ����� � �������
				repr_len = 15;
				repr.assign(repr_len, ' ');
				// � repr �������� ��������� ������������� DWORD
				sprintf(&repr[0], "%u", *((DWORD*)&param_value_edit[0]));
				break;
			}
			case 2:
			{
				// � ���� �������������� ��������� ����� "number", �.�. �������� ���� ������ ����
				auto style = GetWindowLong(hwnd_param_value, GWL_STYLE);
				SetWindowLong(hwnd_param_value, GWL_STYLE, style | ES_NUMBER);
				// �������� ����� � �������
				repr_len = 35;
				repr.assign(repr_len, ' ');
				// � repr �������� ��������� ������������� QWORD
				sprintf(&repr[0], "%llu", *((unsigned long long*) & param_value_edit[0]));
				break;
			}
			case 3:
			case 5:
				// ������ ��������� �� param_value_edit � repr
				repr_len = param_value_size_edit - 1;
				if (repr_len > 0)
				{
					repr.assign(repr_len, ' ');
					strcpy(&repr[0], (char*)&param_value_edit[0]);
				}
				break;
			default:
				// ����� �� ������ ����
				throwerr(false, "AddEditParamValueDlgProc: wrong parameter type", false);
			}
			// ��������� ���������� ��������� ������������� �������� �� repr � ���� �����
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
					// �������� ������ ���������� ��������
					int repr_len = GetWindowTextLength(hwnd_param_value);
					string repr(repr_len, ' ');
					GetWindowText(hwnd_param_value, &repr[0], repr_len + 1);
					// �������� ������������ �����, ����������� repr, ��������� ������������������ �������� �� repr � param_value_edit � param_value_size_edit
					switch (param_type_edit)
					{
					case 0:	// REG_BINARY
						if (repr_len % 2 != 0)
						{
							MessageBox(hwnd, "�������� ������ ��������� ��������: �������� ����� ��������", "������", MB_OK);
							return TRUE;
						}
						// ��������� �����
						param_value_size_edit = repr_len / 2;
						param_value_edit.assign(param_value_size_edit, 0);
						if (param_value_size_edit > 0)
						{
							// ����������� �� ���������� ������������� � ��������
							int bad_char_index;
							if (!ValueReprToBinary(&repr[0], repr_len, &param_value_edit[0], &bad_char_index))
							{
								char message_buf[150];
								sprintf(message_buf, "�������� ������ ��������� ��������: �������� ������ %c � ������� %d", repr[bad_char_index], bad_char_index + 1);
								MessageBox(hwnd, message_buf, "������", MB_OK);
								return TRUE;
							}
						}
						break;
					case 1:	// REG_DWORD
					{
						// ��������� �����
						param_value_size_edit = 4;
						param_value_edit.assign(4, 0);
						size_t end_pos;
						// ��������������� �� ���������� ������������� � DWORD
						try
						{
							*((DWORD*)&param_value_edit[0]) = stoul(repr, &end_pos);
						}
						catch (invalid_argument)
						{
							MessageBox(hwnd, "�������� ������ ��������", "������", MB_OK);
							return TRUE;
						}
						catch (out_of_range)
						{
							MessageBox(hwnd, "������� ������� ��������", "������", MB_OK);
							return TRUE;
						}
						// �� ��� ������ �����������������, �.�. � ������ ���� ����������� ���������� �������
						if (end_pos != repr.length())
						{
							MessageBox(hwnd, "�������� ������", "������", MB_OK);
							return TRUE;
						}
						break;
					}
					case 2:	// REG_QWORD
					{
						// ���������� DWORD, ������ 8 ����
						param_value_size_edit = 8;
						param_value_edit.assign(8, 0);
						size_t end_pos;
						try
						{
							*((unsigned long long*) & param_value_edit[0]) = stoull(repr, &end_pos);
						}
						catch (invalid_argument)
						{
							MessageBox(hwnd, "�������� ������ ��������", "������", MB_OK);
							return TRUE;
						}
						catch (out_of_range)
						{
							MessageBox(hwnd, "������� ������� ��������", "������", MB_OK);
							return TRUE;
						}
						if (end_pos != repr.length())
						{
							MessageBox(hwnd, "�������� ������", "������", MB_OK);
							return TRUE;
						}
						break;
					}
					case 3:	// REG_SZ
					case 5:	// REG_MULTI_SZ
						// ������ ��������
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
