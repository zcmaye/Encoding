#include"Encode.h"
#include<stdbool.h>
#include<malloc.h>

const char* base64Encode(const char* data, size_t len)
{
	static const char base64Char[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	if (data == NULL)
		return NULL;
	uint8_t const* srcStr = (uint8_t const*)data;

	//以24Bit(3个字节)为单位划分,data能够划分成多少块
	uint32_t dataBlock = len / 3;
	//判断划分之后有没有剩余的字节(剩余的字节需要填充=)
	bool havePadding = len % 3;		//0 1
	size_t paddingByte = len % 3;
	//计算编码之后总字节数,有剩余的补上一个单位（data中三个字符变成四个字符）
	uint32_t resultByte = 4 * (dataBlock + havePadding);

	//申请内存存放编码之后的数据,多申请三个字节前两个用来填充=，最后一个用来存放\0
	char* encode = calloc(resultByte + 3, sizeof(char));
	if (encode == NULL)
		return	NULL;

	//把data的三个字节转化为base64的四个字符
	size_t i = 0;
	for (; i < dataBlock; ++i)
	{
		encode[4 * i + 0] = base64Char[srcStr[3 * i] >> 2];												//6
		encode[4 * i + 1] = base64Char[((srcStr[3 * i] & 0x3) << 4) | (srcStr[3 * i + 1] >> 4)];				//2 + 4
		encode[4 * i + 2] = base64Char[((srcStr[3 * i + 1] & 0xF) << 2) | (srcStr[3 * i + 2] >> 6)];		//4 + 2
		encode[4 * i + 3] = base64Char[srcStr[3 * i + 2] & 0x3F];								//6
	}

	//处理剩下的字节(只可能剩下0个，1个或者2个)
	if (havePadding)
	{
		encode[4 * i + 0] = base64Char[srcStr[3 * i] >> 2];
		if (paddingByte == 1)		//留下一个字节补两个字节
		{
			encode[4 * i + 1] = base64Char[(srcStr[3 * i] & 0x3) << 4];
			encode[4 * i + 2] = '=';
		}
		else if (paddingByte == 2)	//留下两个字节补一个字节
		{
			encode[4 * i + 1] = base64Char[((srcStr[3 * i] & 0x3) << 4) | srcStr[3 * i + 1] >> 4];
			encode[4 * i + 2] = base64Char[(srcStr[3 * i + 1] & 0xF) << 2];
		}
		encode[4 * i + 3] = '=';
	}
	//申请内存时全部初始化为0了，这里不需要再补\0
	//encode[resultByte] = '\n';
	return encode;
}


//十六进制字符转十进制
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

//十进制转十六进制字符
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

//检测字符是否需要编码
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
		//不需要编码直接存储
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

