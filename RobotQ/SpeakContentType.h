#pragma once
#include <list>

using namespace std;

class SpeakContentType{
public:
	SpeakContentType(): id(0) {}
	int id;
	char name[100];			//��������
	char content[6000];		//��������
};

typedef list<SpeakContentType> SpeakContentTypeList;