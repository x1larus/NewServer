#include "encode.h"

std::wstring unicode_to_wstring(char str[], int size) 
{
    std::wstring result;
    for (int i = 0; i < size; i += 2)
    {
        wchar_t a = str[i] + (str[i+1] * 256);
        result.push_back(a);
    }
    return result;
}

std::wstring ascii_to_wstring(std::string str) 
{
    std::wstring result;
    for (unsigned int i = 0; i < str.size(); i++)
    {
        result.push_back(str[i]);
    }
    return result;
}

char* unicode_get_bytes(std::wstring str)
{
    char *result = new char[str.size()*2 + 1];
    for (u_int32_t i = 0; i < str.size(); i++)
    {
        result[i*2+1] = str[i] / 256;
        result[i*2] = str[i] - (result[i*2+1] * 256);
    }
    return result;
}

std::string unicode_to_ascii(std::wstring str)
{
    std::string result;
    for (u_int32_t i = 0; i < str.size(); i++)
    {
        char a = str[i] % 256;
        result.push_back(a);
    }
    return result;   
}