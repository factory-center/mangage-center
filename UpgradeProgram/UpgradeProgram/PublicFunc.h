#pragma once
#include <windows.h>
#include <string>
#include <iostream>
using namespace std;

//��ֹĳ������
int KillExe(const char* szExeName);
//��ȡ����ID
int GetPID(char * exename);
//·���ָ�
void PathSplit(string strTotalPath, string &strPath, string &strName);
//��ȡ����汾��Ϣ
int GetVersionInfo(string &VersionInfo, string VersionFilePath);
//�汾У��
bool VersionValidation(bool &versionIsEqual);
//��������
void StartMainProgram();

