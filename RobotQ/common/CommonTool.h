#ifndef __COMMON_TOOL_H__
#define __COMMON_TOOL_H__

#include "include/cJSON.h"

namespace HciExampleComon{
    //���ÿ���̨��ӡ��ɫ
    void SetSpecialConsoleTextAttribute();
    //�ָ�����̨��ӡ��ɫ
    void SetOriginalConsoleTextAttribute();
    //����̨��ӡUTF8�ַ���windowsת��ΪGBK��
    void PrintUtf8String(char *pUTF8Str);
    //ת�뺯��
    int UTF8ToGBK(unsigned char * pUTF8Str,unsigned char ** pGBKStr);
	int GBKToUTF8(unsigned char * pGBKStr,unsigned char ** pUTF8Str);
    //ת���ڴ��ͷź���
    void FreeConvertResult(unsigned char * pConvertResult);

	//Jason��������
	int Json_Explain (char buf[],char Dest[],char result[]);
}

#endif // __FILE_UTIL_H__
