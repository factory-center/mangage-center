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
	//HINSTANCE mmm = ShellExecuteA(NULL, NULL, LOCAL_C_EXE_PATH, "", "", SW_SHOWNORMAL);
	ShellExecuteA(NULL, NULL, "D:\\MyGitCode\\factory-center\\trunk\\UpgradeProgram\\Win32\\Debug\\Carve_Control_Service\\Carve_Control_Service.exe", "", "", SW_SHOWNORMAL);
	//system("D:\\MyGitCode\\factory-center\\trunk\\UpgradeProgram\\Win32\\Debug\\Carve_Control_Service\\Carve_Control_Service.exe");
	int mmm = GetLastError();
	system("pause");
	return 0;
	//��ȡ�������򱾵������ļ�
	if (CLocalFileUpdateInfo::GetInstance()->GetLocalConfigInfo() == false)
	{
		return -1;
	}
	
	//��ȡ��������Զ�������ļ���У��汾
	bool versionIsEqual = false;
	bool rv = VersionValidation(versionIsEqual);
	if (rv == false)
	{
		return -1;
	}

	//�汾��ͬ��ֱ������
	if (versionIsEqual == true)
	{
		StartMainProgram();
		return 0;
	}

	//TODO �ߵ�����˵�����غ�Զ�̰汾��ͬ����Ҫ��������
	UpgradeProgram w;
	w.show();
	return a.exec();
}