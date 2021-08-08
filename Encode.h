#pragma once
#include<stdint.h>

/*Base64 Encode*/
const char* base64Encode(const char* data, size_t len);

/*Url Encode*/
//编码
const char* urlEncode(const char* data,size_t len);

//解码
const char* urlDecode(const char* data,size_t len);

/*@字符转码*/
int unicodeToUtf8(char* pInput, char* pOutput);
/*************************************************************************************************
* 将UTF8编码转换成Unicode（UCS-2LE）编码  低地址存低位字节
* 参数：
*    char* pInput     输入字符串
*    char*pOutput   输出字符串
* 返回值：转换后的Unicode字符串的字节数，如果出错则返回-1
**************************************************************************************************/
int utf8ToUnicode(char* pInput, char* pOutput);
int unicodeToGB2312(char* pOut, char* uData);