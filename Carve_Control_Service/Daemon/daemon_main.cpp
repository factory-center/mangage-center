#include <iostream> 
#include <stdio.h> 
#include <tchar.h> 
#include <string>
#include <fstream>
#include <strsafe.h>
#include "../source/CSpdLog.h"
#include <utils/Resource_Manager.h>
#if (defined WIN32) || (defined WIN64)
#pragma comment(linker, "/subsystem:windows /entry:mainCRTStartup")
#endif
using namespace std;
//��LPCTSTRת��Ϊstring
int lpct2str(LPCTSTR lpsz, size_t nSrc_size, string& str_dst)
{
	//����Ŀ�껺��
	char *pDst_buffer = NULL;
	CNewBuffMngr<char> buffer_mng(pDst_buffer, nSrc_size);

	//ת��
	int nRet = WideCharToMultiByte(CP_OEMCP, 0, lpsz, -1, pDst_buffer, nSrc_size, NULL, FALSE);

	if(nRet <= 0)
	{
		cout << "ת���ַ���ʧ��,������:" << nRet << endl;
		LError("fail to WideCharToMultiByte, ret:{}", nRet);
		return -1;
	}
	//ת���ɹ�
	str_dst = string(pDst_buffer);
	return 0;
}
// Record/Display the error message and exit the process
void record_err_and_exit(LPTSTR lpszFunction) 
{ 
	// Retrieve the system error message for the last-error code
    string str_err_reason;
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
//	MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK); //�ԶԻ�����ʽ��ʾ

	int ret  = lpct2str((LPCTSTR)lpDisplayBuf, LocalSize(lpDisplayBuf) / sizeof(TCHAR), str_err_reason);
	if (ret)
	{
		LError("fail to parse LPCTSTR to String, ret:{}", ret);
	}
	//��������־д���ļ�
	LError("err reason:{}", str_err_reason.c_str());
	cout << __FUNCTION__ << " | err reason:" << str_err_reason << endl;
	LocalFree(lpMsgBuf);
	LocalFree(lpDisplayBuf);
	LWarn("�˳�����");
	ExitProcess(dw); //�˳�����
}

int main(int argc, TCHAR *argv[]) 
{ 
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
			//��������в����ǳ����ַ�������ú������ܻᵼ�·��ʳ�ͻ
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
				record_err_and_exit(TEXT("CreateProcess"));
				return -1; 
			} 
			//����ִ�гɹ�����ӡ������Ϣ 
			cout << "����Ŀ����̳ɹ���������Ϣ��" << endl; 
			cout << "\t����ID:" << pi.dwProcessId << endl; 
			cout << "\t�߳�ID:" << pi.dwThreadId << endl; 
			LCritical("����Ŀ����̳ɹ���������Ϣ����:\n\t����ID:{}\n\t�߳�ID:{}",pi.dwProcessId, pi.dwProcessId);
			// �ȴ�֪���ӽ����˳�... 
			WaitForSingleObject( pi.hProcess, INFINITE);//�������Ƿ�ֹͣ 
			//WaitForSingleObject()�����������״̬�������δȷ������ȴ�����ʱ 
			//�ӽ����˳� 
			cout << "Ŀ������Ѿ��˳�..." << endl;
			LError("Ŀ������Ѿ��˳���");
			//�رս��̺;�� 
			CloseHandle(pi.hProcess); 
			CloseHandle(pi.hThread); 
			//system("pause");//ִ����Ϻ�ȴ� 
		}while(true);//��������Ƴ����ٴ�ִ�з��� 
		cout << __FUNCTION__ << " | �����˳�" << endl;
		LWarn("�ػ������˳�");
		return 0; 
	}
	catch (std::exception& e)
	{
		cout << "�����쳣��ԭ��" << e.what() << endl;
		LError("�ػ����̷����쳣������ԭ��:{}", e.what());
		return -1;
	}
}