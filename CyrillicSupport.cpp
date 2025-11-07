#include "CyrillicSupport.h"
#include <vector>

std::wstring UTF8ToWide(const std::string& str) {
    if (str.empty()) return std::wstring();

    int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
    if (size_needed <= 0) return std::wstring();

    std::vector<wchar_t> buffer(size_needed);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, buffer.data(), size_needed);

    return std::wstring(buffer.data());
}

std::string WideToUTF8(const std::wstring& wstr) {
    if (wstr.empty()) return std::string();

    int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
    if (size_needed <= 0) return std::string();

    std::vector<char> buffer(size_needed);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, buffer.data(), size_needed, NULL, NULL);

    return std::string(buffer.data());
}

void ShowMessageBoxUTF8(HWND hWnd, const std::string& message, const std::string& title, UINT type) {
    MessageBoxW(hWnd, UTF8ToWide(message).c_str(), UTF8ToWide(title).c_str(), type);
}

void SetWindowTextUTF8(HWND hWnd, const std::string& text) {
    SetWindowTextW(hWnd, UTF8ToWide(text).c_str());
}

std::string GetWindowTextUTF8(HWND hWnd) {
    int len = GetWindowTextLengthW(hWnd);
    if (len == 0) return std::string();

    std::vector<wchar_t> buffer(len + 1);
    GetWindowTextW(hWnd, buffer.data(), len + 1);

    return WideToUTF8(std::wstring(buffer.data()));
}