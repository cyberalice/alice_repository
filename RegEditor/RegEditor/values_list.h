#pragma once

// ����� ������������ ���� ��� ���� ������� value_list_xxx.cpp


#include "main.h"
#include <windowsx.h>
#include <string>
#include <vector>
#include <algorithm>
#include <stdexcept>

void GetBinaryValueRepr(BYTE* value, DWORD value_length, char* repr);	// �������� ������� ������������� ��������� ��������� ������� � ����������������� ����
void AddItem(LVITEM& item, DWORD index, HWND hwnd_list, DWORD value_type, char* name_buf, BYTE* value_buf, DWORD value_len, char* value_repr_buf);	// �������� �������� ������� � ListView
bool ValueReprToBinary(char* repr, int repr_len, BYTE* result, int* bad_char_index);	// �������������� ���������� ������������� ��������� �������� � ���� �������� ��������
INT_PTR CALLBACK ChooseParamTypeDlgProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);		// ������� - ���������� ��������� ������� ������ ���� ��������� �������
INT_PTR CALLBACK AddEditParamNameDlgProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);	// ������� - ���������� ��������� ������� ����������/�������������� ����� ��������� �������
INT_PTR CALLBACK AddEditParamValueDlgProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);	// ������� - ���������� ��������� ������� ����������/�������������� �������� ��������� �������
INT_PTR CALLBACK AddEditMultistringValueDlgProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);			// ������� - ���������� ��������� ������� �������������� �������������� ��������� �������


// ���������� ����������, � ��� �����������: ��� ������������ ��������� ������� (������������ � ������� ������ ���� ���������), ��� ������������/��������������
// ��������� (������������ � ������� ����� ����� ������������/�������������� ���������), ������ � �������� ������������/�������������� ���������
// (������������ � ������� ����� �������� ���������).
extern int param_type_edit;	// � ������� ������ ���� ������������ ��������� ��������: 0=REG_BINARY, 1=REG_DWORD, 2=REG_QWORD, 3=REG_SZ, 4=REG_MULTI_SZ, 5=REG_EXPAND_SZ
extern string param_name_edit;
extern int param_value_size_edit;
extern vector<BYTE> param_value_edit;
