#include <iostream> 
#include <windows.h> 
#include <stdio.h> 
#include <tchar.h> 
#include <string>
using namespace std; 

int _tmain(int argc, TCHAR *argv[]) 
{ 
	try
	{
		std::cout << " main enter" << std::endl;
		STARTUPINFO si; 

		PROCESS_INFORMATION pi; //������Ϣ�� 

		ZeroMemory(&si, sizeof(si)); 
		si.cb = sizeof(si); 
		ZeroMemory(&pi, sizeof(pi)); 
		do
		{ 
			//�������ַ���
			LPTSTR lpt_CommandLine = L"F:\\project\\CoreRepo\\Development\\Source\\trunk\\test_tool\\emcs_mtrec\\x64\\Release\\emcs_mtrec.exe";
			// �����ӽ��̣��ж��Ƿ�ִ�гɹ� 
			if(!CreateProcess(
				NULL
				, lpt_CommandLine
				, NULL
				,NULL
				, FALSE
				, 0
				, NULL
				, NULL
				, &si
				, &pi)) 
			{
				cout << "��������ʧ��" << std::endl;
				LPVOID lpMsgBuf = NULL;
				DWORD dw = GetLastError();
				FormatMessage(
					FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
					NULL,
					dw,
					MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
					(LPTSTR) &lpMsgBuf,
					0, NULL );
				cout << "��������ʧ��, ������" << dw  << ", ����ԭ��:" << ((LPTSTR)lpMsgBuf) << endl; 
				system("pause"); //���ڲ��� 
				LocalFree(lpMsgBuf);
				return -1; 
			} 
			//����ִ�гɹ�����ӡ������Ϣ 
			cout << "�������ӽ��̵���Ϣ��" << endl; 
			cout << "����ID pi.dwProcessID: " << pi.dwProcessId << endl; 
			cout << "�߳�ID pi.dwThreadID : " << pi.dwThreadId << endl; 
			// �ȴ�֪���ӽ����˳�... 
			WaitForSingleObject( pi.hProcess, INFINITE);//�������Ƿ�ֹͣ 
			//WaitForSingleObject()�����������״̬�������δȷ������ȴ�����ʱ 
			//�ӽ����˳� 
			cout << "�ӽ����Ѿ��˳�..." << endl; 
			//�رս��̺;�� 
			CloseHandle(pi.hProcess); 
			CloseHandle(pi.hThread); 
			//system("pause");//ִ����Ϻ�ȴ� 
		}while(true);//��������Ƴ����ٴ�ִ�з��� 
		return 0; 
	}
	catch (std::exception& e)
	{
		cout << "�����쳣��ԭ��" << e.what() << endl;
		return -1;
	}
}