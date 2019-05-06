#pragma once
#include <windows.h>
#include <string>
#include <iostream>
using namespace std;

//中止某个进程
int KillExe(const char* szExeName);
//获取进程ID
int GetPID(char * exename);
//路径分割
void PathSplit(string strTotalPath, string &strPath, string &strName);
//获取软件版本信息
int GetVersionInfo(string &VersionInfo, string VersionFilePath);
//版本校验
bool VersionValidation(bool &versionIsEqual);
//启动程序
void StartMainProgram();

