
#include "main.h"

RegistryKey::RegistryKey(HKEY hk)
	:_hkey(hk)	// ��������� ���������� hk
{
}


RegistryKey::RegistryKey(RegistryKey&& other)	// Move constructor
	:_hkey(NULL)
{
	_hkey = other._hkey;	// ��������� � ���� hkey ����������� ������� other
	other._hkey = NULL;		// � ����������� ������� other �������� ���� _hkey, ����� �� ����������� �������� ������������ �������� ��� ������� ������� � �������������� (�����������) �������
}


HKEY RegistryKey::get()
{
	return _hkey;
}


RegistryKey::~RegistryKey()
{
	if (_hkey != NULL)
		RegCloseKey(_hkey);	// ����������� hkey, ���� �� �� NULL
	_hkey = NULL;
}


const char* RegistryKey::GetValueTypeString(DWORD t)
{
	// ���������� ������, ����������� ��� ������ �������
	switch (t)
	{
	case REG_BINARY:
		return "REG_BINARY";
	case REG_DWORD:
		return "REG_DWORD";
	case REG_QWORD:
		return "REG_QWORD";
	case REG_SZ:
		return "REG_SZ";
	case REG_MULTI_SZ:
		return "REG_MULTI_SZ";
	case REG_EXPAND_SZ:
		return "REG_EXPAND_SZ";
	case REG_NONE:
		return "REG_NONE";
	}
	// ������� ���������� - ����������� ��� ������
	throwerr(false, "RegistryKey::GetValueTypeString: wrong registry value type");
	return NULL;
}
