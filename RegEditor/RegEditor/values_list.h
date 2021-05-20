#pragma once

// ќбщий заголовочный файл дл€ всех модулей value_list_xxx.cpp


#include "main.h"
#include <windowsx.h>
#include <string>
#include <vector>
#include <algorithm>
#include <stdexcept>

void GetBinaryValueRepr(BYTE* value, DWORD value_length, char* repr);	// ѕолучить краткое представление двоичного параметра реестра в шестнадцатеричном виде
void AddItem(LVITEM& item, DWORD index, HWND hwnd_list, DWORD value_type, char* name_buf, BYTE* value_buf, DWORD value_len, char* value_repr_buf);	// ƒобавить параметр реестра в ListView
bool ValueReprToBinary(char* repr, int repr_len, BYTE* result, int* bad_char_index);	// ѕреобразование строкового представлени€ двоичного значени€ в само двоичное значение
INT_PTR CALLBACK ChooseParamTypeDlgProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);		// ‘ункци€ - обработчик сообщений диалога выбора типа параметра реестра
INT_PTR CALLBACK AddEditParamNameDlgProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);	// ‘ункци€ - обработчик сообщений диалога добавлени€/редактировани€ имени параметра реестра
INT_PTR CALLBACK AddEditParamValueDlgProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);	// ‘ункци€ - обработчик сообщений диалога добавлени€/редактировани€ значени€ параметра реестра
INT_PTR CALLBACK AddEditMultistringValueDlgProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);			// ‘ункци€ - обработчик сообщений диалога редактировани€ многострочного параметра реестра


// √лобальные переменные, в них сохран€ютс€: тип добавл€емого параметра реестра (используетс€ в диалоге выбора типа параметра), им€ добавл€емого/редактируемого
// параметра (используетс€ в диалоге ввода имени добавл€емого/редактируемого параметра), размер и значение добавл€емого/редактируемого параметра
// (используетс€ в диалоге ввода значени€ параметра).
extern int param_type_edit;	// ¬ диалоге выбора типа используютс€ следующие значени€: 0=REG_BINARY, 1=REG_DWORD, 2=REG_QWORD, 3=REG_SZ, 4=REG_MULTI_SZ, 5=REG_EXPAND_SZ
extern string param_name_edit;
extern int param_value_size_edit;
extern vector<BYTE> param_value_edit;
