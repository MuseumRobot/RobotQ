#pragma once
#include <list>

using namespace std;

class SpeakContentType{
public:
	SpeakContentType(): id(0) {}
	int id;
	char name[100];			//语料名称
	char content[6000];		//语料内容
};

typedef list<SpeakContentType> SpeakContentTypeList;