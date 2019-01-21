#include <iostream> 
#include <stdio.h> 
#include <tchar.h> 
#include <string>
#include <fstream>
#include <strsafe.h>
#include "../source/busin_log.h"
using namespace std;
// Display the error message and exit the process
void ErrorExit(LPTSTR lpszFunction) 
{ 
	// Retrieve the system error message for the last-error code

	LPVOID lpMsgBuf;
	LPVOID lpDisplayBuf;
	DWORD dw = GetLastError(); 

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | 
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dw,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR) &lpMsgBuf,
		0, NULL );

	// Display the error message and exit the process

	lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, 
		(lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR)); 
	StringCchPrintf((LPTSTR)lpDisplayBuf, 
		LocalSize(lpDisplayBuf) / sizeof(TCHAR),
		TEXT("%s failed with error %d: %s"), 
		lpszFunction, dw, lpMsgBuf); 
	MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK); //�ԶԻ�����ʽ��ʾ
	//TODO::Ŀǰû�гɹ���������־д���ļ�
	businlog_error("%s | err reason:%s", __FUNCTION__, (LPCTSTR)lpDisplayBuf);
	LocalFree(lpMsgBuf);
	LocalFree(lpDisplayBuf);
	businlog_warn("%s | �˳�����", __FUNCTION__);
	ExitProcess(dw); //�˳�����
}

int _tmain(int argc, TCHAR *argv[]) 
{ 
	//�ػ����̵���־�ļ�
	string str_log_path = "../log/daemon.log";
	//Ĭ����־���ã�����μ�����Log_Cfg_T
 	businlog_cfg default_cfg(str_log_path.c_str(), "Daemon Logging");
	default_cfg.level(7);
	//����Ҫ�����ļ����ʽ����趨Ϊ��
	string str_cfg_file_path = "";
	//�������ļ������ʧ�ܣ���ʹ��Ĭ����־���á��ʲ���Ҫ�ж��䷵��ֵ
	int ret = businlog_open(default_cfg, str_cfg_file_path.c_str());
	//����Ŀ�����
	try
	{
		STARTUPINFO si; 
		PROCESS_INFORMATION pi; //������Ϣ�� 
		//��ʼ��
		ZeroMemory(&si, sizeof(si)); 
		si.cb = sizeof(si); 
		ZeroMemory(&pi, sizeof(pi)); 
		do
		{ 
			//���ַ�������ָ��Ҫִ�е�ģ�������·�����ļ�����Ҳ����ָ���������ơ�
			//���ڲ������ƣ��ú���ʹ�õ�ǰ�������͵�ǰĿ¼����ɹ淶��
			//�ù��ܲ���ʹ������·�����˲�����������ļ���չ��; û��Ĭ�ϵ���չ��
			LPTSTR lp_app_name =  L"./Carve_Control_Service.exe";
			//�������ַ���(����Ϊ�˲���������ֻ���ڴ��ָ�룺����const�����������ַ���)
			//TODO::������Դ�argv������ȡ������
			// �����ӽ��̣��ж��Ƿ�ִ�гɹ� 
			if(!CreateProcess(
				lp_app_name //TODO::����argv�л�ȡ������ʱ������ʹ��NULL
				, NULL //TODO::����� argv�л�ȡ������
				, NULL
				,NULL
				, FALSE
				, 0
				, NULL
				, NULL
				, &si
				, &pi)) 
			{
				cout << "����Ŀ�����ʧ��" << std::endl;
				ErrorExit(TEXT("CreateProcess"));
				return -1; 
			} 
			//����ִ�гɹ�����ӡ������Ϣ 
			cout << "����Ŀ����̳ɹ���������Ϣ��" << endl; 
			cout << "\t����ID:" << pi.dwProcessId << endl; 
			cout << "\t�߳�ID:" << pi.dwThreadId << endl; 
			businlog_crit("%s | ����Ŀ����̳ɹ���������Ϣ����:\n\t����ID:%d\n\t�߳�ID:%d"
				, __FUNCTION__, pi.dwProcessId, pi.dwProcessId);
			// �ȴ�֪���ӽ����˳�... 
			WaitForSingleObject( pi.hProcess, INFINITE);//�������Ƿ�ֹͣ 
			//WaitForSingleObject()�����������״̬�������δȷ������ȴ�����ʱ 
			//�ӽ����˳� 
			cout << "Ŀ������Ѿ��˳�..." << endl;
			businlog_error("%s | Ŀ������Ѿ��˳���", __FUNCTION__);
			//�رս��̺;�� 
			CloseHandle(pi.hProcess); 
			CloseHandle(pi.hThread); 
			//system("pause");//ִ����Ϻ�ȴ� 
		}while(true);//��������Ƴ����ٴ�ִ�з��� 
		cout << __FUNCTION__ << " | �����˳�" << endl;
		businlog_warn("%s | �ػ������˳�", __FUNCTION__);
		return 0; 
	}
	catch (std::exception& e)
	{
		cout << "�����쳣��ԭ��" << e.what() << endl;
		businlog_error("%s | �ػ����̷����쳣������ԭ��%s.", __FUNCTION__, e.what());
		return -1;
	}
}