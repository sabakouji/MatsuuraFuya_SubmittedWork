#pragma once
#include <string>
#include <vector>

#ifdef UNICODE
using tstring = std::wstring;
#else
using tstring = std::string;
#endif

class TextReader {
public:
    TextReader(tstring filename);
    ~TextReader();

    unsigned int GetLines();
    unsigned int GetColumns(unsigned int line);
    tstring GetString(unsigned int line, unsigned int column);
    int GetInt(unsigned int line, unsigned int column);
    float GetFloat(unsigned int line, unsigned int column);

private:
    struct LINEREC
    {
        std::vector<tstring> record;
    };
    std::vector<LINEREC> all;
};