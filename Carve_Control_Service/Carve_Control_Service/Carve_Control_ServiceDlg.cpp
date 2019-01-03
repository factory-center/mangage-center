
// Carve_Control_ServiceDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "Carve_Control_Service.h"
#include "Carve_Control_ServiceDlg.h"
#include "afxdialogex.h"
#include "../source/busin_log.h"
#include "../source/CCarve.h"
#include "utils/msp_errors.h"
#include "../source/carve_common_lib.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
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


// CCarve_Control_ServiceDlg �Ի���



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
	ON_BN_CLICKED(IDC_UPLOADFILE, &CCarve_Control_ServiceDlg::OnBnClickedUploadfile)
	ON_BN_CLICKED(IDC_BUTTON6, &CCarve_Control_ServiceDlg::OnBnClickedButton6)
	ON_BN_CLICKED(IDC_BUTTON7, &CCarve_Control_ServiceDlg::OnBnClickedButton7)
	ON_BN_CLICKED(IDC_BUTTON8, &CCarve_Control_ServiceDlg::OnBnClickedButton8)
	ON_BN_CLICKED(IDC_BUTTON_cancel_stop_fast, &CCarve_Control_ServiceDlg::OnBnClickedButtoncancelstopfast)
END_MESSAGE_MAP()


// CCarve_Control_ServiceDlg ��Ϣ�������

BOOL CCarve_Control_ServiceDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
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

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	ShowWindow(SW_MAXIMIZE);

	// TODO: �ڴ���Ӷ���ĳ�ʼ������

	//Ĭ����־���ã�����μ�����Log_Cfg_T
	string str_log_path = "../log/emcs.log";
	businlog_cfg default_cfg(str_log_path.c_str(), "Network Engraving Machine Logging");
	default_cfg.level(7);
	//�������ļ������ʧ�ܣ���ʹ��Ĭ����־����
	int ret = businlog_open(default_cfg, "emcs.cfg");
	if (ret)
	{
		businlog_error("%s | open log file:%s error, ret:%d", __FUNCTION__, str_log_path.c_str(), ret);
		return false;
	}
	//��ʼ����
	string str_err_reason;
	bool bSuccess = CCarve_Common_Lib_Tool::instance()->init(str_err_reason);
	businlog_error_return(bSuccess, ("%s | fail to init CCarve_Common_Lib_Tool, reason:%s", __FUNCTION__, str_err_reason.c_str()), FALSE);
	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
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

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CCarve_Control_ServiceDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CCarve_Control_ServiceDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}
// test begine
//�����豸
unsigned int nConn_idx = 0;
const string str_ip = "192.168.101.212";
CCarve carve_obj(nConn_idx, str_ip);
const string str_nc_file_path = "F:\\GitHub\\mangage-center\\Carve_Control_Service\\Win32\\Debug\\1224.nc";
//����ֵ 0��success; ��0��������
int test_connect()
{
	string str_kernel_err_reason;

	int ret = carve_obj.connect(str_kernel_err_reason);
	businlog_error_return(0 == ret, ("%s | fail to connect ip:%s, reason:%s"
		, __FUNCTION__, str_ip.c_str(), str_kernel_err_reason.c_str()), ret);
	//�����豸Ϊ����״̬
	ret = carve_obj.set_continue_status(0, 1000, str_kernel_err_reason);
	businlog_error_return(0 == ret, ("%s | fail to  set_continue_status, ip:%s, reason:%s"
		, __FUNCTION__, str_ip.c_str(), str_kernel_err_reason.c_str()), ret);
	//�����豸
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
int test_query_status(string& str_carve_status_description)
{
	string str_kernel_err_reason;
	ECARVE_STATUS_TYPE eCarve_status = CARVE_STATUS_OFFLINE;
	int ret = carve_obj.get_carve_status(eCarve_status, str_kernel_err_reason);
	businlog_error_return(0 == ret, ("%s | fail to get carve status, nConn:%d, reason:%s"
		, __FUNCTION__, nConn_idx, str_kernel_err_reason.c_str()), ret);
	//��ȡ״̬���Ӧ��������Ϣ
	bool bSuccess = CCarve_Common_Lib_Tool::instance()->get_carve_status_description(eCarve_status, str_carve_status_description, str_kernel_err_reason);
	businlog_error_return(bSuccess, ("%s | fail to get carve status description", __FUNCTION__), -1);
	return 0;
}
int test_disconnect()
{
	string str_kernel_err_reason;
	//�Ͽ��豸
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
	//��ͣ���
	return carve_obj.pause(1000, str_kernel_err_reason);
}
int test_stop_fast()
{
	string str_kernel_err_reason;
	//��ͣ
	return carve_obj.stop_fast(1000, str_kernel_err_reason);
}

int test_cancel_stop_fast()
{
	string str_kernel_err_reason;
	//ȡ����ͣ
	return carve_obj.cancel_fast_stop(1000, str_kernel_err_reason);
}
int test_delete_file()
{
	string str_kernel_err_reason;
	int ret = carve_obj.delete_1_file(str_nc_file_path, str_kernel_err_reason);
	businlog_error_return(0 == ret, ("%s | fail to delete file:%s, nConn:%d, reason:%s"
		, __FUNCTION__, str_nc_file_path.c_str(), nConn_idx, str_kernel_err_reason.c_str()), ret);
	return 0;
}

//test end
void CCarve_Control_ServiceDlg::OnBnClickedButton1()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
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
	// TODO: �ڴ���ӿؼ�֪ͨ����������
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
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	int ret = test_pause();
	if (ret)
	{
		MessageBox(_T("pause failed"));
	}
	else
	{
		MessageBox(_T("pause successfully"));
	}
}


void CCarve_Control_ServiceDlg::OnBnClickedButton4()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
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
	// TODO: �ڴ���ӿؼ�֪ͨ����������
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
	// TODO: �ڴ���ӿؼ�֪ͨ����������
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


void CCarve_Control_ServiceDlg::OnBnClickedButton6()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	string str_carve_status_description;
	int ret = test_query_status(str_carve_status_description);
	if (ret)
	{
		MessageBox(_T("query carve status  failed"));
	}
	else
	{
		MessageBox(_T("query carve status successfully, carve status description:") + CString(str_carve_status_description.c_str()));
	}
}


void CCarve_Control_ServiceDlg::OnBnClickedButton7()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	int ret = test_stop_fast();
	if (ret)
	{
		MessageBox(_T("fast stop failed"));
	}
	else
	{
		MessageBox(_T("fast stop successfully"));
	}
}


void CCarve_Control_ServiceDlg::OnBnClickedButton8()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	int ret = test_delete_file();
	if (ret)
	{
		MessageBox(_T("delete file failed"));
	}
	else
	{
		MessageBox(_T("delete file successfully"));
	}
}


void CCarve_Control_ServiceDlg::OnBnClickedButtoncancelstopfast()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	int ret = test_cancel_stop_fast();
	if (ret)
	{
		MessageBox(_T("cancel fast stop failed"));
	}
	else
	{
		MessageBox(_T("cancel fast stop successfully"));
	}
}
