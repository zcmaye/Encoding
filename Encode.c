#include"Encode.h"
#include<stdbool.h>
#include<malloc.h>

/*@base64*/
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


/*@Url编码*/
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
		(c == '/' || c == '.'  || c == '_' || c=='*')
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


/*@字符转码*/
int unicodeToUtf8(char* pInput, char* pOutput)
{
	int len = 0; //记录转换后的Utf8字符串的字节数
	while (*pInput)
	{
		//处理一个unicode字符
		char low = *pInput;//取出unicode字符的低8位
		pInput++;
		char high = *pInput;//取出unicode字符的高8位
		int w = high << 8;
		unsigned  wchar = (high << 8) + low;//高8位和低8位组成一个unicode字符,加法运算级别高

		if (wchar <= 0x7F) //英文字符
		{
			pOutput[len] = (char)wchar;  //取wchar的低8位
			len++;
		}
		else if (wchar >= 0x80 && wchar <= 0x7FF)  //可以转换成双字节pOutput字符
		{
			pOutput[len] = 0xc0 | ((wchar >> 6) & 0x1f);  //取出unicode编码低6位后的5位，填充到110yyyyy 10zzzzzz 的yyyyy中
			len++;
			pOutput[len] = 0x80 | (wchar & 0x3f);  //取出unicode编码的低6位，填充到110yyyyy 10zzzzzz 的zzzzzz中
			len++;
		}
		else if (wchar >= 0x800 && wchar < 0xFFFF)  //可以转换成3个字节的pOutput字符
		{
			pOutput[len] = 0xe0 | ((wchar >> 12) & 0x0f);  //高四位填入1110xxxx 10yyyyyy 10zzzzzz中的xxxx
			len++;
			pOutput[len] = 0x80 | ((wchar >> 6) & 0x3f);  //中间6位填入1110xxxx 10yyyyyy 10zzzzzz中的yyyyyy
			len++;
			pOutput[len] = 0x80 | (wchar & 0x3f);  //低6位填入1110xxxx 10yyyyyy 10zzzzzz中的zzzzzz
			len++;
		}

		else //对于其他字节数的unicode字符不进行处理
		{
			return -1;
		}
		pInput++;//处理下一个unicode字符
	}
	//utf8字符串后面，有个\0
	pOutput[len] = 0;
	return len;
}
/*************************************************************************************************
* 将UTF8编码转换成Unicode（UCS-2LE）编码  低地址存低位字节
* 参数：
*    char* pInput     输入字符串
*    char*pOutput   输出字符串
* 返回值：转换后的Unicode字符串的字节数，如果出错则返回-1
**************************************************************************************************/
int utf8ToUnicode(char* pInput, char* pOutput)
{
	int outputSize = 0; //记录转换后的Unicode字符串的字节数

	while (*pInput)
	{
		if (*pInput > 0x00 && *pInput <= 0x7F) //处理单字节UTF8字符（英文字母、数字）
		{
			*pOutput = *pInput;
			pOutput++;
			*pOutput = 0; //小端法表示，在高地址填补0
		}
		else if (((*pInput) & 0xE0) == 0xC0) //处理双字节UTF8字符
		{
			char high = *pInput;
			pInput++;
			char low = *pInput;
			if ((low & 0xC0) != 0x80)  //检查是否为合法的UTF8字符表示
			{
				return -1; //如果不是则报错
			}

			*pOutput = (high << 6) + (low & 0x3F);
			pOutput++;
			*pOutput = (high >> 2) & 0x07;
		}
		else if (((*pInput) & 0xF0) == 0xE0) //处理三字节UTF8字符
		{
			char high = *pInput;
			pInput++;
			char middle = *pInput;
			pInput++;
			char low = *pInput;
			if (((middle & 0xC0) != 0x80) || ((low & 0xC0) != 0x80))
			{
				return -1;
			}
			*pOutput = (middle << 6) + (low & 0x3F);//取出middle的低两位与low的低6位，组合成unicode字符的低8位
			pOutput++;
			*pOutput = (high << 4) + ((middle >> 2) & 0x0F); //取出high的低四位与middle的中间四位，组合成unicode字符的高8位
		}
		else //对于其他字节数的UTF8字符不进行处理
		{
			return -1;
		}
		pInput++;//处理下一个utf8字符
		pOutput++;
		outputSize += 2;
	}
	//unicode字符串后面，有两个\0
	*pOutput = 0;
	pOutput++;
	*pOutput = 0;
	return outputSize;
}

int unicodeToGB2312(char* pOut, char* uData)
{

	return 0;
}
