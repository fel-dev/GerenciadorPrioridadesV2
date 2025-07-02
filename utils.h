#pragma once
#include <string>
#include <cwchar>

// Remove espaços em branco do final de uma std::wstring
inline void trim_right(std::wstring& s) {
    s.erase(s.find_last_not_of(L" \t\n\r") + 1);
}

// Compara duas strings wide (case-insensitive)
inline bool equals_ignore_case(const std::wstring& a, const std::wstring& b) {
    return _wcsicmp(a.c_str(), b.c_str()) == 0;
}

// Concatena valor numérico e sufixo (ex: "123 KB")
inline std::wstring to_wstring_with_suffix(size_t value, const std::wstring& suffix) {
    return std::to_wstring(value) + suffix;
}
