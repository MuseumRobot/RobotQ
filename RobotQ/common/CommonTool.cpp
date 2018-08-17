#include "CommonTool.h"
#include <stdio.h>
#ifdef _WIN32
#include <windows.h>
#endif

#ifdef __LINUX__
#include <stdlib.h> 
#endif

namespace HciExampleComon{
	int Json_Explain (char buf[],char Dest[],char result[]){  
		cJSON *json , *json_result, *json_intention, *json_answer,*json_content,*date,*description,*direction,*high,*low,*location,
			*power,*domain,*content,*text,*date_gongli,*lunarDay,*lunarMonth,*week,*holiday,*channelName,*desc,*title;
		char temp[1000] = {NULL};
		// 解析数据包  
		json = cJSON_Parse(buf);  
		if (!json){  
			printf("Error before: [%s]\n",cJSON_GetErrorPtr());  
		}  
		else{// 解析开关值  
			json_result = cJSON_GetObjectItem( json, "result");  //intention=weather;calendar;train;flight;joke;story;baike
			json_answer =  cJSON_GetObjectItem(json , "answer");
			json_content = cJSON_GetObjectItem(json_answer , "content");
			json_intention=cJSON_GetObjectItem(json_answer , "intention");
			domain=cJSON_GetObjectItem(json_intention , "domain");
			if( json_result != NULL &&domain != NULL ){
				strcpy(result,json_result->valuestring);
				if(!strcmp(domain->valuestring,"weather")){
					// 从valuestring中获得结果  
					date = cJSON_GetObjectItem(json_content , "date");
					description = cJSON_GetObjectItem(json_content , "description");
					direction = cJSON_GetObjectItem(json_content , "direction");
					location = cJSON_GetObjectItem(json_content , "lcoation");
					power = cJSON_GetObjectItem(json_content , "power");
					high = cJSON_GetObjectItem(json_content , "high");
					low = cJSON_GetObjectItem(json_content , "low");

					strcpy(Dest,json_result->valuestring);
					strcpy(Dest,domain->valuestring);
					strcat(Dest,",");
					strcat(Dest,date->valuestring);
					strcat(Dest,",");
					strcat(Dest,description->valuestring);
					strcat(Dest,",");
					strcat(Dest,direction->valuestring);
					strcat(Dest,",");
					strcat(Dest,"风力");
					strcat(Dest,power->valuestring);
					strcat(Dest,",");
					//strcat(Dest,location->valuestring);
					strcat(Dest,"最高气温");
					itoa(high->valueint,temp,10);
					strcat(Dest,temp);
					strcat(Dest,"摄氏度");
					strcat(Dest,",");
					strcat(Dest,"最低气温");
					itoa(low->valueint,temp,10);
					strcat(Dest,temp);
					strcat(Dest,"摄氏度");
					strcat(Dest,"。");
				}
				else if(!strcmp(domain->valuestring,"joke")){
					content = cJSON_GetObjectItem(json_content , "content");	
					strcpy(Dest,content->valuestring);
				}
				else if(!strcmp(domain->valuestring,"story")){
					content = cJSON_GetObjectItem(json_content , "content");
					strcpy(Dest,content->valuestring);
				}
				else if(!strcmp(domain->valuestring,"baike")){
					content = cJSON_GetObjectItem(json_content , "content");
					text = content = cJSON_GetObjectItem(json_content , "text");
					strcpy(Dest,text->valuestring);

				}
				else if(!strcmp(domain->valuestring,"calendar")){
					content = cJSON_GetObjectItem(json_content , "content");
					date_gongli =  cJSON_GetObjectItem(json_content , "date");
					lunarMonth =  cJSON_GetObjectItem(json_content , "lunarMonthChinese");
					lunarDay =  cJSON_GetObjectItem(json_content , "lunarDayChinese");
					week =  cJSON_GetObjectItem(json_content , "weekday");
					holiday = cJSON_GetObjectItem(json_content , "holiday");
					strcpy(Dest,date_gongli->valuestring);
					strcat(Dest,",");
					strcat(Dest,"农历");
					strcat(Dest,lunarMonth->valuestring);
					strcat(Dest,lunarDay->valuestring);
					strcat(Dest,",");
					strcat(Dest,week->valuestring);
					strcat(Dest,",");
					strcat(Dest,holiday->valuestring);
					strcat(Dest,".");
				}
				else if(!strcmp(domain->valuestring,"translation")){
					content = cJSON_GetObjectItem(json_content , "content");
					text = content = cJSON_GetObjectItem(json_content , "text");
					strcpy(Dest,text->valuestring);
				}
				else if(!strcmp(domain->valuestring,"news")){
					content = cJSON_GetObjectItem(json_content , "content");
					channelName = cJSON_GetObjectItem(json_content , "channelName");
					desc = cJSON_GetObjectItem(json_content , "desc");
					title = cJSON_GetObjectItem(json_content , "title");
					strcpy(Dest,channelName->valuestring);
					strcat(Dest,",");
					strcat(Dest,title->valuestring);
					strcat(Dest,",");
					strcat(Dest,desc->valuestring);
				}	
			}
			else
			{  strcpy(Dest,"哎呀,这个问题我不会啊,你教教我吧!");}
			// 释放内存空间  
			cJSON_Delete(json); 
		}
		return 0;  
	}  
    void SetSpecialConsoleTextAttribute()
    {
#ifndef _WIN32_WCE
#ifdef _WIN32
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY );
#else
        printf( "\033[1;32;" );
#endif
#endif
    }

    void SetOriginalConsoleTextAttribute()
    {
#ifndef _WIN32_WCE
#ifdef _WIN32
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
#else
        printf( "\033[0;39m" );
#endif
#endif
    }

    void PrintUtf8String(char *pUTF8Str)
    {
#ifdef _WIN32
        unsigned char* pszGBK;
        UTF8ToGBK( (unsigned char*)pUTF8Str, (unsigned char**)&pszGBK);
        printf( "%s", pszGBK );
        FreeConvertResult( pszGBK );
#else
        printf( "%s", pUTF8Str );
#endif
    }

#ifdef _WIN32
int WTC(unsigned char * pUTF8Str,unsigned char * pGBKStr,int nGBKStrLen);
int CTW(unsigned char * lpGBKStr,unsigned char * lpUTF8Str,int nUTF8StrLen);
#endif

int UTF8ToGBK(unsigned char * pUTF8Str,unsigned char ** pGBKStr)
{
#ifdef _WIN32
    int nRetLen = 0;
    nRetLen = WTC(pUTF8Str,NULL,0);
    (* pGBKStr) = (unsigned char *)malloc((nRetLen + 1)*sizeof(char));
    if((* pGBKStr) == NULL)
        return 0;
    nRetLen = WTC(pUTF8Str,(* pGBKStr),nRetLen);
    return nRetLen;
#else
    //TODO
#endif
}

int GBKToUTF8(unsigned char * pGBKStr,unsigned char ** pUTF8Str)
{
#ifdef _WIN32
    int nRetLen = 0;
    nRetLen = CTW(pGBKStr,NULL,0);
    (* pUTF8Str) = (unsigned char *)malloc((nRetLen + 1)*sizeof(char));
    if((* pUTF8Str) == NULL)
        return 0;
    nRetLen = CTW(pGBKStr,(* pUTF8Str),nRetLen);
    return nRetLen;
#else
    //TODO
#endif
}


void FreeConvertResult(unsigned char * pConvertResult){
    if (pConvertResult){
        free(pConvertResult);
        pConvertResult = NULL;
    }
    
}

#ifdef _WIN32

int WTC(unsigned char * pUTF8Str,unsigned char * pGBKStr,int nGBKStrLen){
    wchar_t * lpUnicodeStr = NULL;
    int nRetLen = 0;
    if(!pUTF8Str)  //如果GBK字符串为NULL则出错退出
        return 0;
    nRetLen = MultiByteToWideChar(CP_UTF8,0,(char *)pUTF8Str,-1,NULL,0);  //获取转换到Unicode编码后所需要的字符空间长度
    //lpUnicodeStr = new WCHAR[nRetLen + 1];  //为Unicode字符串空间
    lpUnicodeStr = (WCHAR *)malloc((nRetLen + 1) * sizeof(WCHAR));  //为Unicode字符串空间
    if(lpUnicodeStr == NULL)
        return 0;
    memset(lpUnicodeStr,0,(nRetLen + 1) * sizeof(WCHAR));
    nRetLen = MultiByteToWideChar(CP_UTF8,0,(char *)pUTF8Str,-1,lpUnicodeStr,nRetLen);  //转换到Unicode编码
    if(!nRetLen)  //转换失败则出错退出
        return 0;
    nRetLen = WideCharToMultiByte(CP_ACP,0,lpUnicodeStr,-1,NULL,0,NULL,NULL);  //获取转换到UTF8编码后所需要的字符空间长度
    if(!pGBKStr)  //输出缓冲区为空则返回转换后需要的空间大小
    {
        if(lpUnicodeStr)       
            //delete []lpUnicodeStr;
            free(lpUnicodeStr);
        return nRetLen;
    }

    if(nGBKStrLen < nRetLen)  //如果输出缓冲区长度不够则退出
    {
        if(lpUnicodeStr)
            //delete []lpUnicodeStr;
            free(lpUnicodeStr);
        return 0;
    }

    nRetLen = WideCharToMultiByte(CP_ACP,0,lpUnicodeStr,-1,(char *)pGBKStr,nGBKStrLen,NULL,NULL);  //转换到UTF8编码

    if(lpUnicodeStr)
        //delete []lpUnicodeStr;
        free(lpUnicodeStr);

    return nRetLen;
}

int CTW(unsigned char * lpGBKStr,unsigned char * lpUTF8Str,int nUTF8StrLen)
{
    wchar_t * lpUnicodeStr = NULL;
    int nRetLen = 0;

    if(!lpGBKStr)  //如果GBK字符串为NULL则出错退出
        return 0;

    nRetLen = MultiByteToWideChar(CP_ACP,0,(char *)lpGBKStr,-1,NULL,0);  //获取转换到Unicode编码后所需要的字符空间长度
    //lpUnicodeStr = new WCHAR[nRetLen + 1];  //为Unicode字符串空间
    lpUnicodeStr = (WCHAR *)malloc((nRetLen + 1) * sizeof(WCHAR));  //为Unicode字符串空间
    if(lpUnicodeStr == NULL)
        return 0;


    nRetLen = MultiByteToWideChar(CP_ACP,0,(char *)lpGBKStr,-1,lpUnicodeStr,nRetLen);  //转换到Unicode编码
    if(!nRetLen)  //转换失败则出错退出
        return 0;

    nRetLen = WideCharToMultiByte(CP_UTF8,0,lpUnicodeStr,-1,NULL,0,NULL,NULL);  //获取转换到UTF8编码后所需要的字符空间长度

    if(!lpUTF8Str)  //输出缓冲区为空则返回转换后需要的空间大小
    {
        if(lpUnicodeStr)       
            //delete []lpUnicodeStr;
            free(lpUnicodeStr);
        return nRetLen;
    }

    if(nUTF8StrLen < nRetLen)  //如果输出缓冲区长度不够则退出
    {
        if(lpUnicodeStr)
            //delete []lpUnicodeStr;
            free(lpUnicodeStr);
        return 0;
    }

    nRetLen = WideCharToMultiByte(CP_UTF8,0,lpUnicodeStr,-1,(char *)lpUTF8Str,nUTF8StrLen,NULL,NULL);  //转换到UTF8编码

    if(lpUnicodeStr)
        //delete []lpUnicodeStr;
        free(lpUnicodeStr);
    return nRetLen;
}

#endif

}
