#pragma once
#include<stdint.h>

/*Base64 Encode*/
const char* base64Encode(const char* data, size_t len);

/*Url Encode*/
//±àÂë
const char* urlEncode(const char* data,size_t len);

//½âÂë
const char* urlDecode(const char* data,size_t len);