#pragma once
#include <string>
#include <Windows.h>

// Преобразование UTF-8 строки в UTF-16 (широкую)
std::wstring UTF8ToWide(const std::string& str);

// Преобразование широкой строки (UTF-16) в UTF-8
std::string WideToUTF8(const std::wstring& wstr);

// Диалоговые окна с поддержкой UTF-8
void ShowMessageBoxUTF8(HWND hWnd, const std::string& message, const std::string& title, UINT type);

// Установка текста в элемент управления
void SetWindowTextUTF8(HWND hWnd, const std::string& text);

// Получение текста из элемента управления
std::string GetWindowTextUTF8(HWND hWnd);