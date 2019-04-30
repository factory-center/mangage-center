#pragma once
#include <string>
#include <vector>
#include <map>
using namespace std;

//升级程序本地配置类
class CLocalFileUpdateInfo
{
public:
	static CLocalFileUpdateInfo* const GetInstance()
	{
		static CLocalFileUpdateInfo* pFileUpdateInfo = new CLocalFileUpdateInfo();
		return pFileUpdateInfo;
	}

	CLocalFileUpdateInfo()
	{
		strCFileLocalPath.clear();
		strCFileRemotePath.clear();
		strPFileLocalPath.clear();
		strPFileRemotePath.clear();
		strRemoteConfigPath.clear();
		strRemoteConfigPathNoDirectory.clear();
	}

	string strCFileLocalPath;
	string strCFileRemotePath;
	string strPFileLocalPath;
	string strPFileRemotePath;
	string strRemoteConfigPath;
	string strRemoteConfigPathNoDirectory;
};

//升级程序远程配置类
class CRemoteConfigFileInfo
{
public:
	static CRemoteConfigFileInfo* const GetInstance()
	{
		static CRemoteConfigFileInfo* pFileUpdateInfo = new CRemoteConfigFileInfo();
		return pFileUpdateInfo;
	}

	CRemoteConfigFileInfo()
	{
		vecUpdateFiles.clear();
		mapVersionFiles.clear();
	}

	//文件列表
	vector<string> vecUpdateFiles;
	
	//文件版本列表
	map<string, string> mapVersionFiles;
};