#include "encrypting.h"
//#include "zmq.h"
#include <cassert>
#include <cstring>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <sstream>

/*
#include <crypto++/modes.h>
#include <crypto++/aes.h>
#include <crypto++/filters.h>
*/
#include "des_encrypting.h"

#if defined(ANDROID) || defined(__ANDROID__)
#include <android/log.h>

#define LOG_TAG "NCNN_NATIVE_LIB"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)

#endif

namespace untouch {

    std::string GetCodeFromServer() {

        std::string result;
        /*
        void* context,*pull;

        context = zmq_ctx_new();
        pull = zmq_socket(context,ZMQ_PULL);


        int ret = zmq_connect(pull,"tcp://192.168.1.124:10539");
//        int ret = zmq_connect(pull,"tcp://localhost:10539");
        assert(ret == 0);


        for(int i=5;i>0;i--) {
            char data[20] = {'\0'};
            ret = zmq_recv(pull,data,sizeof(char)*20,ZMQ_DONTWAIT);
            result = std::string(data);
            if (result != "") break;
            usleep(5*1000);
        }

        zmq_close(pull);
        zmq_ctx_destroy(context);
*/
        return result;
    }

    void Authenticate() {
        // connect master2 get password , if do not match , just break down.
        const std::string key = "zhanhekakuii";
        /*
        std::string fetched_key = GetCodeFromServer();
        if(key != fetched_key)
            abort();
            */
    }


    std::string ReadFile(FILE* fp) {
        // read all data from fp into an string array.
        fseek(fp,0,SEEK_END);
        long length_of_file = ftell(fp);
        fseek(fp,0,SEEK_SET);

        std::stringstream ss;
        size_t buffer_size = 2048;
        char buffer[2048] = {'\0'};
        size_t avaliable_size = 1; // just a init value.
        while(avaliable_size != 0) {
            avaliable_size = fread(buffer,sizeof(char),buffer_size,fp);
            ss << std::string(buffer,avaliable_size);
            memset(buffer,'\0',buffer_size);
        }
        fseek(fp,0,SEEK_SET);
        return ss.str();
    }

/*
    FILE* Decrypt_AES(FILE *fp) {
        // read file content from fp and decrypt it according key.
        byte key[32],iv[CryptoPP::AES::BLOCKSIZE];
        // key and iv
        char s_key[] = "493b4c5d8d3d7326b2e9fdc882fdbdfd";
        char s_iv[] = "39216b956173193a";
        memcpy(key,s_key,sizeof(char)*32);
        memcpy(iv,s_iv,sizeof(char)*CryptoPP::AES::BLOCKSIZE);

        std::string ciphertext = ReadFile(fp);
        std::string decryptedtext;
        CryptoPP::AES::Decryption aesDecryption(key, CryptoPP::AES::DEFAULT_KEYLENGTH);
        CryptoPP::CBC_Mode_ExternalCipher::Decryption cbcDecryption( aesDecryption, iv );

        CryptoPP::StreamTransformationFilter stfDecryptor(cbcDecryption, new CryptoPP::StringSink( decryptedtext ) );
        stfDecryptor.Put( reinterpret_cast<const unsigned char*>( ciphertext.c_str() ), ciphertext.size() );
        stfDecryptor.MessageEnd();


        FILE* tmp_file = tmpfile();
        fwrite(decryptedtext.c_str(),sizeof(char),decryptedtext.length(),tmp_file);
        fflush(tmp_file);
        fseek(tmp_file,0,SEEK_SET);
        return tmp_file;
    }
*/
/*
    void Decrypt_AES(FILE *fp,std::string path) {
        // read file content from fp and decrypt it according key.
        byte key[32],iv[CryptoPP::AES::BLOCKSIZE];
        // key and iv
        char s_key[] = "493b4c5d8d3d7326b2e9fdc882fdbdfd";
        char s_iv[] = "39216b956173193a";
        memcpy(key,s_key,sizeof(char)*32);
        memcpy(iv,s_iv,sizeof(char)*CryptoPP::AES::BLOCKSIZE);

        std::string ciphertext = ReadFile(fp);
        std::string decryptedtext;

        CryptoPP::AES::Decryption aesDecryption(key, CryptoPP::AES::DEFAULT_KEYLENGTH);
        CryptoPP::CBC_Mode_ExternalCipher::Decryption cbcDecryption( aesDecryption, iv );
        CryptoPP::StreamTransformationFilter stfDecryptor(cbcDecryption, new CryptoPP::StringSink( decryptedtext ) );

        stfDecryptor.Put( reinterpret_cast<const unsigned char*>( ciphertext.c_str() ), ciphertext.size() );
        stfDecryptor.MessageEnd();

//        std::cout << "decryptedtext done" << std::endl;
//        std::cout << ciphertext << std::endl;

        FILE* tmp_file = fopen(path.c_str(),"wb");
        fwrite(decryptedtext.c_str(),sizeof(char),decryptedtext.length(),tmp_file);
        fclose(tmp_file);
    }
*/
/*
    bool Encrypt(FILE* fp,std::string path) {
        //1.read all data from fp
        //2. encrypt it.
        //3. create a file and save it to path.
        std::string file_content = ReadFile(fp);

        // read file content from fp and decrypt it according key.
        byte key[32],iv[CryptoPP::AES::BLOCKSIZE];
        // key and iv
        char s_key[] = "493b4c5d8d3d7326b2e9fdc882fdbdfd";
        char s_iv[] = "39216b956173193a";
        memcpy(key,s_key,sizeof(char)*32);
        memcpy(iv,s_iv,sizeof(char)*CryptoPP::AES::BLOCKSIZE);

        std::string ciphertext;
        CryptoPP::AES::Encryption aesEncryption(key, CryptoPP::AES::DEFAULT_KEYLENGTH);
        CryptoPP::CBC_Mode_ExternalCipher::Encryption cbcEncryption( aesEncryption, iv );

        CryptoPP::StreamTransformationFilter stfEncryptor(cbcEncryption, new CryptoPP::StringSink( ciphertext ) );
        stfEncryptor.Put( reinterpret_cast<const unsigned char*>( file_content.c_str() ), file_content.length() );
        stfEncryptor.MessageEnd();

        std::ofstream os(path.c_str(),std::ios::out | std::ios::binary);
        os << ciphertext;
        os.close();

        return true;
    }
*/
    void gen_random(char *s, const int len) {
        static const char alphanum[] =
                "0123456789"
                "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                "abcdefghijklmnopqrstuvwxyz";

        for (int i = 0; i < len; ++i) {
            s[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
        }

        s[len] = 0;
    }

    FILE* Decrypt(FILE *fp) {
        using namespace std;
        std::string key = std::string("ruj12Zol");
        InitFromKey(key);

        stringstream ss;
        std::string decryptedtext;

        std::string cipher = ReadFile(fp);



        size_t curr_index = 0;
        for(;curr_index < cipher.length();curr_index+=8) {
            size_t endpos = curr_index + 8;
            if (endpos > cipher.length()) endpos = cipher.length();
            std::string input_chars = cipher.substr(curr_index,endpos);
            if(input_chars.length() != 8) {
                std::string temp = std::string(8,'\0');
                temp.insert(0,input_chars,0,input_chars.length());
                input_chars = temp;
            }
            bitset<64> cipher = charToBitset(input_chars.c_str());
            bitset<64> plain = decrypt(cipher);
            ss.write((char*)(&plain),sizeof(plain));
        }

        decryptedtext = ss.str();

#if defined(__ANDROID__) || defined(ANDROID)
        char random_name[15] = {'\0'};
        std::string root = "/sdcard/Android/data/com.ut.sdk/files/";
        gen_random(random_name,15);
        std::string temp_path = root+std::string(random_name);
        FILE* tmp_file = fopen(temp_path.c_str(),"wb");

        LOGI("read file size : %ld , dump name %s ",cipher.length(),temp_path.c_str());
#else
        FILE* tmp_file = tmpfile();
#endif
        fwrite(decryptedtext.c_str(),sizeof(char),decryptedtext.length(),tmp_file);
        fflush(tmp_file);
        fseek(tmp_file,0,SEEK_SET);
        return tmp_file;
    }


    void Decrypt(FILE *fp,std::string path){
        using namespace std;
        std::string key = std::string("ruj12Zol");
        InitFromKey(key);

        stringstream ss;
        std::string decryptedtext;

        std::string cipher = ReadFile(fp);

        size_t curr_index = 0;
        for(;curr_index < cipher.length();curr_index+=8) {
            size_t endpos = curr_index + 8;
            if (endpos > cipher.length()) endpos = cipher.length();
            std::string input_chars = cipher.substr(curr_index,endpos);
            if(input_chars.length() != 8) {
                std::string temp = std::string(8,'\0');
                temp.insert(0,input_chars,0,input_chars.length());
                input_chars = temp;
            }
            bitset<64> cipher = charToBitset(input_chars.c_str());
            bitset<64> plain = decrypt(cipher);
            ss.write((char*)(&plain),sizeof(plain));
        }

        decryptedtext = ss.str();

        std::cout << decryptedtext << std::endl;

        std::ofstream os(path.c_str(),std::ios::out | std::ios::binary);
        os << decryptedtext;
        os.close();
    }



    bool Encrypt(FILE *fp,std::string path) {
        using namespace std;

        std::string key = std::string("ruj12Zol");
        InitFromKey(key);

        std::string file_content = ReadFile(fp);

        std::ofstream os(path.c_str(),std::ios::out | std::ios::binary);

        size_t curr_index = 0;
        for(;curr_index < file_content.length();curr_index+=8) {
            size_t endpos = curr_index + 8;
            if (endpos > file_content.length()) endpos = file_content.length();
            std::string input_chars = file_content.substr(curr_index,endpos);
            if(input_chars.length() != 8) {
                std::string temp = std::string(8,'\0');
                temp.insert(0,input_chars,0,input_chars.length());
                input_chars = temp;
            }
            bitset<64> plain = charToBitset(input_chars.c_str());
            bitset<64> cipher = encrypt(plain);
            os.write((char*)(&cipher),sizeof(cipher));
        }
        os.close();
        return true;
    }



}