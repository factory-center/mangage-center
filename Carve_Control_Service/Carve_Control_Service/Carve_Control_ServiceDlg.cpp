
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
#include "../source/http_server/server.hpp"
#include "../source/http_server/singleton_server.h"
#include <boost_common.h>
#include <json/json.h>
#include "../source/carve_manager.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif
//test begin
//�����豸
unsigned int nConn_idx = 0;
const string str_ip = "192.168.101.212";
boost::shared_ptr<CCarve> carve_ptr;
const string str_nc_file_path = "F:\\GitHub\\mangage-center\\Carve_Control_Service\\Win32\\Debug\\1224.nc";
//test end
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
	ON_WM_CLOSE()
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
	//TODO::��ʱ��ôд�ģ���ʼ��ʧ�ܣ�����Ӧ�ñ����˳���
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
	string str_err_reason_for_debug, str_err_reason_for_user;
	ret = CCarve_Manager::instance()->start_poll_carve_status(str_err_reason_for_debug, str_err_reason_for_user);
	businlog_error_return(!ret, ("%s | start_poll_carve_status failed, reason:%s", __FUNCTION__, str_err_reason_for_debug.c_str()), false);
	//��������ģ�飺�����߳��Լ����˿�
	//TODO::������������ط��������ú�ip:port��Ŀǰ�������ﲢд��
	string str_local_ip  = "192.168.101.21";
	string str_port = "2350";
	size_t nThread_num = 5;

    ret = singleton_default<CSingleton_Server>::instance().start(
		str_local_ip, str_port, nThread_num, str_err_reason);
	businlog_error_return(!ret, ("%s | fail to start Server, reason:%s", __FUNCTION__, str_err_reason.c_str()), FALSE);
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
/*
// test begin
//����ֵ 0��success; ��0��������
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
	//�����豸Ϊ����״̬
	ret = carve_ptr->set_continue_status(0, 1000, str_err_reason_for_debug, str_err_reason_for_user);
	businlog_error_return(0 == ret, ("%s | fail to  set_continue_status, ip:%s, reason:%s"
		, __FUNCTION__, str_ip.c_str(), str_err_reason_for_debug.c_str()), ret);
	//�����豸
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
	//��ȡ״̬���Ӧ��������Ϣ
	bool bSuccess = CCarve_Common_Lib_Tool::instance()->get_carve_status_description(eCarve_status, str_carve_status_description, str_err_reason_for_debug, str_err_reason_for_user);
	businlog_error_return(bSuccess, ("%s | fail to get carve status description", __FUNCTION__), -1);
	return 0;
}
int test_disconnect()
{
	string str_err_reason_for_debug, str_err_reason_for_user;
	//�Ͽ��豸
	int ret = carve_ptr->disconnect(str_err_reason_for_debug, str_err_reason_for_user);
	//ע�⣺���ܵ�����Դδ�ͷžͷ�����
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
	//��ͣ���
	return carve_ptr->pause(1000, str_err_reason_for_debug, str_err_reason_for_user);
}
int test_stop_fast()
{
	string str_err_reason_for_debug, str_err_reason_for_user;
	//��ͣ
	return carve_ptr->stop_fast(1000, str_err_reason_for_debug, str_err_reason_for_user);
}

int test_cancel_stop_fast()
{
	string str_err_reason_for_debug, str_err_reason_for_user;
	//ȡ����ͣ
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
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	if (MessageBox(L"ȷ��Ҫ�˳�������", L"�˳���ʾ", MB_ICONINFORMATION | MB_YESNO) == IDNO)
		return; //ע���޷���ֵ
	//���һЩ�˳�����
	singleton_default<CSingleton_Server>::instance().stop();
	CDialogEx::OnClose();
}
