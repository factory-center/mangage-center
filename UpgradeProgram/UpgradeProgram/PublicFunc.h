#pragma once
#include <iostream>
using namespace std;
//��ֹĳ������
int KillExe(const char* szExeName);

//��ȡ����ID
int GetPID(char * exename);

//·���ָ�
void PathSplit(string strTotalPath, string &strPath, string &strName);

