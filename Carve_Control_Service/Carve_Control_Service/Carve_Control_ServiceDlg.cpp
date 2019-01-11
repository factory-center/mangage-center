
// Carve_Control_ServiceDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "Carve_Control_Service.h"
#include "Carve_Control_ServiceDlg.h"
#include "afxdialogex.h"
#include "../source/busin_log.h"
#include "../source/CCarve.h"
#include "utils/msp_errors.h"
#include "../source/carve_common_lib.h"
#include "../source/http_server/server.hpp"
#include "../source/http_server/singleton_server.h"
#include <boost_common.h>
#include <json/json.h>
#include "../source/carve_manager.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif
//test begin
//连接设备
unsigned int nConn_idx = 0;
const string str_ip = "192.168.101.212";
boost::shared_ptr<CCarve> carve_ptr;
const string str_nc_file_path = "F:\\GitHub\\mangage-center\\Carve_Control_Service\\Win32\\Debug\\1224.nc";
//test end
// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CCarve_Control_ServiceDlg 对话框



CCarve_Control_ServiceDlg::CCarve_Control_ServiceDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CCarve_Control_ServiceDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CCarve_Control_ServiceDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CCarve_Control_ServiceDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// CCarve_Control_ServiceDlg 消息处理程序

BOOL CCarve_Control_ServiceDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	ShowWindow(SW_MAXIMIZE);

	// TODO: 在此添加额外的初始化代码
	//TODO::临时这么写的，初始化失败，进程应该报错退出的
	//默认日志配置，具体参见类型Log_Cfg_T
	string str_log_path = "../log/emcs.log";
	businlog_cfg default_cfg(str_log_path.c_str(), "Network Engraving Machine Logging");
	default_cfg.level(7);
	//打开配置文件，如果失败，则使用默认日志配置
	int ret = businlog_open(default_cfg, "emcs.cfg");
	if (ret)
	{
		businlog_error("%s | open log file:%s error, ret:%d", __FUNCTION__, str_log_path.c_str(), ret);
		return false;
	}
	//初始化库
	string str_err_reason;
	bool bSuccess = CCarve_Common_Lib_Tool::instance()->init(str_err_reason);
	businlog_error_return(bSuccess, ("%s | fail to init CCarve_Common_Lib_Tool, reason:%s", __FUNCTION__, str_err_reason.c_str()), FALSE);
	string str_err_reason_for_debug, str_err_reason_for_user;
	ret = CCarve_Manager::instance()->start_poll_carve_status(str_err_reason_for_debug, str_err_reason_for_user);
	businlog_error_return(!ret, ("%s | start_poll_carve_status failed, reason:%s", __FUNCTION__, str_err_reason_for_debug.c_str()), false);
	//启动网络模块：创建线程以监听端口
	//TODO::后面放在其他地方并且设置好ip:port，目前放在这里并写死
	string str_local_ip  = "192.168.101.21";
	string str_port = "2350";
	size_t nThread_num = 5;

    ret = singleton_default<CSingleton_Server>::instance().start(
		str_local_ip, str_port, nThread_num, str_err_reason);
	businlog_error_return(!ret, ("%s | fail to start Server, reason:%s", __FUNCTION__, str_err_reason.c_str()), FALSE);
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CCarve_Control_ServiceDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CCarve_Control_ServiceDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CCarve_Control_ServiceDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}
/*
// test begin
//返回值 0：success; 非0：错误码
int test_connect()
{
	string str_err_reason_for_debug;
	string str_err_reason_for_user;
	int ret = 0;
	ret = carve_ptr->acquire_resource(str_err_reason_for_debug, str_err_reason_for_user);
	businlog_error_return(0 == ret, ("%s | fail to acquire resource, ip:%s, reason:%s"
		, __FUNCTION__, str_ip.c_str(), str_err_reason_for_debug.c_str()), ret);
	ret = carve_ptr->connect(str_err_reason_for_debug, str_err_reason_for_user);
	businlog_error_return(0 == ret, ("%s | fail to connect ip:%s, reason:%s"
		, __FUNCTION__, str_ip.c_str(), str_err_reason_for_debug.c_str()), ret);
	//更新设备为继续状态
	ret = carve_ptr->set_continue_status(0, 1000, str_err_reason_for_debug, str_err_reason_for_user);
	businlog_error_return(0 == ret, ("%s | fail to  set_continue_status, ip:%s, reason:%s"
		, __FUNCTION__, str_ip.c_str(), str_err_reason_for_debug.c_str()), ret);
	//重置设备
	carve_ptr->reset(1000, str_err_reason_for_debug, str_err_reason_for_user);
	businlog_error_return(0 == ret, ("%s | fail to  reset_carve, ip:%s, reason:%s"
		, __FUNCTION__, str_ip.c_str(), str_err_reason_for_debug.c_str()), ret);
	
	return 0;
}
int test_upload()
{
	return 0;
}
int test_query_status(string& str_carve_status_description)
{
	string str_err_reason_for_debug;
	string str_err_reason_for_user;
	ECARVE_STATUS_TYPE eCarve_status = CARVE_STATUS_OFFLINE;
	int ret = carve_ptr->get_carve_status(eCarve_status, str_err_reason_for_debug, str_err_reason_for_user);
	businlog_error_return(0 == ret, ("%s | fail to get carve status, nConn:%d, reason:%s"
		, __FUNCTION__, nConn_idx, str_err_reason_for_debug.c_str()), ret);
	//获取状态码对应的描述信息
	bool bSuccess = CCarve_Common_Lib_Tool::instance()->get_carve_status_description(eCarve_status, str_carve_status_description, str_err_reason_for_debug, str_err_reason_for_user);
	businlog_error_return(bSuccess, ("%s | fail to get carve status description", __FUNCTION__), -1);
	return 0;
}
int test_disconnect()
{
	string str_err_reason_for_debug, str_err_reason_for_user;
	//断开设备
	int ret = carve_ptr->disconnect(str_err_reason_for_debug, str_err_reason_for_user);
	//注意：可能导致资源未释放就返回了
	businlog_error_return(0 == ret, ("%s | fail to disconnect, nConn:%d, reason:%s"
		, __FUNCTION__, nConn_idx, str_err_reason_for_debug.c_str()), ret);
	ret = carve_ptr->release_resource(str_err_reason_for_debug, str_err_reason_for_user);
	businlog_error_return(0 == ret, ("%s | fail to release resource, nConn:%d, reason:%s"
		, __FUNCTION__, nConn_idx, str_err_reason_for_debug.c_str()), ret);
	return 0;
}

int test_start()
{
	string str_err_reason_for_debug, str_err_reason_for_user;
	int ret = carve_ptr->start(1000, str_err_reason_for_debug, str_err_reason_for_user);
	businlog_error_return(0 == ret, ("%s | fail to start, nc path:%s, ret:%d, reason:%s"
		, __FUNCTION__, str_nc_file_path.c_str(), ret, str_err_reason_for_debug.c_str()), ret);
	return 0;
}
int test_pause()
{
	string str_err_reason_for_debug, str_err_reason_for_user;
	//暂停雕刻
	return carve_ptr->pause(1000, str_err_reason_for_debug, str_err_reason_for_user);
}
int test_stop_fast()
{
	string str_err_reason_for_debug, str_err_reason_for_user;
	//急停
	return carve_ptr->stop_fast(1000, str_err_reason_for_debug, str_err_reason_for_user);
}

int test_cancel_stop_fast()
{
	string str_err_reason_for_debug, str_err_reason_for_user;
	//取消急停
	return carve_ptr->cancel_fast_stop(1000, str_err_reason_for_debug, str_err_reason_for_user);
}
int test_delete_file()
{
	string str_err_reason_for_debug, str_err_reason_for_user;
	int ret = carve_ptr->delete_1_file(str_nc_file_path, str_err_reason_for_debug, str_err_reason_for_user);
	businlog_error_return(0 == ret, ("%s | fail to delete file:%s, nConn:%d, reason:%s"
		, __FUNCTION__, str_nc_file_path.c_str(), nConn_idx, str_err_reason_for_debug.c_str()), ret);
	return 0;
}
int test_get_line_num(int& nLine_num)
{
	string str_err_reason_for_debug, str_err_reason_for_user;
	int ret = carve_ptr->get_current_line_num(nLine_num, str_err_reason_for_debug, str_err_reason_for_user);
	businlog_error_return(0 == ret, ("%s | fail to get line num:%s, nConn:%d, reason:%s"
		, __FUNCTION__, str_nc_file_path.c_str(), nConn_idx, str_err_reason_for_debug.c_str()), ret);
	return 0;
}
//test end
*/


void CCarve_Control_ServiceDlg::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (MessageBox(L"确定要退出程序吗？", L"退出提示", MB_ICONINFORMATION | MB_YESNO) == IDNO)
		return; //注意无返回值
	//完成一些退出操作
	singleton_default<CSingleton_Server>::instance().stop();
	CDialogEx::OnClose();
}
