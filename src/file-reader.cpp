#include "file-reader.h"

#include <vector>
#include <string>

std::vector<char> read_hdl_file_contents(const std::string& filename) {

    std::vector<char> v;

    FILE* fptr = fopen(filename.c_str(), "rb");

    const int SRC_BUF_SIZE = 512;
    char src_buf[SRC_BUF_SIZE];

    const int state_default         = 0;
    const int state_first_fwd_slash = 1;
    const int state_comment         = 2;

    int state_current = state_default;

    int rd_sz = fread(src_buf, 1, SRC_BUF_SIZE, fptr);
    while(rd_sz > 0) {

        for(int i = 0; i < rd_sz; i++) {
            const char c = src_buf[i];

            switch(state_current) {
            case state_default:
                if(c == '/') {
                    state_current = state_first_fwd_slash;
                }
                else {
                    if(c != '\r')
                        v.push_back(c);
                }
                break;
            case state_first_fwd_slash:
                if(c == '/') {
                    state_current = state_comment;
                }
                else {
                    v.push_back('/');
                    if(c != '\r')
                        v.push_back(c);
                    state_current = state_default;
                }
                break;
            case state_comment:
                if(c == '\n') {
                    v.push_back('\n');
                    state_current = state_default;
                }
                break;
            }
        }

        rd_sz = fread(src_buf, 1, SRC_BUF_SIZE, fptr);
    }

    fclose(fptr);

    v.push_back('\n');
    return v;
}

