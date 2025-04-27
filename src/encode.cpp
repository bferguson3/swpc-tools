#include <iostream>
#include <string>
#include <vector>
#include <iconv.h>

std::string utf8_to_sjis(const std::string& utf8_string) {
    std::string sjis_string;
    iconv_t conversion = iconv_open("SHIFT_JIS", "UTF-8");
    if (conversion == (iconv_t)-1) {
        std::cerr << "iconv_open failed!" << std::endl;
        return "";
    }

    size_t inbytesleft = utf8_string.size();
    size_t outbytesleft = 2 * inbytesleft + 1; // Maximum possible size
    std::vector<char> buffer(outbytesleft, 0);
    char* in_ptr = const_cast<char*>(utf8_string.c_str());
    char* out_ptr = buffer.data();

    size_t result = iconv(conversion, &in_ptr, &inbytesleft, &out_ptr, &outbytesleft);
    if (result == (size_t)-1) {
        std::cerr << "iconv failed!" << std::endl;
        iconv_close(conversion);
        return "";
    }

    sjis_string.assign(buffer.data(), out_ptr - buffer.data());
    iconv_close(conversion);
    return sjis_string;
}

int main() {
    std::string utf8_text = "こんにちは世界";
    std::string sjis_text = utf8_to_sjis(utf8_text);

    if (!sjis_text.empty()) {
      std::cout << "UTF-8: " << utf8_text << std::endl;
      std::cout << "SJIS: ";
      for (char c : sjis_text) {
          printf("\\x%02X", static_cast<unsigned char>(c));
      }
      std::cout << std::endl;
    }
    return 0;
}