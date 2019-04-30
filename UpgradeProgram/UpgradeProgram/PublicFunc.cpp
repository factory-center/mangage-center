#include <windows.h>
#include <AtlConv.h>
#include "PublicFunc.h"
#include "VersionCheck.h"
#include "tlhelp32.h" 



int KillExe(const char* szExeName)
{
	if (szExeName == NULL)
	{
		return -1;
	}

	int lsr = lstrlenA(szExeName);
	if (lsr == 0)
	{
		return -1;
	}

	PROCESSENTRY32 my;
	HANDLE l = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (((int)l) != -1)
	{
		my.dwSize = sizeof(my);
		if (Process32First(l, &my))
		{
			do
			{
				USES_CONVERSION;
				if (lstrcmpiA(szExeName, W2A(my.szExeFile) + lstrlen(my.szExeFile) - lsr) == 0)
				{
					HANDLE hProcessHandle;
					hProcessHandle = OpenProcess(0x1F0FFF, true, my.th32ProcessID);
					if (hProcessHandle != NULL)
					{
						if (TerminateProcess(hProcessHandle, 0) != NULL)
						{
							CloseHandle(hProcessHandle);
						}
					}
					CloseHandle(l);
					return 0;
				}
			} while (Process32Next(l, &my));
		}
		CloseHandle(l);
	}
	return 0;
}

int GetPID(char * exename)
{
	bool			flag;
	PROCESSENTRY32	pe32;

	flag = false;
	pe32.dwSize = sizeof(pe32);

	char chName[MAX_PATH * 2] = { 0 };
	memcpy_s(chName, sizeof(chName), exename, strlen(exename));

	_strupr(chName);
	//给系统内的所有进程拍一个快照; 
	HANDLE hProcessSnap = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hProcessSnap == INVALID_HANDLE_VALUE)
	{
		return -1;
	}

	//遍历进程快照，轮流显示每个进程的信息;
	BOOL bMore = ::Process32First(hProcessSnap, &pe32);
	USES_CONVERSION;
	char * PTemp = NULL;
	while (bMore)
	{
		PTemp = _strupr(W2A(pe32.szExeFile));
		if (strcmp(chName, PTemp) == 0)
		{
			flag = true;
			break;
		}

		bMore = Process32Next(hProcessSnap, &pe32);
	}
	CloseHandle(hProcessSnap);

	if (flag == true)
	{
		return pe32.th32ProcessID;
	}
	else
	{
		return -1;
	}
}

void PathSplit(string strTotalPath, string &strPath, string &strName)
{
	char driver[MAX_PATH] = { 0 };
	char path[MAX_PATH] = { 0 };

	char tmp[MAX_PATH] = { 0 };

	_splitpath_s(strTotalPath.c_str(), driver, sizeof(driver), path, sizeof(path), NULL, 0, NULL, 0);

	strcat(tmp, driver);
	strcat(tmp, path);
	strPath = tmp;

	strName = strTotalPath.substr(strlen(tmp), strTotalPath.length() - strlen(tmp));

}
