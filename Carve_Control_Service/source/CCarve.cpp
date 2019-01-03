/******************************************************************
* Copyright (c) 2016-2017,HIIRI Inc.
* All rights reserved.
* @file: CCarve.cpp
* @brief: 简短说明文件功能、用途 (Comment)。
* @author:	minglu2
* @version: 1.0
* @date: 2018/12/20
* 
* @see
* 
* <b>版本记录：</b><br>
* <table>
*  <tr> <th>版本	<th>日期		<th>作者	<th>备注 </tr>
*  <tr> <td>1.0	    <td>2018/12/20	<td>minglu	<td>Create head file </tr>
* </table>
*****************************************************************/
#include "CCarve.h"
#include "CBaoyuan_Lib_Tool.h"
#include "busin_log.h"
#include "utils/msp_errors.h"
#include <json/json.h>
#ifdef _WINDOWS
#define __CLASS_FUNCTION__ ((std::string(__FUNCTION__)).c_str()) 
#else
#define __CLASS_FUNCTION__ ((std::string("CCarve::") + std::string(__FUNCTION__)).c_str()) 
#endif


int CCarve::connect(string& str_kernel_err_reason)
{
	boost::mutex::scoped_lock guard(m_mutex_for_cmd);
	if (m_bConnected)
	{
		businlog_warn("%s | carve ip:%s is connected.", __CLASS_FUNCTION__, m_str_ip.c_str());
		return MSP_SUCCESS;
	}
	//构造连接参数
	Json::Value json_conn_value;
	json_conn_value[ms_str_factory_type_key] = m_eFactory_type;
	json_conn_value[ms_str_carve_type_key] = m_str_carve_type;
	if (CARVE_FACTORY_TYPE_BAOYUAN == m_eFactory_type)
	{
		//宝元库所需要的参数
		json_conn_value[ms_str_conn_idx_key] = m_nConn_idx;
		json_conn_value[ms_str_ip_key] = m_str_ip;
	}
	else if(false)
	{
		//TODO::其他厂商
	}
	else
	{
		businlog_error_return_err_reason(false, __CLASS_FUNCTION__ << " | factory_type is invalid:"
			<< m_eFactory_type, str_kernel_err_reason, MSP_ERROR_NOT_SUPPORT);
	}
	
//    bool bSuccess = CBaoyuan_Lib::instance()->create_connection(m_nConn_idx, m_str_ip, str_kernel_err_reason);
	bool bSuccess = CCarve_Common_Lib_Tool::instance()->connect(json_conn_value, str_kernel_err_reason);
	businlog_error_return(bSuccess, ("%s | fail to connect carve, param in json:%s, reason:%s"
		, __CLASS_FUNCTION__, json_conn_value.toStyledString().c_str(), str_kernel_err_reason.c_str()), MSP_ERROR_FAIL);
	m_bConnected = true;
	return MSP_SUCCESS;
}

int CCarve::disconnect(string& str_kernel_err_reason)
{
	boost::mutex::scoped_lock guard(m_mutex_for_cmd);
	//判定当前状态
	businlog_error_return_err_reason(true == m_bConnected, __CLASS_FUNCTION__ << " | carve ip:" << m_str_ip 
		<<" is not connected", str_kernel_err_reason, MSP_ERROR_FAIL);
	//构造参数
	Json::Value json_conn_value;
	json_conn_value[ms_str_factory_type_key] = m_eFactory_type;
	json_conn_value[ms_str_carve_type_key] = m_str_carve_type;
	if (CARVE_FACTORY_TYPE_BAOYUAN == m_eFactory_type)
	{
		//宝元库所需要的参数
		json_conn_value[ms_str_conn_idx_key] = m_nConn_idx;
	}
	else if(false)
	{
		//TODO::其他厂商
	}
	else
	{
		businlog_error_return_err_reason(false, __CLASS_FUNCTION__ << " | factory_type is invalid:"
			<< m_eFactory_type, str_kernel_err_reason, MSP_ERROR_NOT_SUPPORT);
	}

	bool bSuccess = CCarve_Common_Lib_Tool::instance()->disconnect(json_conn_value, str_kernel_err_reason);
	businlog_error_return(bSuccess, ("%s | fail to disconnect carve, params in json:%s, reason:%s"
		, __CLASS_FUNCTION__, json_conn_value.toStyledString().c_str(), str_kernel_err_reason.c_str()), MSP_ERROR_FAIL);
	//成功断开，则更新状态
	m_bConnected = false;
	return MSP_SUCCESS;
}

int CCarve::set_continue_status(unsigned char nStatus, unsigned short nMax_wait_time, string& str_kernel_err_reason)
{
	boost::mutex::scoped_lock guard(m_mutex_for_cmd);
	businlog_error_return_err_reason(true == m_bConnected, __CLASS_FUNCTION__ << " | carve ip:" << m_str_ip 
		<<" is not connected", str_kernel_err_reason, MSP_ERROR_INVALID_OPERATION);

	//构造参数
	Json::Value json_conn_value;
	json_conn_value[ms_str_factory_type_key] = m_eFactory_type;
	json_conn_value[ms_str_carve_type_key] = m_str_carve_type;
	if (CARVE_FACTORY_TYPE_BAOYUAN == m_eFactory_type)
	{
		//宝元库所需要的参数
		json_conn_value[ms_str_conn_idx_key] = m_nConn_idx;
		json_conn_value[ms_str_status_key] = nStatus;
		json_conn_value[ms_str_max_wait_time_key] =  nMax_wait_time;
	}
	else if(false)
	{
		//TODO::其他厂商
	}
	else
	{
		businlog_error_return_err_reason(false, __CLASS_FUNCTION__ << " | factory_type is invalid:"
			<< m_eFactory_type, str_kernel_err_reason, MSP_ERROR_NOT_SUPPORT);
	}

	bool bSuccess = CCarve_Common_Lib_Tool::instance()->set_continue_status(json_conn_value, str_kernel_err_reason);
//	bool bSuccess = CBaoyuan_Lib::instance()->set_continue_status(m_nConn_idx, nStatus, nMax_wait_time, str_kernel_err_reason);
	businlog_error_return(bSuccess, ("%s | fail to set continue status, ip:%s, json info:%s, reason:%s"
		, __CLASS_FUNCTION__, m_str_ip.c_str(), json_conn_value.toStyledString().c_str(), str_kernel_err_reason.c_str()), MSP_ERROR_FAIL);
	return MSP_SUCCESS;
}

int CCarve::reset(unsigned short nMax_wait_time, string& str_kernel_err_reason)
{
	boost::mutex::scoped_lock guard(m_mutex_for_cmd);
	businlog_error_return_err_reason(true == m_bConnected, __CLASS_FUNCTION__ << " | carve ip:" << m_str_ip 
		<<" is not connected", str_kernel_err_reason, MSP_ERROR_INVALID_OPERATION);

	//构造参数
	Json::Value json_conn_value;
	json_conn_value[ms_str_factory_type_key] = m_eFactory_type;
	json_conn_value[ms_str_carve_type_key] = m_str_carve_type;
	if (CARVE_FACTORY_TYPE_BAOYUAN == m_eFactory_type)
	{
		//宝元库所需要的参数
		json_conn_value[ms_str_conn_idx_key] = m_nConn_idx;
		json_conn_value[ms_str_max_wait_time_key] =  nMax_wait_time;
	}
	else if(false)
	{
		//TODO::其他厂商
	}
	else
	{
		businlog_error_return_err_reason(false, __CLASS_FUNCTION__ << " | factory_type is invalid:"
			<< m_eFactory_type, str_kernel_err_reason, MSP_ERROR_NOT_SUPPORT);
	}


	bool bSuccess = CCarve_Common_Lib_Tool::instance()->reset_carve(json_conn_value, str_kernel_err_reason);
	businlog_error_return(bSuccess, ("%s | fail to reset carve, ip:%s, json info:%s, reason:%s"
		, __CLASS_FUNCTION__, m_str_ip.c_str(), json_conn_value.toStyledString().c_str(), str_kernel_err_reason.c_str()), MSP_ERROR_FAIL);
	return MSP_SUCCESS;
}


int CCarve::start(const string& str_nc_file_path, unsigned short nMax_wait_time, string& str_kernel_err_reason)
{
	//TODO::额外操作
	//使雕刻机开始雕刻
	boost::mutex::scoped_lock guard(m_mutex_for_cmd);
	businlog_error_return_err_reason(true == m_bConnected, __CLASS_FUNCTION__ << " | carve ip:" << m_str_ip 
		<<" is not connected", str_kernel_err_reason, MSP_ERROR_INVALID_OPERATION);

	//构造参数
	Json::Value json_conn_value;
	json_conn_value[ms_str_factory_type_key] = m_eFactory_type;
	json_conn_value[ms_str_carve_type_key] = m_str_carve_type;
	if (CARVE_FACTORY_TYPE_BAOYUAN == m_eFactory_type)
	{
		//宝元库所需要的参数
		json_conn_value[ms_str_conn_idx_key] = m_nConn_idx;
		json_conn_value[ms_str_file_path_key] =  str_nc_file_path;
		json_conn_value[ms_str_max_wait_time_key] =  nMax_wait_time;
	}
	else if(false)
	{
		//TODO::其他厂商
	}
	else
	{
		businlog_error_return_err_reason(false, __CLASS_FUNCTION__ << " | factory_type is invalid:"
			<< m_eFactory_type, str_kernel_err_reason, MSP_ERROR_NOT_SUPPORT);
	}

	bool bSuccess = CCarve_Common_Lib_Tool::instance()->start(json_conn_value, str_kernel_err_reason);
	businlog_error_return(bSuccess, ("%s | fail to start carve to engrave, ip:%s, reason:%s"
		, __CLASS_FUNCTION__, m_str_ip.c_str(), str_kernel_err_reason.c_str()), MSP_ERROR_FAIL);
	return MSP_SUCCESS;
}

int CCarve::pause(unsigned short nMax_wait_time, string& str_kernel_err_reason)
{
	boost::mutex::scoped_lock guard(m_mutex_for_cmd);
	businlog_error_return_err_reason(true == m_bConnected, __CLASS_FUNCTION__ << " | carve ip:" << m_str_ip 
		<<" is not connected", str_kernel_err_reason, MSP_ERROR_INVALID_OPERATION);

	//构造参数
	Json::Value json_conn_value;
	json_conn_value[ms_str_factory_type_key] = m_eFactory_type;
	json_conn_value[ms_str_carve_type_key] = m_str_carve_type;
	if (CARVE_FACTORY_TYPE_BAOYUAN == m_eFactory_type)
	{
		//宝元库所需要的参数
		json_conn_value[ms_str_conn_idx_key] = m_nConn_idx;
		json_conn_value[ms_str_max_wait_time_key] =  nMax_wait_time;
	}
	else if(false)
	{
		//TODO::其他厂商
	}
	else
	{
		businlog_error_return_err_reason(false, __CLASS_FUNCTION__ << " | factory_type is invalid:"
			<< m_eFactory_type, str_kernel_err_reason, MSP_ERROR_NOT_SUPPORT);
	}

	bool bSuccess = CCarve_Common_Lib_Tool::instance()->pause(json_conn_value, str_kernel_err_reason);
	businlog_error_return(bSuccess, ("%s | fail to pause carve, ip:%s, json info:%s, reason:%s"
		, __CLASS_FUNCTION__, m_str_ip.c_str(), json_conn_value.toStyledString().c_str(), str_kernel_err_reason.c_str()), MSP_ERROR_FAIL);
	return MSP_SUCCESS;
}

int CCarve::upload_1_file(const string& str_file_path, string& str_kernel_err_reason)
{
	businlog_tracer_perf(CCarve::upload_1_file);
	boost::mutex::scoped_lock guard(m_mutex_for_cmd);
	businlog_error_return_err_reason(true == m_bConnected, __CLASS_FUNCTION__ << " | carve ip:" << m_str_ip 
		<<" is not connected", str_kernel_err_reason, MSP_ERROR_INVALID_OPERATION);
	
	//构造参数
	Json::Value json_conn_value;
	json_conn_value[ms_str_factory_type_key] = m_eFactory_type;
	json_conn_value[ms_str_carve_type_key] = m_str_carve_type;
	if (CARVE_FACTORY_TYPE_BAOYUAN == m_eFactory_type)
	{
		//宝元库所需要的参数
		json_conn_value[ms_str_conn_idx_key] = m_nConn_idx;
		json_conn_value[ms_str_file_path_key] = str_file_path;
	}
	else if(false)
	{
		//TODO::其他厂商
	}
	else
	{
		businlog_error_return_err_reason(false, __CLASS_FUNCTION__ << " | factory_type is invalid:"
			<< m_eFactory_type, str_kernel_err_reason, MSP_ERROR_NOT_SUPPORT);
	}

	//上传文件
	bool bSuccess = CCarve_Common_Lib_Tool::instance()->upload_1file(json_conn_value, str_kernel_err_reason);
	businlog_error_return(bSuccess, ("%s | fail to upload file, carve ip:%s, file path:%s, json info:%s, reason:%s"
		, __CLASS_FUNCTION__, m_str_ip.c_str(), str_file_path.c_str(), json_conn_value.toStyledString().c_str()
		, str_kernel_err_reason.c_str()), MSP_ERROR_FAIL);
	return MSP_SUCCESS;
}

int CCarve::get_carve_status(ECARVE_STATUS_TYPE& eCarve_common_status, string& str_kernel_err_reason)
{
	boost::mutex::scoped_lock guard(m_mutex_for_cmd);
	if (false == m_bConnected)
	{//未连接
		//设置为离线状态
		eCarve_common_status = CARVE_STATUS_OFFLINE;
		return MSP_SUCCESS;
	}
	//构造连接参数
	Json::Value json_conn_value;
	json_conn_value[ms_str_factory_type_key] = m_eFactory_type;
	json_conn_value[ms_str_carve_type_key] = m_str_carve_type;
	if (CARVE_FACTORY_TYPE_BAOYUAN == m_eFactory_type)
	{
		//宝元库所需要的参数
		json_conn_value[ms_str_conn_idx_key] = m_nConn_idx;
	}
	else if(false)
	{
		//TODO::其他厂商
	}
	else
	{
		businlog_error_return_err_reason(false, __CLASS_FUNCTION__ << " | factory_type is invalid:"
			<< m_eFactory_type, str_kernel_err_reason, MSP_ERROR_NOT_SUPPORT);
	}
	bool bSuccess = CCarve_Common_Lib_Tool::instance()->get_carve_status(json_conn_value, eCarve_common_status, str_kernel_err_reason);
	businlog_error_return(bSuccess, ("%s | fail to get baoyuan carve status, json:%s, reason:%s"
		, __CLASS_FUNCTION__, json_conn_value.toStyledString().c_str(), str_kernel_err_reason.c_str()), MSP_ERROR_FAIL);
	//此时成功获取雕刻机状态
	
	return MSP_SUCCESS;
}

int CCarve::stop_fast(unsigned short nMax_wait_time, string& str_kernel_err_reason)
{
	boost::mutex::scoped_lock guard(m_mutex_for_cmd);
	businlog_error_return_err_reason(true == m_bConnected, __CLASS_FUNCTION__ << " | carve ip:" << m_str_ip 
		<<" is not connected", str_kernel_err_reason, MSP_ERROR_INVALID_OPERATION);

	//构造参数
	Json::Value json_conn_value;
	json_conn_value[ms_str_factory_type_key] = m_eFactory_type;
	json_conn_value[ms_str_carve_type_key] = m_str_carve_type;
	if (CARVE_FACTORY_TYPE_BAOYUAN == m_eFactory_type)
	{
		//宝元库所需要的参数
		json_conn_value[ms_str_conn_idx_key] = m_nConn_idx;
		json_conn_value[ms_str_max_wait_time_key] =  nMax_wait_time;
	}
	else if(false)
	{
		//TODO::其他厂商
	}
	else
	{
		businlog_error_return_err_reason(false, __CLASS_FUNCTION__ << " | factory_type is invalid:"
			<< m_eFactory_type, str_kernel_err_reason, MSP_ERROR_NOT_SUPPORT);
	}

	bool bSuccess = CCarve_Common_Lib_Tool::instance()->stop_fast(json_conn_value, str_kernel_err_reason);
	businlog_error_return(bSuccess, ("%s | fail to fast stop carve, ip:%s, json info:%s, reason:%s"
		, __CLASS_FUNCTION__, m_str_ip.c_str(), json_conn_value.toStyledString().c_str(), str_kernel_err_reason.c_str()), MSP_ERROR_FAIL);
	return MSP_SUCCESS;
}

int CCarve::cancel_fast_stop(unsigned short nMax_wait_time, string& str_kernel_err_reason)
{
	businlog_tracer_perf(CCarve::cancel_fast_stop);
	boost::mutex::scoped_lock guard(m_mutex_for_cmd);
	businlog_error_return_err_reason(true == m_bConnected, __CLASS_FUNCTION__ << " | carve ip:" << m_str_ip 
		<<" is not connected", str_kernel_err_reason, MSP_ERROR_INVALID_OPERATION);

	//构造参数
	Json::Value json_conn_value;
	json_conn_value[ms_str_factory_type_key] = m_eFactory_type;
	json_conn_value[ms_str_carve_type_key] = m_str_carve_type;
	if (CARVE_FACTORY_TYPE_BAOYUAN == m_eFactory_type)
	{
		//宝元库所需要的参数
		json_conn_value[ms_str_conn_idx_key] = m_nConn_idx;
		json_conn_value[ms_str_max_wait_time_key] =  nMax_wait_time;
	}
	else if(false)
	{
		//TODO::其他厂商
	}
	else
	{
		businlog_error_return_err_reason(false, __CLASS_FUNCTION__ << " | factory_type is invalid:"
			<< m_eFactory_type, str_kernel_err_reason, MSP_ERROR_NOT_SUPPORT);
	}

	bool bSuccess = CCarve_Common_Lib_Tool::instance()->cancel_fast_stop(json_conn_value, str_kernel_err_reason);
	businlog_error_return(bSuccess, ("%s | fail to cancel fast stop carve, ip:%s, json info:%s, reason:%s"
		, __CLASS_FUNCTION__, m_str_ip.c_str(), json_conn_value.toStyledString().c_str(), str_kernel_err_reason.c_str()), MSP_ERROR_FAIL);
	return MSP_SUCCESS;
}

int CCarve::delete_1_file(const string& str_file_path, string& str_kernel_err_reason)
{
	businlog_tracer_perf(CCarve::delete_1_file);
	boost::mutex::scoped_lock guard(m_mutex_for_cmd);
	businlog_error_return_err_reason(true == m_bConnected, __CLASS_FUNCTION__ << " | carve ip:" << m_str_ip 
		<<" is not connected", str_kernel_err_reason, MSP_ERROR_INVALID_OPERATION);

	//构造参数
	Json::Value json_conn_value;
	json_conn_value[ms_str_factory_type_key] = m_eFactory_type;
	json_conn_value[ms_str_carve_type_key] = m_str_carve_type;
	if (CARVE_FACTORY_TYPE_BAOYUAN == m_eFactory_type)
	{
		//宝元库所需要的参数
		json_conn_value[ms_str_conn_idx_key] = m_nConn_idx;
		json_conn_value[ms_str_file_path_key] = str_file_path;
	}
	else if(false)
	{
		//TODO::其他厂商
	}
	else
	{
		businlog_error_return_err_reason(false, __CLASS_FUNCTION__ << " | factory_type is invalid:"
			<< m_eFactory_type, str_kernel_err_reason, MSP_ERROR_NOT_SUPPORT);
	}

	//上传文件
	bool bSuccess = CCarve_Common_Lib_Tool::instance()->delete_1file(json_conn_value, str_kernel_err_reason);
	businlog_error_return(bSuccess, ("%s | fail to delete file, carve ip:%s, file path:%s, json info:%s, reason:%s"
		, __CLASS_FUNCTION__, m_str_ip.c_str(), str_file_path.c_str(), json_conn_value.toStyledString().c_str()
		, str_kernel_err_reason.c_str()), MSP_ERROR_FAIL);
	return MSP_SUCCESS;
}

const string CCarve::ms_str_factory_type_key = "factory_type";

const string CCarve::ms_str_carve_type_key = "carve_type";

const string CCarve::ms_str_conn_idx_key = "conn_idx";

const string CCarve::ms_str_ip_key = "ip";

const string CCarve::ms_str_file_path_key = "file_path";

const string CCarve::ms_str_status_key = "status";

const string CCarve::ms_str_max_wait_time_key = "max_wait_time";

CCarve::CCarve(unsigned short nConn_idx, const string& str_ip) 
	: CDevice(ECARVE, str_ip)
	, m_nConn_idx(nConn_idx)
	, m_bConnected(false)
	, m_eFactory_type(CARVE_FACTORY_TYPE_BAOYUAN) //TODO:临时这么写，后面由参数传入
{

}
