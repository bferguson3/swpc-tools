#include <iostream>
#include <string>
#include <cstdint>
#include <cstdio>

using namespace std;

// pipe SHIFTJIS.txt in and pipe to (binary) file out
int main()
{
    string s;
    uint8_t *mapping; //same bigendian array as in converting function
    mapping = new uint8_t[2*(256 + 3*256*16)];

    //initializing with space for invalid value, and then ASCII control chars
    for(size_t i = 32; i < 256 + 3*256*16; i++)
    {
        mapping[2 * i] = 0;
        mapping[2 * i + 1] = 0x20;
    }
    for(size_t i = 0; i < 32; i++)
    {
        mapping[2 * i] = 0;
        mapping[2 * i + 1] = i;
    }

    while(getline(cin, s)) //pipe the file SHIFTJIS to stdin
    {
        if(s.substr(0, 2) != "0x") continue; //comment lines

        uint16_t shiftJisValue, unicodeValue;
        if(2 != sscanf(s.c_str(), "%hx %hx", &shiftJisValue, &unicodeValue)) //getting hex values
        {
            puts("Error hex reading");
            continue;
        }

        size_t offset; //array offset
        if((shiftJisValue >> 8) == 0) offset = 0;
        else if((shiftJisValue >> 12) == 0x8) offset = 256;
        else if((shiftJisValue >> 12) == 0x9) offset = 256 + 16*256;
        else if((shiftJisValue >> 12) == 0xE) offset = 256 + 2*16*256;
        else
        {
            puts("Error input values");
            continue;
        }

        offset = 2 * (offset + (shiftJisValue & 0xfff));
        if(mapping[offset] != 0 || mapping[offset + 1] != 0x20)
        {
            puts("Error mapping not 1:1");
            continue;
        }

        mapping[offset] = unicodeValue >> 8;
        mapping[offset + 1] = unicodeValue & 0xff;
    }

    fwrite(mapping, 1, 2*(256 + 3*256*16), stdout);
    delete[] mapping;
    return 0;
}