#pragma once
#include <iostream>
using namespace std;
//中止某个进程
int KillExe(const char* szExeName);

//获取进程ID
int GetPID(char * exename);

//路径分割
void PathSplit(string strTotalPath, string &strPath, string &strName);

