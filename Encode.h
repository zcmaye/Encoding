#pragma once
#include<stdint.h>

/*Base64 Encode*/
const char* base64Encode(const char* data, size_t len);

/*Url Encode*/
//����
const char* urlEncode(const char* data,size_t len);

//����
const char* urlDecode(const char* data,size_t len);

/*@�ַ�ת��*/
int unicodeToUtf8(char* pInput, char* pOutput);
/*************************************************************************************************
* ��UTF8����ת����Unicode��UCS-2LE������  �͵�ַ���λ�ֽ�
* ������
*    char* pInput     �����ַ���
*    char*pOutput   ����ַ���
* ����ֵ��ת�����Unicode�ַ������ֽ�������������򷵻�-1
**************************************************************************************************/
int utf8ToUnicode(char* pInput, char* pOutput);
int unicodeToGB2312(char* pOut, char* uData);