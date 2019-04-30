#include <windows.h>
#include "Downloader.h"
#include "FileLoad.h"
#include "PublicDefine.h" 
#include "PublicFunc.h" 
#include "tlhelp32.h" 
#include "FileEncryption.h" 
#include "VersionCheck.h"
#include <QDateTime>
#include <exception>
#include <QMessagebox>
using namespace std;

int GetVersionInfo(string &VersionInfo,string VersionFilePath)
{
	int status = 0;
	int s_major_ver = 0;
	int s_minor_ver = 0;
	int s_build_num = 0;
	int s_revision_num = 0;
	char* buf = NULL;
	if( 0 == s_major_ver && 0 == s_minor_ver && 0 == s_build_num && 0 == s_revision_num )
	{
		char temp[1024];
		DWORD dwHnd;
		DWORD dwVerInfoSize;
		if( 0 >= (dwVerInfoSize = GetFileVersionInfoSizeA(((const char *)VersionFilePath.c_str()),&dwHnd)))
		{
			status = -2;
			goto _ERROR;
		}
		buf = new char[dwVerInfoSize];
		if( NULL == buf )
		{
			status = -3;
			goto _ERROR;
		}
		if(!GetFileVersionInfoA((const char *)VersionFilePath.c_str(),dwHnd,dwVerInfoSize,buf))
		{
			status = -4;
			goto _ERROR;
		}
 
		VS_FIXEDFILEINFO* p_version;
		unsigned int version_len = 0;
		if(!VerQueryValueA(buf, "\\", (void**)&p_version, &version_len))
		{
			status = -6;
			goto _ERROR;
		}
 
		s_major_ver = (p_version->dwFileVersionMS >> 16) & 0x0000FFFF;
		s_minor_ver = p_version->dwFileVersionMS & 0x0000FFFF;
		s_build_num = (p_version->dwFileVersionLS >> 16) & 0x0000FFFF;
		s_revision_num = p_version->dwFileVersionLS & 0x0000FFFF;

		char tempBuf[8] = {0};
		sprintf(tempBuf,"%d",s_major_ver);
		VersionInfo = VersionInfo + tempBuf + ".";
		memset(tempBuf,0x00,sizeof(tempBuf));

		sprintf(tempBuf,"%d",s_minor_ver);
		VersionInfo = VersionInfo + tempBuf + ".";
		memset(tempBuf,0x00,sizeof(tempBuf));

		sprintf(tempBuf,"%d",s_build_num);
		VersionInfo = VersionInfo + tempBuf + ".";
		memset(tempBuf,0x00,sizeof(tempBuf));

		sprintf(tempBuf,"%d",s_revision_num);
		VersionInfo = VersionInfo + tempBuf;
		memset(tempBuf,0x00,sizeof(tempBuf));
	}
	
_ERROR:
	delete [] buf;
	return status;
}

bool GetLocalConfigInfo()
{

	char bufTemp[1024] = { 0 };
	GetPrivateProfileStringA("url", "RemoteUrl", "", bufTemp, 1024, LOCAL_CONFIG_PATH);
	if (strlen(bufTemp) == 0)
	{
		return false;
	}
	CLocalFileUpdateInfo::GetInstance()->strRemoteConfigPath = bufTemp;

	memset(bufTemp, 0x00, sizeof(bufTemp));
	GetPrivateProfileStringA("url", "RemoteUrlNoDirectory", "", bufTemp, 1024, LOCAL_CONFIG_PATH);
	CLocalFileUpdateInfo::GetInstance()->strRemoteConfigPathNoDirectory = bufTemp;

	return true;
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
	while(1)
	{
		if(murl.IsDownloadEnd()) 
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
	QString strUrlPath;

	//下载远程配置文件
	GetRemoteConfig();
	//解密文件
	//decode(REMOTE_CONFIG_PATH, LOCAL_CONFIG_PATH,"ABCDE");
	//获取远程配置文件版本信息
	GetRemoteConfigVersionInfo();
	
	//读取本地软件版本号
	string strCLocalExeVersion;
	string strPLocalExeVersion;
	string strCRemoteExeVersion;
	string strPRemoteExeVersion;
	GetVersionInfo(strCLocalExeVersion, "./Carve_Control_Service/Carve_Control_Service.exe");
	//GetVersionInfo(strPLocalExeVersion, "./runserver.exe");

	for (auto iter = CRemoteConfigFileInfo::GetInstance()->mapVersionFiles.begin(); 
		iter != CRemoteConfigFileInfo::GetInstance()->mapVersionFiles.end();
		iter++)
	{
		if (iter->first == "CVersion")
		{
			strCRemoteExeVersion = iter->second;
		}
		if (iter->first == "runserver.exe")
		{
			strPRemoteExeVersion = iter->second;
		}		
	}

	if ((strCRemoteExeVersion == strCLocalExeVersion)
		/*&& (strPRemoteExeVersion == strPLocalExeVersion)*/)
	{
		versionIsEqual = true;
	}

	return true;
}

int StartMainProgram()
{
	//版本相同，不需要更新
		//检查程序是否已经启动--如果已启动，直接退出
	if (GetPID("Carve_Control_Service.exe") == -1)
	{
		//未启动->启动
		ShellExecuteA(NULL, NULL, ".\\Carve_Control_Service\\Carve_Control_Service.exe", "", "", SW_SHOWNORMAL);
		while (true)
		{
			if (GetPID("Carve_Control_Service.exe") != -1)
			{
				break;
			}
		}
		KillExe("runserver.exe");
		Sleep(100);
		ShellExecuteA(NULL, NULL, ".\\ManagementControlUnit\\runserver.exe", "", "", SW_SHOWNORMAL);
		while (true)
		{
			if (GetPID("runserver.exe") != -1)
			{
				break;
			}
		}
	}
	else
	{
		//控制层已经启动，检测python层
		if (GetPID("runserver.exe") == -1)
		{
			ShellExecuteA(NULL, NULL, "runserver.exe", "", "", SW_SHOWNORMAL);
			while (true)
			{
				if (GetPID("runserver.exe") != -1)
				{
					break;
				}
			}
		}
	}
	return true;
}