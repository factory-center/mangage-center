#include <iostream> 
#include <windows.h> 
#include <stdio.h> 
#include <tchar.h> 
#include <string>
#include <fstream>
#include <strsafe.h>
#include <atlconv.h>

using namespace std;
// Display the error message and exit the process
void ErrorExit(LPTSTR lpszFunction, ofstream& fout_log) 
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
	//TODO::��������Ϣд����־�ļ�
	LocalFree(lpMsgBuf);
	LocalFree(lpDisplayBuf);
	fout_log << __FUNCTION__ << " | �˳�����" << endl;
	ExitProcess(dw); //�˳�����
}

int _tmain(int argc, TCHAR *argv[]) 
{ 
	//�ػ����̵���־�ļ�
	string str_log_path = "../log/daemon.log";
	ofstream fout_log(str_log_path.c_str(), ofstream::app);
	if (!fout_log)
	{
		cout << "file to open log file:" << str_log_path << endl;
		return -1;
	}

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
				ErrorExit(TEXT("CreateProcess"), fout_log);
				return -1; 
			} 
			//����ִ�гɹ�����ӡ������Ϣ 
			cout << "����Ŀ����̳ɹ���������Ϣ��" << endl; 
			cout << "\t����ID:" << pi.dwProcessId << endl; 
			cout << "\t�߳�ID:" << pi.dwThreadId << endl; 
			fout_log << "����Ŀ����̳ɹ���������Ϣ����:" << endl;
			fout_log << "\t����ID:" << pi.dwProcessId << endl; 
			fout_log << "\t�߳�ID:" << pi.dwThreadId << endl; 
			// �ȴ�֪���ӽ����˳�... 
			WaitForSingleObject( pi.hProcess, INFINITE);//�������Ƿ�ֹͣ 
			//WaitForSingleObject()�����������״̬�������δȷ������ȴ�����ʱ 
			//�ӽ����˳� 
			cout << "Ŀ������Ѿ��˳�..." << endl;
			fout_log << "Ŀ������Ѿ��˳�" << endl;
			//�رս��̺;�� 
			CloseHandle(pi.hProcess); 
			CloseHandle(pi.hThread); 
			//system("pause");//ִ����Ϻ�ȴ� 
		}while(true);//��������Ƴ����ٴ�ִ�з��� 
		cout << __FUNCTION__ << " | �����˳�" << endl;
		fout_log << __FUNCTION__ << " | �����˳�" << endl;
		return 0; 
	}
	catch (std::exception& e)
	{
		cout << "�����쳣��ԭ��" << e.what() << endl;
		fout_log << "�����쳣������ԭ��" << e.what() << endl;
		return -1;
	}
}