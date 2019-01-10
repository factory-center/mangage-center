#include <stdio.h>
#include <iostream>
#include <SDKDDKVer.h>
#include "../source/busin_log.h"
#include "../source/CCarve.h"
#include "utils/msp_errors.h"
#include "../source/carve_common_lib.h"
#include "../source/http_server/server.hpp"
#include "../source/http_server/singleton_server.h"
#include <boost_common.h>
#include <json/json.h>


using namespace std;



unsigned int nConn_idx = 0;
const string str_ip = "192.168.101.212";
boost::shared_ptr<CCarve> carve_ptr;


bool brun=false;
SERVICE_STATUS servicestatus;
SERVICE_STATUS_HANDLE hstatus;
void WINAPI ServiceMain(int argc, char** argv);
void WINAPI CtrlHandler(DWORD request);

void WINAPI ServiceMain(int argc, char** argv)
{
	servicestatus.dwServiceType = SERVICE_WIN32;
	servicestatus.dwCurrentState = SERVICE_START_PENDING;
	servicestatus.dwControlsAccepted = SERVICE_ACCEPT_SHUTDOWN|SERVICE_ACCEPT_STOP;//�ڱ�����ֻ����ϵͳ�ػ���ֹͣ�������ֿ�������
	servicestatus.dwWin32ExitCode = 0;
	servicestatus.dwServiceSpecificExitCode = 0;
	servicestatus.dwCheckPoint = 0;
	servicestatus.dwWaitHint = 0;

	hstatus = ::RegisterServiceCtrlHandler(L"CarveControlServiceTest", CtrlHandler);
	if (hstatus==0)
	{
		return;
	}

	//��SCM ��������״̬
	servicestatus.dwCurrentState = SERVICE_RUNNING;
	SetServiceStatus (hstatus, &servicestatus);

	//�����������

	//Ĭ����־���ã�����μ�����Log_Cfg_T
	string str_log_path = "../log/emcs.log";
	businlog_cfg default_cfg(str_log_path.c_str(), "Network Engraving Machine Logging");
	default_cfg.level(7);
	//�������ļ������ʧ�ܣ���ʹ��Ĭ����־����
	int ret = businlog_open(default_cfg, "emcs.cfg");
	if (ret)
	{
		businlog_error("%s | open log file:%s error, ret:%d", __FUNCTION__, str_log_path.c_str(), ret);
	}

	//��ʼ����
	string str_err_reason;
	bool bSuccess = CCarve_Common_Lib_Tool::instance()->init(str_err_reason);
	//businlog_error_return(bSuccess, ("%s | fail to init CCarve_Common_Lib_Tool, reason:%s", __FUNCTION__, str_err_reason.c_str()), FALSE);

	//��������ģ�飺�����߳��Լ����˿�
	//TODO::������������ط��������ú�ip:port��Ŀǰ�������ﲢд��
	string str_local_ip  = "192.168.101.20";
	string str_port = "8899";
	size_t nThread_num = 5;

	singleton_default<CSingleton_Server>::instance().start(str_local_ip, str_port, nThread_num, str_err_reason);
}

void WINAPI CtrlHandler(DWORD request)
{
	switch (request)
	{
	case SERVICE_CONTROL_STOP:
		brun=false;
		servicestatus.dwCurrentState = SERVICE_STOPPED;
		break;

	case SERVICE_CONTROL_SHUTDOWN:
		brun=false;
		servicestatus.dwCurrentState = SERVICE_STOPPED;
		break;

	default:
		break;
	}

	SetServiceStatus (hstatus, &servicestatus);
}


int main()
{
	SERVICE_TABLE_ENTRY entrytable[2];
	entrytable[0].lpServiceName= L"CarveControlServiceTest";
	entrytable[0].lpServiceProc=(LPSERVICE_MAIN_FUNCTION)ServiceMain;
	entrytable[1].lpServiceName=NULL;
	entrytable[1].lpServiceProc=NULL;
	StartServiceCtrlDispatcher(entrytable);

	return 0;
}