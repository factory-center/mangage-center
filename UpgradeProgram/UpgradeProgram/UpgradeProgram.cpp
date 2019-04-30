#include "Downloader.h"
#include "FileLoad.h"
#include "PublicFunc.h"
#include "UpgradeProgram.h"
#include "VersionCheck.h"
#include "FileEncryption.h"
#include <windows.h>
#include <QString>
#include <stdlib.h>
#include <time.h>

// #include <QDateTime>

UpgradeProgram::UpgradeProgram(QWidget *parent)
	: QDialog(parent)
{
	setWindowFlags(Qt::FramelessWindowHint);//无边框
	ui.setupUi(this);

	//无边框相关---begin
	m_bPressed = false;
	m_areaMovable = geometry();
	//无边框相关---end


	this->setFixedSize(411, 183);
	ui.btComplete->hide();
	ui.label->setText(QString::fromLocal8Bit("当前有可升级版本，是否更新"));
	connect(ui.btCancel, SIGNAL(clicked()), this, SLOT(startExe()));
	connect(ui.btComplete, SIGNAL(clicked()), this, SLOT(startExe()));
	connect(ui.btConfirm, SIGNAL(clicked()), this, SLOT(startProgress()));
}


UpgradeProgram::~UpgradeProgram()
{

}
void UpgradeProgram::startProgress()
{
	ui.btCancel->setEnabled(false);
	ui.btConfirm->setEnabled(false);

	ui.label->setText(QString::fromLocal8Bit("正在升级......"));
	//关闭主程序
	KillExe("Carve_Control_Service.exe");
	KillExe("ruanserver.exe");

	//! 考虑到每次都做备份多次更新后会导致本地文件过大，改为删除需要替换的文件
	//文件备份
	//QString fileLocalInfo;
	//QString fileLocalInfoBak;
	//QDateTime current_date_time = QDateTime::currentDateTime();
	//QString current_date = current_date_time.toString("_yyyyMMddhhmmsszzz");
	//for (CFileUpdateInfo::GetInstance()->iter = CFileUpdateInfo::GetInstance()->vecFiles.begin();
	//	CFileUpdateInfo::GetInstance()->iter != CFileUpdateInfo::GetInstance()->vecFiles.end();
	//	(CFileUpdateInfo::GetInstance()->iter)++)
	//{
	//	fileLocalInfo.clear();
	//	fileLocalInfoBak.clear();
	//	fileLocalInfo = *(CFileUpdateInfo::GetInstance()->iter);
	//	fileLocalInfoBak = fileLocalInfo + current_date;
	//	//如果源文件不存在，直接下载；存在则进行重命名后下载 故：此函数不进行执行错误判断，成功or失败均继续执行
	//	rename(fileLocalInfo.toLocal8Bit(), fileLocalInfoBak.toLocal8Bit());
	//}

	DWORD tick = GetTickCount();
	CDownloader murl;

	for (auto iter = CRemoteConfigFileInfo::GetInstance()->vecUpdateFiles.begin();
		iter != CRemoteConfigFileInfo::GetInstance()->vecUpdateFiles.end();
		iter++)
	{
		//先删除后下载
		string strPath = "";
		string strName = "";
		PathSplit(*iter, strPath, strName);
		replace(strPath.begin(), strPath.end(), '\\', '/');
		string strUrl = CLocalFileUpdateInfo::GetInstance()->strRemoteConfigPathNoDirectory + strPath + strName;
		

		//string strUrl = "http://47.99.145.127:30010/Carve_Control_Service/Carve_Control_Service.exe";
		strPath = "./" + strPath;
		replace(strPath.begin(), strPath.end(), '/', '\\');
		strName = strName.substr(0, strName.length() - 1);
		remove((strPath + strName).c_str());		

		DLIO mDlWork;
		strcpy(mDlWork.url, strUrl.c_str());
		strcpy(mDlWork.filePath, strPath.c_str());
		murl.AddDownloadWork(mDlWork);
	}

	murl.StartDownloadThread();         //开启下载线程
	CURDI curInfo;
	double curDownloadLen, preLen = 0.0;
	while (1)
	{
		if (murl.IsDownloadBegin())
		{
			murl.GetCurrentDownloadInfo(&curInfo);  //获取每次下载的信息（一次相当于毫秒级，这里速度也用毫秒计算）
			curDownloadLen = curInfo.CurDownloadLen;
			ui.progressBar->setValue(((double)curInfo.preLocalLen + curInfo.CurDownloadLen) / curInfo.totalFileLen * 100);
			//printf("正在下载:%s,下载进度:%6.2lf%%,下载速度:%9.2lfKB/s\r",curInfo.fileName,
			//	((double)curInfo.preLocalLen+curInfo.CurDownloadLen)/curInfo.totalFileLen*100,(curDownloadLen-preLen)/(double)(GetTickCount()-tick));
			tick = GetTickCount();
			Sleep(500);
		}
		if (murl.IsDownloadEnd())
		{
			//文件解密
			for (auto iter = CRemoteConfigFileInfo::GetInstance()->vecUpdateFiles.begin();
				iter != CRemoteConfigFileInfo::GetInstance()->vecUpdateFiles.end();
				iter++)
			{
				string strPath = "";
				string strPathTemp = "";
				string strName = "";
				PathSplit(*iter, strPath, strName);
				
				strPath = ".\\" + *iter;
				strPathTemp = strPath;

				strPath = strPath.substr(0, strPath.length() - 1);
				
				decode((char*)strPathTemp.c_str(), (char*)strPath.c_str(), "ABCDE");
				remove(strPathTemp.c_str());
			}


			ui.progressBar->setValue(100);
			ui.label->setText(QString::fromLocal8Bit("升级完成"));
			ui.btComplete->setText(QString::fromLocal8Bit("完成"));
			ui.btConfirm->hide();
			ui.btComplete->show();
			break;
		}
		preLen = curDownloadLen;
	}
}

void UpgradeProgram::startExe()
{
	ShellExecuteA(NULL, NULL, ".\\Carve_Control_Service\\Carve_Control_Service.exe", "", "", SW_SHOWNORMAL);
	while (true)
	{
		if (GetPID("Carve_Control_Service.exe") != -1)
		{
			break;
		}
	}

	ShellExecuteA(NULL, NULL, ".\\ManagementControlUnit\\runserver.exe", "", "", SW_SHOWNORMAL);
	while (true)
	{
		if (GetPID("runserver.exe") != -1)
		{
			break;
		}
	}
	
	close();
}

void UpgradeProgram::mousePressEvent(QMouseEvent *e)
{
	//鼠标左键
	if (e->button() == Qt::LeftButton)
	{
		m_ptPress = e->pos();
		m_bPressed = m_areaMovable.contains(m_ptPress);
	}
}

void UpgradeProgram::mouseMoveEvent(QMouseEvent *e)
{
	if (m_bPressed)
	{
		move(pos() + e->pos() - m_ptPress);
	}
}

void UpgradeProgram::mouseReleaseEvent(QMouseEvent *)
{
	m_bPressed = false;
}

//设置鼠标按下的区域
void UpgradeProgram::setAreaMovable(const QRect rt)
{
	if (m_areaMovable != rt)
	{
		m_areaMovable = rt;
	}
}