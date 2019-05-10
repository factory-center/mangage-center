#include <QtWidgets/QApplication>
#include <windows.h>
#include "UpgradeProgram.h"
#include "FileLoad.h"
#include "PublicFunc.h"
#include "PublicDefine.h"
using namespace std;

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	//只启动一个进程
	HANDLE  hHandle = CreateMutexA(NULL, FALSE, "UpgradeProgramMuter");
	if (ERROR_ALREADY_EXISTS == GetLastError())
	{
		return 0;
	}

	//读取升级程序本地配置文件
	if (CLocalFileUpdateInfo::GetInstance()->GetLocalConfigInfo() == false)
	{
		return -1;
	}
	
	//读取升级程序远程配置文件并校验版本
	bool versionIsEqual = false;
	bool rv = VersionValidation(versionIsEqual);
	if (rv == false)
	{
		return -1;
	}

	//版本相同，直接启动
	if (versionIsEqual == true)
	{
		StartMainProgram();
		return 0;
	}

	//TODO 走到这里说明本地和远程版本不同，需要进行升级
	UpgradeProgram w;
	w.show();
	return a.exec();
}
