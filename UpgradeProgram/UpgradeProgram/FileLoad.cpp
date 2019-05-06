#include "FileLoad.h"
#include "PublicDefine.h"
#include <windows.h>

CLocalFileUpdateInfo::CLocalFileUpdateInfo()
{
	strCFileLocalPath.clear();
	strCFileRemotePath.clear();
	strPFileLocalPath.clear();
	strPFileRemotePath.clear();
	strRemoteConfigPath.clear();
	strRemoteConfigPathNoDirectory.clear();
}
bool CLocalFileUpdateInfo::GetLocalConfigInfo()
{
	//读远程url地址---完整地址
	char bufTemp[1024] = { 0 };
	GetPrivateProfileStringA("url", "RemoteUrl", "", bufTemp, 1024, LOCAL_CONFIG_PATH);
	strRemoteConfigPath = bufTemp;

	//读远程url地址---不带目录
	memset(bufTemp, 0x00, sizeof(bufTemp));
	GetPrivateProfileStringA("url", "RemoteUrlNoDirectory", "", bufTemp, 1024, LOCAL_CONFIG_PATH);
	strRemoteConfigPathNoDirectory = bufTemp;

	if (strRemoteConfigPath.length()==0 || strRemoteConfigPathNoDirectory.length()==0)
	{
		return false;
	}
	return true;
}



CRemoteConfigFileInfo::CRemoteConfigFileInfo()
{
	vecUpdateFiles.clear();
	mapVersionFiles.clear();
}

bool CRemoteConfigFileInfo::GetRemoteConfigVersionInfo()
{
	//读取远程软件版本号
	char bufTemp[1024] = { 0 };
	GetPrivateProfileStringA("version", "CVersion", "", bufTemp, 1024, REMOTE_LOCAL_CONFIG_PATH);
	mapVersionFiles["CVersion"] = bufTemp;
	memset(bufTemp, 0x00, sizeof(bufTemp));

	GetPrivateProfileStringA("version", "PVersion", "", bufTemp, 1024, REMOTE_LOCAL_CONFIG_PATH);
	mapVersionFiles["PVersion"] = bufTemp;
	memset(bufTemp, 0x00, sizeof(bufTemp));

	GetPrivateProfileStringA("files", "FilesNum", "", bufTemp, 1024, REMOTE_LOCAL_CONFIG_PATH);
	int fileNum = std::stoi(bufTemp);

	for (int i = 1; i <= fileNum; i++)
	{
		string strFileName = "File" + to_string(i);
		memset(bufTemp, 0x00, sizeof(bufTemp));
		GetPrivateProfileStringA("files", strFileName.c_str(), "", bufTemp, _MAX_PATH * fileNum - 1, REMOTE_LOCAL_CONFIG_PATH);
		vecUpdateFiles.push_back(bufTemp);
	}

	//版本号和文件列表均不能为空 --- 如果没有更新信息，保留上次更新信息
	if (mapVersionFiles.empty() == true || vecUpdateFiles.empty() == true) 
	{
		return false;
	}

	return true;
}