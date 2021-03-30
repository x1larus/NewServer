#pragma once
#include <string>

std::wstring to_wstring(char str[], int size);
std::wstring to_wstring(std::string str);
char* to_char(std::wstring str);
std::string to_string(std::wstring str);