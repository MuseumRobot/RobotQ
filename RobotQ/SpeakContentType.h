#pragma once
#include <list>

using namespace std;

class SpeakContentType{
public:
	SpeakContentType(): id(0) {}
	int id;
	char name[32];
	char short_content[100];
	char content[1600];
};

typedef list<SpeakContentType> SpeakContentTypeList;