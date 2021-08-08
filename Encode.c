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


/*@Url����*/
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


/*@�ַ�ת��*/
int unicodeToUtf8(char* pInput, char* pOutput)
{
	int len = 0; //��¼ת�����Utf8�ַ������ֽ���
	while (*pInput)
	{
		//����һ��unicode�ַ�
		char low = *pInput;//ȡ��unicode�ַ��ĵ�8λ
		pInput++;
		char high = *pInput;//ȡ��unicode�ַ��ĸ�8λ
		int w = high << 8;
		unsigned  wchar = (high << 8) + low;//��8λ�͵�8λ���һ��unicode�ַ�,�ӷ����㼶���

		if (wchar <= 0x7F) //Ӣ���ַ�
		{
			pOutput[len] = (char)wchar;  //ȡwchar�ĵ�8λ
			len++;
		}
		else if (wchar >= 0x80 && wchar <= 0x7FF)  //����ת����˫�ֽ�pOutput�ַ�
		{
			pOutput[len] = 0xc0 | ((wchar >> 6) & 0x1f);  //ȡ��unicode�����6λ���5λ����䵽110yyyyy 10zzzzzz ��yyyyy��
			len++;
			pOutput[len] = 0x80 | (wchar & 0x3f);  //ȡ��unicode����ĵ�6λ����䵽110yyyyy 10zzzzzz ��zzzzzz��
			len++;
		}
		else if (wchar >= 0x800 && wchar < 0xFFFF)  //����ת����3���ֽڵ�pOutput�ַ�
		{
			pOutput[len] = 0xe0 | ((wchar >> 12) & 0x0f);  //����λ����1110xxxx 10yyyyyy 10zzzzzz�е�xxxx
			len++;
			pOutput[len] = 0x80 | ((wchar >> 6) & 0x3f);  //�м�6λ����1110xxxx 10yyyyyy 10zzzzzz�е�yyyyyy
			len++;
			pOutput[len] = 0x80 | (wchar & 0x3f);  //��6λ����1110xxxx 10yyyyyy 10zzzzzz�е�zzzzzz
			len++;
		}

		else //���������ֽ�����unicode�ַ������д���
		{
			return -1;
		}
		pInput++;//������һ��unicode�ַ�
	}
	//utf8�ַ������棬�и�\0
	pOutput[len] = 0;
	return len;
}
/*************************************************************************************************
* ��UTF8����ת����Unicode��UCS-2LE������  �͵�ַ���λ�ֽ�
* ������
*    char* pInput     �����ַ���
*    char*pOutput   ����ַ���
* ����ֵ��ת�����Unicode�ַ������ֽ�������������򷵻�-1
**************************************************************************************************/
int utf8ToUnicode(char* pInput, char* pOutput)
{
	int outputSize = 0; //��¼ת�����Unicode�ַ������ֽ���

	while (*pInput)
	{
		if (*pInput > 0x00 && *pInput <= 0x7F) //�����ֽ�UTF8�ַ���Ӣ����ĸ�����֣�
		{
			*pOutput = *pInput;
			pOutput++;
			*pOutput = 0; //С�˷���ʾ���ڸߵ�ַ�0
		}
		else if (((*pInput) & 0xE0) == 0xC0) //����˫�ֽ�UTF8�ַ�
		{
			char high = *pInput;
			pInput++;
			char low = *pInput;
			if ((low & 0xC0) != 0x80)  //����Ƿ�Ϊ�Ϸ���UTF8�ַ���ʾ
			{
				return -1; //��������򱨴�
			}

			*pOutput = (high << 6) + (low & 0x3F);
			pOutput++;
			*pOutput = (high >> 2) & 0x07;
		}
		else if (((*pInput) & 0xF0) == 0xE0) //�������ֽ�UTF8�ַ�
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
			*pOutput = (middle << 6) + (low & 0x3F);//ȡ��middle�ĵ���λ��low�ĵ�6λ����ϳ�unicode�ַ��ĵ�8λ
			pOutput++;
			*pOutput = (high << 4) + ((middle >> 2) & 0x0F); //ȡ��high�ĵ���λ��middle���м���λ����ϳ�unicode�ַ��ĸ�8λ
		}
		else //���������ֽ�����UTF8�ַ������д���
		{
			return -1;
		}
		pInput++;//������һ��utf8�ַ�
		pOutput++;
		outputSize += 2;
	}
	//unicode�ַ������棬������\0
	*pOutput = 0;
	pOutput++;
	*pOutput = 0;
	return outputSize;
}

int unicodeToGB2312(char* pOut, char* uData)
{

	return 0;
}
