#pragma once
#include <windows.h>
#include <string>

using namespace std;

int GetVersionInfo(string &VersionInfo,string VersionFilePath);
bool GetLocalConfigInfo();
bool VersionValidation(bool &versionIsEqual);
int StartMainProgram();