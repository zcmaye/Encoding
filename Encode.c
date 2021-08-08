#include"Encode.h"
#include<stdbool.h>
#include<malloc.h>

const char* base64Encode(const char* data, size_t len)
{
	static const char base64Char[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	if (data == NULL)
		return NULL;
	uint8_t const* srcStr = (uint8_t const*)data;

	//��24Bit(3���ֽ�)Ϊ��λ����,data�ܹ����ֳɶ��ٿ�
	uint32_t dataBlock = len / 3;
	//�жϻ���֮����û��ʣ����ֽ�(ʣ����ֽ���Ҫ���=)
	bool havePadding = len % 3;		//0 1
	size_t paddingByte = len % 3;
	//�������֮�����ֽ���,��ʣ��Ĳ���һ����λ��data�������ַ�����ĸ��ַ���
	uint32_t resultByte = 4 * (dataBlock + havePadding);

	//�����ڴ��ű���֮�������,�����������ֽ�ǰ�����������=�����һ���������\0
	char* encode = calloc(resultByte + 3, sizeof(char));
	if (encode == NULL)
		return	NULL;

	//��data�������ֽ�ת��Ϊbase64���ĸ��ַ�
	size_t i = 0;
	for (; i < dataBlock; ++i)
	{
		encode[4 * i + 0] = base64Char[srcStr[3 * i] >> 2];												//6
		encode[4 * i + 1] = base64Char[((srcStr[3 * i] & 0x3) << 4) | (srcStr[3 * i + 1] >> 4)];				//2 + 4
		encode[4 * i + 2] = base64Char[((srcStr[3 * i + 1] & 0xF) << 2) | (srcStr[3 * i + 2] >> 6)];		//4 + 2
		encode[4 * i + 3] = base64Char[srcStr[3 * i + 2] & 0x3F];								//6
	}

	//����ʣ�µ��ֽ�(ֻ����ʣ��0����1������2��)
	if (havePadding)
	{
		encode[4 * i + 0] = base64Char[srcStr[3 * i] >> 2];
		if (paddingByte == 1)		//����һ���ֽڲ������ֽ�
		{
			encode[4 * i + 1] = base64Char[(srcStr[3 * i] & 0x3) << 4];
			encode[4 * i + 2] = '=';
		}
		else if (paddingByte == 2)	//���������ֽڲ�һ���ֽ�
		{
			encode[4 * i + 1] = base64Char[((srcStr[3 * i] & 0x3) << 4) | srcStr[3 * i + 1] >> 4];
			encode[4 * i + 2] = base64Char[(srcStr[3 * i + 1] & 0xF) << 2];
		}
		encode[4 * i + 3] = '=';
	}
	//�����ڴ�ʱȫ����ʼ��Ϊ0�ˣ����ﲻ��Ҫ�ٲ�\0
	//encode[resultByte] = '\n';
	return encode;
}


//ʮ�������ַ�תʮ����
static int hexToDec(char hex)
{
	if ('0' <= hex && hex <= '9')
	{
		return hex - '0';
	}
	else if ('a' <= hex && hex <= 'f')
	{
		return hex - 'a' + 10;
	}
	else if ('A' <= hex && hex <= 'F')
	{
		return hex - 'A' + 10;
	}
	return -1;
}

//ʮ����תʮ�������ַ�
static int decToHex(uint8_t dec)
{
	if (0 <= dec && dec <= 9)
	{
		return dec + '0';
	}
	else if (10 <= dec && dec <= 15)
	{
		return dec + 'A' - 10;
	}
	return -1;
}

//����ַ��Ƿ���Ҫ����
static bool needCoding(char c)
{
	if (
		('0' <= c && c <= '9') ||
		('a' <= c && c <= 'z') ||
		('A' <= c && c <= 'Z') ||
		(c == '/' || c == '.'  || c == '_')
			)
		{
			return false;
		}
	return true;
}

const char* urlEncode(const char* data, size_t len)
{
	static char urlCode[128] = { 0 };
	size_t url_i = 0;
	for (size_t i = 0; i < len; ++i)
	{
		char c = data[i];
		//����Ҫ����ֱ�Ӵ洢
		if (!needCoding(c))
		{
			urlCode[url_i++] = c;
		}
		else
		{
			int j = (int)c;
			if (j < 0)
			{
				j += 256;
			}
			int b1 = j / 16;
			int b0 = j - b1 * 16;
			urlCode[url_i++] = '%';
			urlCode[url_i++] = decToHex(b1);
			urlCode[url_i++] = decToHex(b0);
		}
	}
	return urlCode;
}

const char* urlDecode(const char* data, size_t len)
{
	static char urlCode[128] = {0};
	size_t url_i = 0;
	for (size_t i = 0; i < len; ++i)
	{
		char c = data[i];
		if (c != '%')
		{
			urlCode[url_i++] = c;
		}
		else
		{
			char c1 = data[++i];
			char c0 = data[++i];
			int num = hexToDec(c1) * 16 + hexToDec(c0);
			urlCode[url_i++] = num;
		}
	}
	return urlCode;
}

