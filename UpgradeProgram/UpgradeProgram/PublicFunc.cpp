#include <windows.h>
#include <AtlConv.h>
#include "PublicFunc.h"
#include "tlhelp32.h" 
#include "Downloader.h"
#include "FileLoad.h"
#include "PublicDefine.h" 
#include "PublicFunc.h" 
#include "tlhelp32.h" 
#include "FileEncryption.h" 
using namespace std;



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

int GetVersionInfo(string &VersionInfo, string VersionFilePath)
{
	int status = 0;
	int s_major_ver = 0;
	int s_minor_ver = 0;
	int s_build_num = 0;
	int s_revision_num = 0;
	char* buf = NULL;
	if (0 == s_major_ver && 0 == s_minor_ver && 0 == s_build_num && 0 == s_revision_num)
	{
		char temp[1024];
		DWORD dwHnd;
		DWORD dwVerInfoSize;
		if (0 >= (dwVerInfoSize = GetFileVersionInfoSizeA(((const char *)VersionFilePath.c_str()), &dwHnd)))
		{
			status = -2;
			goto _ERROR;
		}
		buf = new char[dwVerInfoSize];
		if (NULL == buf)
		{
			status = -3;
			goto _ERROR;
		}
		if (!GetFileVersionInfoA((const char *)VersionFilePath.c_str(), dwHnd, dwVerInfoSize, buf))
		{
			status = -4;
			goto _ERROR;
		}

		VS_FIXEDFILEINFO* p_version;
		unsigned int version_len = 0;
		if (!VerQueryValueA(buf, "\\", (void**)&p_version, &version_len))
		{
			status = -6;
			goto _ERROR;
		}

		s_major_ver = (p_version->dwFileVersionMS >> 16) & 0x0000FFFF;
		s_minor_ver = p_version->dwFileVersionMS & 0x0000FFFF;
		s_build_num = (p_version->dwFileVersionLS >> 16) & 0x0000FFFF;
		s_revision_num = p_version->dwFileVersionLS & 0x0000FFFF;

		char tempBuf[8] = { 0 };
		sprintf(tempBuf, "%d", s_major_ver);
		VersionInfo = VersionInfo + tempBuf + ".";
		memset(tempBuf, 0x00, sizeof(tempBuf));

		sprintf(tempBuf, "%d", s_minor_ver);
		VersionInfo = VersionInfo + tempBuf + ".";
		memset(tempBuf, 0x00, sizeof(tempBuf));

		sprintf(tempBuf, "%d", s_build_num);
		VersionInfo = VersionInfo + tempBuf + ".";
		memset(tempBuf, 0x00, sizeof(tempBuf));

		sprintf(tempBuf, "%d", s_revision_num);
		VersionInfo = VersionInfo + tempBuf;
		memset(tempBuf, 0x00, sizeof(tempBuf));
	}

_ERROR:
	delete[] buf;
	return status;
}


void GetRemoteConfig()
{
	DWORD tick = GetTickCount();
	CDownloader murl;
	DLIO mDlWork;

	strcpy(mDlWork.url, CLocalFileUpdateInfo::GetInstance()->strRemoteConfigPath.c_str());
	strcpy(mDlWork.filePath, ".\\");
	murl.AddDownloadWork(mDlWork);      //添加到下载任务中
	murl.StartDownloadThread();         //开启下载线程
	while (1)
	{
		if (murl.IsDownloadEnd())
		{
			break;
		}
		Sleep(10);
	}
}

bool GetRemoteConfigVersionInfo()
{
	//读取远程软件版本号

	char bufTemp[1024] = { 0 };
	GetPrivateProfileStringA("version", "CVersion", "", bufTemp, 1024, REMOTE_LOCAL_CONFIG_PATH);
	CRemoteConfigFileInfo::GetInstance()->mapVersionFiles["CVersion"] = bufTemp;
	memset(bufTemp, 0x00, sizeof(bufTemp));

	//GetPrivateProfileStringA("version", "PVersion", "", bufTemp, 1024, REMOTE_LOCAL_CONFIG_PATH);
	//CRemoteConfigFileInfo::GetInstance()->mapVersionFiles["PVersion"] = bufTemp;
	memset(bufTemp, 0x00, sizeof(bufTemp));

	GetPrivateProfileStringA("files", "FilesNum", "", bufTemp, 1024, REMOTE_LOCAL_CONFIG_PATH);
	int fileNum = std::stoi(bufTemp);

	for (int i = 1; i <= fileNum; i++)
	{
		string strFileName = "File" + to_string(i);
		memset(bufTemp, 0x00, sizeof(bufTemp));
		GetPrivateProfileStringA("files", strFileName.c_str(), "", bufTemp, _MAX_PATH * fileNum - 1, REMOTE_LOCAL_CONFIG_PATH);
		CRemoteConfigFileInfo::GetInstance()->vecUpdateFiles.push_back(bufTemp);
	}

	return true;
}


bool VersionValidation(bool &versionIsEqual)
{
	//下载远程配置文件
	GetRemoteConfig();
	//解密文件
	decrypt(REMOTE_CONFIG_PATH,123321);
	//获取远程配置文件版本信息
	CRemoteConfigFileInfo::GetInstance()->GetRemoteConfigVersionInfo();

	//读取本地软件版本号
	string strCLocalExeVersion;
	string strPLocalExeVersion;
	string strCRemoteExeVersion;
	string strPRemoteExeVersion;
	GetVersionInfo(strCLocalExeVersion, LOCAL_C_EXE_PATH);
	GetVersionInfo(strPLocalExeVersion, LOCAL_P_EXE_PATH);

	for (auto iter = CRemoteConfigFileInfo::GetInstance()->mapVersionFiles.begin();
		iter != CRemoteConfigFileInfo::GetInstance()->mapVersionFiles.end();
		iter++)
	{
		if (iter->first == "CVersion")
		{
			strCRemoteExeVersion = iter->second;
		}
		if (iter->first == "PVersion")
		{
			strPRemoteExeVersion = iter->second;
		}
	}

	if ((strCRemoteExeVersion == strCLocalExeVersion)
		&& (strPRemoteExeVersion == strPLocalExeVersion))
	{
		versionIsEqual = true;
	}

	return true;
}

void StartMainProgram()
{
	//检查程序是否已经启动--如果已启动，直接退出
	if (GetPID(LOCAL_C_EXE_NAME) == -1)
	{
		//未启动->启动
		ShellExecuteA(NULL, NULL, LOCAL_C_EXE_PATH, "", "", SW_SHOWNORMAL);
		while (true)
		{
			if (GetPID(LOCAL_C_EXE_NAME) != -1)
			{
				break;
			}
		}
		KillExe(LOCAL_P_EXE_NAME);
		Sleep(100);
		ShellExecuteA(NULL, NULL, LOCAL_P_EXE_PATH, "", "", SW_SHOWNORMAL);
		while (true)
		{
			if (GetPID(LOCAL_P_EXE_NAME) != -1)
			{
				break;
			}
		}
	}
	else
	{
		//控制层已经启动，检测python层
		if (GetPID(LOCAL_P_EXE_NAME) == -1)
		{
			ShellExecuteA(NULL, NULL, LOCAL_P_EXE_PATH, "", "", SW_SHOWNORMAL);
			while (true)
			{
				if (GetPID(LOCAL_P_EXE_NAME) != -1)
				{
					break;
				}
			}
		}
	}
}