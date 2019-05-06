#pragma once
#include <string>
#include <vector>
#include <map>
using namespace std;

//�������򱾵�������
class CLocalFileUpdateInfo
{
public:
	static CLocalFileUpdateInfo* const GetInstance()
	{
		static CLocalFileUpdateInfo* pFileUpdateInfo = new CLocalFileUpdateInfo();
		return pFileUpdateInfo;
	}

	CLocalFileUpdateInfo();

	bool GetLocalConfigInfo();


	string strCFileLocalPath;
	string strCFileRemotePath;
	string strPFileLocalPath;
	string strPFileRemotePath;
	string strRemoteConfigPath;
	string strRemoteConfigPathNoDirectory;
};

//��������Զ��������
class CRemoteConfigFileInfo
{
public:
	static CRemoteConfigFileInfo* const GetInstance()
	{
		static CRemoteConfigFileInfo* pFileUpdateInfo = new CRemoteConfigFileInfo();
		return pFileUpdateInfo;
	}

	CRemoteConfigFileInfo();



	bool GetRemoteConfigVersionInfo();

	//�ļ��б�
	vector<string> vecUpdateFiles;
	//�ļ��汾�б�
	map<string, string> mapVersionFiles;
};