
// Carve_Control_ServiceDlg.cpp : ʵ���ļ�
//

//#include "afxdialogex.h"
#ifdef win32
	#include <SDKDDKVer.h>
#endif

#include "source/busin_log.h"
#include "source/CCarve.h"
#include "utils/msp_errors.h"
#include "source/carve_common_lib.h"
#include "source/http_server/server.hpp"
#include "source/http_server/singleton_server.h"
#include <boost_common.h>
#include <json/json.h>
#include "source/carve_manager.h"
#include <boost/property_tree/ptree.hpp>  
#include <boost/property_tree/ini_parser.hpp>

//#pragma comment( linker, "/subsystem:windows /entry:mainCRTStartup" )



// CCarve_Control_ServiceDlg ��Ϣ�������

int main()
{

	// TODO: �ڴ���Ӷ���ĳ�ʼ������
	//TODO::��ʱ��ôд�ģ���ʼ��ʧ�ܣ�����Ӧ�ñ����˳���
	//Ĭ����־���ã�����μ�����Log_Cfg_T
	string str_log_path = "../log/emcs.log";
	businlog_cfg default_cfg(str_log_path.c_str(), "Network Engraving Machine Logging");
	default_cfg.level(7);
	string str_cfg_file_path = "./emcs.cfg";
	//�������ļ������ʧ�ܣ���ʹ��Ĭ����־����
	int ret = businlog_open(default_cfg, str_cfg_file_path.c_str());
	if (ret)
	{
		businlog_error("%s | open log file:%s error, ret:%d", __FUNCTION__, str_log_path.c_str(), ret);
		return false;
	}
	//��ʼ����
	string str_err_reason;
	bool bSuccess = CCarve_Common_Lib_Tool::instance()->init(str_err_reason);
	businlog_error_return(bSuccess, ("%s | fail to init CCarve_Common_Lib_Tool, reason:%s", __FUNCTION__, str_err_reason.c_str()), false);
	string str_err_reason_for_debug, str_err_reason_for_user;
#ifdef SERVER_WITH_CONTROL_LOGIC
	ret = CCarve_Manager::instance()->start_poll_carve_status(str_err_reason_for_debug, str_err_reason_for_user);
	businlog_error_return(!ret, ("%s | start_poll_carve_status failed, reason:%s", __FUNCTION__, str_err_reason_for_debug.c_str()), false);
#endif
	//��������ģ�飺�����߳��Լ����˿�
	//�������ļ��ж�ȡ����ip��port
	string str_local_ip;	//����ip
	string str_port;		//�����˿�
	size_t nThread_num = 5;
	boost::property_tree::ptree pt_config, pt_section;
	try
	{
		read_ini(str_cfg_file_path, pt_config);
		string str_section_name = "network";
		pt_section = pt_config.get_child(str_section_name); //ʧ��ʱ���׳��쳣
		str_local_ip = pt_section.get<string>("ip");
		str_port = pt_section.get<string>("port");
		nThread_num = pt_section.get<int>("receiver_num", 5);
	}
	catch (std::exception& e)
	{
		businlog_error("%s | open config file:%s error, err reason:%s", __FUNCTION__, str_cfg_file_path.c_str(), e.what());
		return false;
	}
	if (str_local_ip.empty())
	{
		businlog_error("%s | fail to read ip from config file:%s.", __FUNCTION__, str_cfg_file_path.c_str());
		return false;
	}
	if (str_port.empty())
	{
		businlog_error("%s | fail to read port from config file:%s.", __FUNCTION__, str_cfg_file_path.c_str());
		return false;
	}

	businlog_info("%s | start:ip[%s],port[%s]", __FUNCTION__, str_local_ip.c_str(), str_port.c_str());
	ret = singleton_default<CSingleton_Server>::instance().start(
		str_local_ip, str_port, nThread_num, str_err_reason);

	businlog_error_return(!ret, ("%s | fail to start Server, reason:%s", __FUNCTION__, str_err_reason.c_str()), false);

	while (1)
	{
		sleep(10);
		businlog_error("%s","carve control service sleep 10s");
	}

	return true;  // ���ǽ��������õ��ؼ������򷵻� true
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

