#pragma once
#include<stdint.h>

/*Base64 Encode*/
const char* base64Encode(const char* data, size_t len);

/*Url Encode*/
//����
const char* urlEncode(const char* data,size_t len);

//����
const char* urlDecode(const char* data,size_t len);