#include <QtWidgets/QApplication>
#include <windows.h>
#include "UpgradeProgram.h"
#include "VersionCheck.h"
#include "FileLoad.h"
#include "PublicFunc.h"
using namespace std;

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	//��ȡ�������򱾵������ļ�
	if (GetLocalConfigInfo() == false)
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
