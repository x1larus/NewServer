#pragma once
#include <string>

std::wstring unicode_to_wstring(char str[], int size);
std::wstring ascii_to_wstring(std::string str);
char* unicode_get_bytes(std::wstring str);
std::string unicode_to_ascii(std::wstring str);
