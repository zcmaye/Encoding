#include <iostream>
#include "unicodeTables.h"
#include "gbkTables.h"
#include<uchar.h>
using namespace std;

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
		(c == '/' || c == '.' || c == '_' || c == '*')
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
	static char urlCode[128] = { 0 };
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
int unicodeToUtf8(const char* pInput, char* pOutput)
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
int utf8ToUnicode(const char* pInput, char* pOutput)
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

//gbk->unicode
int gbkToUnicode(const char *gbk_buf, unsigned short *unicode_buf, int max_unicode_buf_size)
{
    unsigned short word;
    unsigned char *gbk_ptr =(unsigned char *) gbk_buf;
    unsigned short *uni_ptr = unicode_buf;
	unsigned int uni_ind = 0, gbk_ind = 0, uni_num = 0;
	unsigned char ch;
    int word_pos;

    if( !gbk_buf || !unicode_buf )
        return -1;

    while(1)
    {
    	ch = *(gbk_ptr + gbk_ind);

		if(ch == 0x00)
			break;
		
        if( ch > 0x80 )
        {
			//将gbk编码的中文字符的两个字节组合成一个    unsigned short word;
				word = *(gbk_ptr + gbk_ind);
				word <<= 8;
				word += *(gbk_ptr + gbk_ind+1);
				gbk_ind += 2;

            word_pos = word - gbk_first_code;
            if(word >= gbk_first_code && word <= gbk_last_code  && (word_pos < unicode_buf_size))
            {
				*(uni_ptr + uni_ind) = unicodeTables[word_pos];
				uni_ind++;
				uni_num++;
            }
        }
		else
		{
			gbk_ind++;
			*(uni_ptr + uni_ind) = ch;
			uni_ind++;
			uni_num++;
        }
        
        if(uni_num > max_unicode_buf_size - 1)
			break;
    }

    return uni_num;
}

//unicode->gbk
int unicodeToGbk(unsigned short *unicode_buf, char *gbk_buf, int max_gbk_buf_size)
{
	unsigned short word;
	unsigned short gbk_word;
	unsigned char ch;
    unsigned char *gbk_ptr =(unsigned char *) gbk_buf;
    unsigned short *uni_ptr = unicode_buf;
	unsigned int uni_ind = 0, gbk_ind = 0, gbk_num = 0;
    int word_pos;

    if( !gbk_buf || !unicode_buf )
        return -1;

	while(1)
	{
		word = *(uni_ptr + uni_ind);
		uni_ind++;

		if(word == 0x0000)  //字符串结束符
			break;
		
		if(word < 0x80)  /*ASCII不用查表*/
		{
			*(gbk_ptr + gbk_ind) = (unsigned char)word;
			gbk_ind++;
		}
		else
		{
			word_pos = word - unicode_first_code;
			if(word >= unicode_first_code && word <= unicode_last_code && word_pos < gbk_buf_size)
			{
				gbk_word = gbkTables[word_pos];//gbk_word是gbk编码，但是为unsigned short类型，需要拆分成两个字节

				*(gbk_ptr + gbk_ind) = (unsigned char)(gbk_word >> 8);//提取高8位
				gbk_ind++;
				*(gbk_ptr + gbk_ind) = (unsigned char)(gbk_word >> 0);//提取低8位
				gbk_ind++;
				gbk_num +=2;//gbk字符加2个
			}
		}

		if(gbk_num > max_gbk_buf_size - 1)
			break;
	}

    return gbk_num;
}
size_t gbkCnt(const char* buf, size_t len);

//gbk->utf-8
size_t getGbkToUtf8Len(const char* gbk,size_t len)
{
	size_t gbkNum = gbkCnt(gbk, len);					//gbk字符个数
	size_t utf8Num = gbkNum * 3 + (len - gbkNum * 2);	//把gbk转成utf8之后需要的字节数
	return utf8Num + 1;	//要把\0也算上
}

int gbkToUtf8(const char* gbk, char* utf8,size_t len)
{
	int resLen =  getGbkToUtf8Len(gbk, strlen(gbk));
	if (len < getGbkToUtf8Len(gbk, strlen(gbk)))
	{
		return -1;	//utf8的大小不够，转换之后存储不下
	}
	unsigned short* unicode = (unsigned short*)calloc(resLen, sizeof(unsigned short));
	gbkToUnicode(gbk, unicode, 50);
	int size = unicodeToUtf8((const char*)unicode, utf8);
	free(unicode);
	return size;
}

//utf8->gbk
int utf8ToGbk(const char* utf8, char* gbk,size_t len)
{
	unsigned short* unicode = (unsigned short*)calloc(len, sizeof(unsigned short));
	utf8ToUnicode(utf8, (char*)unicode);
	int size = unicodeToGbk(unicode, gbk, 50);
	free(unicode);
	return size;
}

//查看数据的二进制
void binary(const char* buf, size_t len)
{
	for (size_t cnt = 0; cnt < len; cnt++)
	{
		for (int i = 7; i >= 0; i--)
		{
			if (i == 3)
				printf("'");
			if ((buf[cnt] >> i) & 0x1)
			{
				printf("1");
			}
			else
			{
				printf("0");
			}
		}
		printf(" ");
	}
	printf("\n");
}

//检测字符串是不是utf8编码
bool isUtf8(const char *buf)
{
	//假设为utf8,统计高字节从左到右连续为1的数量(即几个字节存储中文)
	int cnt = 0;	
	for (int i = 7; i >=0; i--)
	{
		if ((buf[0] >> i) & 0x1)
		{
			cnt++;
		}
		else
		{
			break;
		}
	}
	//printf("cnt:%d", cnt);

	//检测后面连续cnt-1个字节是不是以10开头
	int num = 0;
	for (size_t i = 1; i < cnt; i++)
	{
		if ((buf[i] >> 7) & 0x1 && !((buf[i] >> 6) & 0x1))
		{
			++num;
		}
	}
	if (num == cnt - 1)
	{
		return true;
	}
	return false;
}

//检查文字编码类型 utf8 gb2312  ascii(全部是ascii时返回)
const char* codeType(const char* buf, size_t len)
{
	for (size_t i = 0; i < len; i++)
	{
		//有中文
		if (buf[i] < 0)
		{
			if (isUtf8(buf + i))
			{
				return "utf8";
			}
			else
			{
				return "gb2312";
			}
		}
	}
	return "ascii";
}

//以gbk方式统计中文个数(2个字节存储)
size_t gbkCnt(const char* buf, size_t len)
{
	int cnt = 0;//多检测几个字符，判断是不是utf8
	for (size_t i = 0; i < len;)
	{
		//有中文
		if (buf[i] < 0)
		{
			i += 2;
			cnt++;
		}
		else
		{
			i++;
		}
	}
	//printf("gbkCnt 个数：%d\n", cnt);
	return cnt;
}

//以utf8方式统计中文个数(3个字节存储)
size_t utf8Cnt(const char* buf, size_t len)
{
	int cnt = 0;//多检测几个字符，判断是不是utf8
	for (size_t i = 0; i < len;)
	{
		//有中文
		if (buf[i] < 0)
		{
			i += 3;
			cnt++;
		}
		else
		{
			i++;
		}
	}
	//printf("utf8Cnt 个数：%d\n", cnt);
	return cnt;
}
int main()
{

	unsigned short unicode[10] = { 0 };
	int len = gbkToUnicode("123", unicode, 10);
	printf("len:%d\n", len);

	int lne = getGbkToUtf8Len("你好hello", 9);
	printf("lne:%d", lne);
	return 0;
}

int main33()
{
	const char* gbk = "你好AB";	//c4 e3 ba c3 
	binary(gbk, strlen(gbk));

	const char* ut8 = u8"你好AB";	//e4 bd a0 e5 a5 bd
	binary(ut8, strlen(ut8));

	printf("%d %d\n", gbk[0], ut8[0]);


	//printf("type: %s\n", codeType(gbk, strlen(gbk)));
	//printf("type: %s\n", codeType(ut8, strlen(ut8)));
	//printf("type: %s\n", codeType("asd123", 6));

	//printf("type: %s\n", codeType("-*-/545你好，我是隔壁的laowang，哈哈", 36));
	//printf("type: %s\n", codeType(u8"-*-/545你好，我是隔壁的laowang，哈哈", 47));

	gbkCnt(u8"-*-/545你好，我是隔壁的laowang，哈哈", 48);
	utf8Cnt(u8"-*-/545你好，我是隔壁的laowang，哈哈", 48);


	return 0;
}
/*
int main000()
{
	const char* strgbk = "你好";		//c4 e3 ba c3

	unsigned short unicode[50] = { 0 };
	int ret = gbkToUnicode(strgbk, unicode, 50);
	printf("size:%d  unicode:%s\n", ret, (char*)unicode);	//2  60 4f 7d 59

	char gbk[50] = { 0 };
	ret = unicodeToGbk(unicode, gbk, 50);
	printf("size:%d  gbk:%s\n", ret, gbk);


	const char* stru8 = u8"你好";		//e4 bd a0 e5 a5 bd

	char u8unicode[50] = { 0 };
	ret = utf8ToUnicode(stru8, u8unicode);
	printf("size:%d  u8unicode:%s\n", ret, (char*)u8unicode);	//2  60 4f 7d 59

	char u8gbk[50] = { 0 };
	ret = unicodeToUtf8(u8unicode, u8gbk);
	printf("size:%d  u8gbk:%s\n", ret, u8gbk);

	char gbkr[50] = { 0 };
	ret = unicodeToGbk((unsigned short*)u8unicode, gbkr, 50);
	printf("size:%d  gbk:%s\n", ret, gbkr);


	char nihao[50] = { 0 };
	ret = gbkToUtf8("你好", nihao);
	printf("size:%d  nihao:%s\n", ret, nihao);

	char res[50] = { 0 };
	ret = utf8ToGbk(nihao, res);
	printf("size:%d  res:%s\n", ret, res);


	const char *c = "\u4f60\u597d";	//c4 e3 ba c3


	//////1、gbk->unicode
	////char *strGbk ="我";
	////unsigned short strUnicode[5] ={0x00};
	////
	//// int num = GbkToUnicode(strGbk,strUnicode,5);  
 ////
 ////   unsigned char* p = (unsigned char*)strUnicode;     
 ////   for (int i = 0; i < num*2; i++)  
 ////   {  
 ////       printf("%0x", *p);  
 ////       p++;  
 ////   }//输出1162 
 ////   printf("\n");  

	////2、unicode->gbk
	//unsigned short strUnicode[2] = {0x6211,0x00};//加上0x00字符串结束符,终止函数内部的循环
	//char strGbk[5]={0x00};

	//int num = UnicodeToGbk(strUnicode,strGbk,5);

 //   unsigned char* p = (unsigned char*)strGbk;     
 //   for (int i = 0; i < num; i++)  
 //   {  
 //       printf("%0x", *p);  
 //       p++;  
 //   }//输出ced2 
 //   printf("\n");  
     return 0;
}
/*
	你好：
	gb2312 c4 e3 ba c3
	utf-8  e4 bd a0 e5 a5 bd
	unicode 63 6d b2 72 bd 30
*/