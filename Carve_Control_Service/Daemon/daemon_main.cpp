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
	MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK); //以对话框形式显示
	//TODO::目前没有成功将错误日志写入文件
	businlog_error("%s | err reason:%s", __FUNCTION__, (LPCTSTR)lpDisplayBuf);
	LocalFree(lpMsgBuf);
	LocalFree(lpDisplayBuf);
	businlog_warn("%s | 退出程序", __FUNCTION__);
	ExitProcess(dw); //退出程序
}

int _tmain(int argc, TCHAR *argv[]) 
{ 
	//守护进程的日志文件
	string str_log_path = "../log/daemon.log";
	//默认日志配置，具体参见类型Log_Cfg_T
 	businlog_cfg default_cfg(str_log_path.c_str(), "Daemon Logging");
	default_cfg.level(7);
	//不需要配置文件，故将其设定为空
	string str_cfg_file_path = "";
	//打开配置文件，如果失败，则使用默认日志配置。故不需要判定其返回值
	int ret = businlog_open(default_cfg, str_cfg_file_path.c_str());
	//创建目标进程
	try
	{
		STARTUPINFO si; 
		PROCESS_INFORMATION pi; //进程信息： 
		//初始化
		ZeroMemory(&si, sizeof(si)); 
		si.cb = sizeof(si); 
		ZeroMemory(&pi, sizeof(pi)); 
		do
		{ 
			//该字符串可以指定要执行的模块的完整路径和文件名，也可以指定部分名称。
			//对于部分名称，该函数使用当前驱动器和当前目录来完成规范。
			//该功能不会使用搜索路径。此参数必须包含文件扩展名; 没有默认的扩展名
			LPTSTR lp_app_name =  L"./Carve_Control_Service.exe";
			//命令行字符串(不能为此参数不能是只读内存的指针：例如const变量或文字字符串)
			//TODO::后面可以从argv中来获取命令行
			// 创建子进程，判断是否执行成功 
			if(!CreateProcess(
				lp_app_name //TODO::当从argv中获取命令行时，这里使用NULL
				, NULL //TODO::后面从 argv中获取命名行
				, NULL
				,NULL
				, FALSE
				, 0
				, NULL
				, NULL
				, &si
				, &pi)) 
			{
				cout << "创建目标进程失败" << std::endl;
				ErrorExit(TEXT("CreateProcess"));
				return -1; 
			} 
			//进程执行成功，打印进程信息 
			cout << "创建目标进程成功，进程信息：" << endl; 
			cout << "\t进程ID:" << pi.dwProcessId << endl; 
			cout << "\t线程ID:" << pi.dwThreadId << endl; 
			businlog_crit("%s | 创建目标进程成功，进程信息如下:\n\t进程ID:%d\n\t线程ID:%d"
				, __FUNCTION__, pi.dwProcessId, pi.dwProcessId);
			// 等待知道子进程退出... 
			WaitForSingleObject( pi.hProcess, INFINITE);//检测进程是否停止 
			//WaitForSingleObject()函数检查对象的状态，如果是未确定的则等待至超时 
			//子进程退出 
			cout << "目标进程已经退出..." << endl;
			businlog_error("%s | 目标进程已经退出。", __FUNCTION__);
			//关闭进程和句柄 
			CloseHandle(pi.hProcess); 
			CloseHandle(pi.hThread); 
			//system("pause");//执行完毕后等待 
		}while(true);//如果进程推出就再次执行方法 
		cout << __FUNCTION__ << " | 程序退出" << endl;
		businlog_warn("%s | 守护进程退出", __FUNCTION__);
		return 0; 
	}
	catch (std::exception& e)
	{
		cout << "发生异常，原因：" << e.what() << endl;
		businlog_error("%s | 守护进程发生异常，错误原因：%s.", __FUNCTION__, e.what());
		return -1;
	}
}