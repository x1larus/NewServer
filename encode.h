#pragma once
#include <string>

//2 bytes to unicode string
std::wstring unicode_to_wstring(char str[], int size);

//1 byte string to unicode string
std::wstring ascii_to_wstring(std::string str);

//unicode string to 2 bytes
char* unicode_get_bytes(std::wstring str);

//unicode string (ONLY ASCII SYMBOLS!) to string
std::string unicode_to_ascii(std::wstring str);
