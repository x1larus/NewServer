#pragma once
#include <string>

//encode 2-char to wstring
std::wstring to_wstring(char str[], int size) 
{
    std::wstring result;
    for (int i = 0; i < size; i += 2)
    {
        wchar_t a = str[i] + (str[i+1] * 256);
        result.push_back(a);
    }
    return result;
}

//encode 1-char to wstring
std::wstring to_wstring(std::string str) 
{
    std::wstring result;
    for (int i = 0; i < str.size(); i++)
    {
        result.push_back(str[i]);
        result.push_back('\0');
    }
    return result;
}


//return 2-char array
char* to_char(std::wstring str)
{
    char *result = new char[str.size()*2 + 1];
    for (u_int32_t i = 0; i < str.size(); i++)
    {
        result[i*2+1] = str[i] / 256;
        result[i*2] = str[i] - (result[i*2+1] * 256);
    }
    return result;
}

//return 1-char array
std::string to_string(std::wstring str)
{
    std::string result;
    for (u_int32_t i = 0; i < str.size(); i += 2)
    {
        char a = str[i] % 256;
        result.push_back(a);
    }
    return result;   
}