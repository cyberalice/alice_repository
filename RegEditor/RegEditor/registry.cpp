
#include "main.h"

RegistryKey::RegistryKey(HKEY hk)
	:_hkey(hk)	// Сохраняем переданный hk
{
}


RegistryKey::RegistryKey(RegistryKey&& other)	// Move constructor
	:_hkey(NULL)
{
	_hkey = other._hkey;	// Сохраняем у себя hkey переданного объекта other
	other._hkey = NULL;		// У переданного объекта other обнуляем поле _hkey, чтобы не происходило двойного освобождения ресурсов или попытки доступа к освобожденному (невалидному) ресурсу
}


HKEY RegistryKey::get()
{
	return _hkey;
}


RegistryKey::~RegistryKey()
{
	if (_hkey != NULL)
		RegCloseKey(_hkey);	// Освобождаем hkey, если он не NULL
	_hkey = NULL;
}


const char* RegistryKey::GetValueTypeString(DWORD t)
{
	// Возвращает строку, описывающую тип данных реестра
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
	// Бросаем исключение - неизвестный тип данных
	throwerr(false, "RegistryKey::GetValueTypeString: wrong registry value type");
	return NULL;
}
