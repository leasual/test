#pragma once

#include <string>
#include <cstdio>
#include <cstdlib>

namespace untouch {

    /**
     * Connected to a specified server and read key.
     * if key do not match , just call abort() to prohibit software using.
     */
    void Authenticate();

    /**
     * Decrypt a encrypted file
     * @param [in] fp
     * @return [out] file pointer is a temporary file.
     */
    FILE* Decrypt(FILE *fp);

    /**
     * Encrypt a FILE* using AES algorithm with a specified key, save new FILE to path.
     * @param [in] fp input FILE stream pointer.
     * @param [in] path new file dump path.
     * @return [out] true for success.
     */
    bool Encrypt(FILE* fp,std::string path);


    /**
     * For testing usage.
     * @param fp
     * @param path
     */
    void Decrypt(FILE *fp,std::string path);

    /**
     * For testing usage.
     * @param fp
     * @return
     */
    std::string ReadFile(FILE* fp);
}
