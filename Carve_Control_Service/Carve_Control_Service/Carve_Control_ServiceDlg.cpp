
// Carve_Control_ServiceDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "Carve_Control_Service.h"
#include "Carve_Control_ServiceDlg.h"
#include "afxdialogex.h"
#include "../source/CBaoyuan_Lib_Tool.h"
#include "../source/busin_log.h"
#include "../source/CCarve.h"
#include "utils/msp_errors.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


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
	ON_BN_CLICKED(IDC_BUTTON1, &CCarve_Control_ServiceDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CCarve_Control_ServiceDlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON3, &CCarve_Control_ServiceDlg::OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BUTTON4, &CCarve_Control_ServiceDlg::OnBnClickedButton4)
	ON_BN_CLICKED(IDC_BUTTON5, &CCarve_Control_ServiceDlg::OnBnClickedButton5)
	ON_BN_CLICKED(IDC_UPLOADFILE, &CCarve_Control_ServiceDlg::OnBnClickedUploadfile)
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
// test begine
//连接设备
unsigned int nConn_idx = 0;
const string str_ip = "192.168.101.212";
CCarve carve_obj(nConn_idx, str_ip);
const string str_nc_file_path = "F:\\GitHub\\mangage-center\\Carve_Control_Service\\Win32\\Debug\\wenzi_test.nc";
//返回值 0：success; 非0：错误码
int test_connect()
{

	//初始化
	int nMakeId = 1111;
	string str_key = "79A08F845B1BCADC25EF0B396062EE91C06EB78EFFE16A7A";
	bool bSuccess = CBaoyuan_Lib::instance()->init(nMakeId, str_key, 100, 1000000);
	businlog_error_return(bSuccess, ("%s | fail to init baoyuan lib, makeId:%d, key:%s", __FUNCTION__, nMakeId, str_key.c_str()), -1);

	string str_kernel_err_reason;
	
	int ret = carve_obj.connect(str_kernel_err_reason);
	businlog_error_return(0 == ret, ("%s | fail to connect ip:%s, reason:%s"
		, __FUNCTION__, str_ip.c_str(), str_kernel_err_reason.c_str()), ret);
	//更新设备为继续状态
	ret = carve_obj.set_continue_status(0, 1000, str_kernel_err_reason);
	businlog_error_return(0 == ret, ("%s | fail to  set_continue_status, ip:%s, reason:%s"
		, __FUNCTION__, str_ip.c_str(), str_kernel_err_reason.c_str()), ret);
	//重置设备
	carve_obj.reset(1000, str_kernel_err_reason);
	businlog_error_return(0 == ret, ("%s | fail to  reset_carve, ip:%s, reason:%s"
		, __FUNCTION__, str_ip.c_str(), str_kernel_err_reason.c_str()), ret);
	
	return 0;
}
int test_upload()
{
	string str_kernel_err_reason;
	int ret = carve_obj.upload_1_file(str_nc_file_path, str_kernel_err_reason);
	businlog_error_return(0 == ret, ("%s | fail to upload file:%s, nConn:%d, reason:%s"
		, __FUNCTION__, str_nc_file_path.c_str(), nConn_idx, str_kernel_err_reason.c_str()), ret);
	return 0;
}
int test_disconnect()
{
	string str_kernel_err_reason;
	//断开设备
	int ret = carve_obj.disconnect(str_kernel_err_reason);
	businlog_error_return(0 == ret, ("%s | fail to disconnect, nConn:%d, reason:%s"
		, __FUNCTION__, nConn_idx, str_kernel_err_reason.c_str()), ret);
	return 0;
}

int test_start()
{
	string str_kernel_err_reason;
	int ret = carve_obj.start(str_nc_file_path, 1000, str_kernel_err_reason);
	businlog_error_return(0 == ret, ("%s | fail to start, nc path:%s, ret:%d, reason:%s"
		, __FUNCTION__, str_nc_file_path.c_str(), ret, str_kernel_err_reason.c_str()), ret);
	return 0;
}
int test_pause()
{
	string str_kernel_err_reason;
	//暂停雕刻
	return carve_obj.pause(1000, str_kernel_err_reason);
}
//test end
void CCarve_Control_ServiceDlg::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码
	int ret = test_connect();
	if (ret)
	{
		MessageBox(_T("connect failed"));
	}
	else
	{
		MessageBox(_T("connect successfully"));
	}
}


void CCarve_Control_ServiceDlg::OnBnClickedButton2()
{
	// TODO: 在此添加控件通知处理程序代码
	int ret = test_disconnect();
	if (ret)
	{
		MessageBox(_T("disconnect failed"));
	}
	else
	{
		MessageBox(_T("disconnect successfully"));
	}
}


void CCarve_Control_ServiceDlg::OnBnClickedButton3()
{
	// TODO: 在此添加控件通知处理程序代码
}


void CCarve_Control_ServiceDlg::OnBnClickedButton4()
{
	// TODO: 在此添加控件通知处理程序代码
	int ret = test_start();
	if (ret)
	{
		MessageBox(_T("start failed"));
	}
	else
	{
		MessageBox(_T("start successfully"));
	}
}


void CCarve_Control_ServiceDlg::OnBnClickedButton5()
{
	// TODO: 在此添加控件通知处理程序代码
	int ret = test_upload();
	if (ret)
	{
		MessageBox(_T("upload failed"));
	}
	else
	{
		MessageBox(_T("upload successfully"));
	}
}


void CCarve_Control_ServiceDlg::OnBnClickedUploadfile()
{
	// TODO: 在此添加控件通知处理程序代码
	MessageBox(_T("CCarve_Control_ServiceDlg::OnBnClickedUploadfile"));
	int ret = test_upload();
	if (ret)
	{
		MessageBox(_T("upload file failed"));
	}
	else
	{
		MessageBox(_T("upload file successfully"));
	}

}
