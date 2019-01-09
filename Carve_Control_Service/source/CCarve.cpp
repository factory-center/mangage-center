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


int CCarve::connect(string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	boost::mutex::scoped_lock guard(m_mutex_for_cmd);
	if (m_bConnected)
	{
		businlog_warn("%s | carve ip:%s is connected.", __CLASS_FUNCTION__, m_str_ip.c_str());
		return MSP_SUCCESS;
	}
	//判定资源是否申请成功
	businlog_error_return_debug_and_user_reason(true == m_bAcq_Res_Success, __CLASS_FUNCTION__ << " | resources is not OK, ip:"
		<< m_str_ip, str_err_reason_for_debug, "没有成功申请资源", str_err_reason_for_user, MSP_ERROR_RES_LOAD);
	//构造连接参数
	Json::Value json_conn_value;
	json_conn_value[ms_str_factory_type_key] = m_eFactory_type;
	json_conn_value[ms_str_carve_type_key] = m_str_carve_type;
	if (CARVE_FACTORY_TYPE_BAOYUAN == m_eFactory_type)
	{
		//宝元库所需要的参数
		json_conn_value[ms_str_ip_key] = m_str_ip;
		json_conn_value[ms_str_conn_idx_key] =  m_nConn_idx;
	}
	else if(false)
	{
		//TODO::其他厂商
	}
	else
	{
		businlog_error_return_debug_and_user_reason(false, __CLASS_FUNCTION__ << " | factory_type is invalid:"
			<< m_eFactory_type, str_err_reason_for_debug, "厂商类型错误", str_err_reason_for_user, MSP_ERROR_NOT_SUPPORT);
	}
//    bool bSuccess = CBaoyuan_Lib::instance()->create_connection(m_nConn_idx, m_str_ip, str_kernel_err_reason);
	bool bSuccess = CCarve_Common_Lib_Tool::instance()->connect(json_conn_value, str_err_reason_for_debug, str_err_reason_for_user);
	businlog_error_return(bSuccess, ("%s | fail to connect carve, param in json:%s, reason:%s"
		, __CLASS_FUNCTION__, json_conn_value.toStyledString().c_str(), str_err_reason_for_debug.c_str()), MSP_ERROR_FAIL);
	m_bConnected = true;
	if (CARVE_FACTORY_TYPE_BAOYUAN == m_eFactory_type)
	{
		m_nConn_idx = json_conn_value[ms_str_conn_idx_key].asInt();
	}
	return MSP_SUCCESS;
}

int CCarve::disconnect(string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	boost::mutex::scoped_lock guard(m_mutex_for_cmd);
	//判定当前状态
	businlog_error_return_debug_and_user_reason(true == m_bConnected, __CLASS_FUNCTION__ << " | carve ip:" << m_str_ip 
		<<" is not connected", str_err_reason_for_debug, "设备未连接", str_err_reason_for_user, MSP_ERROR_FAIL);
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
		businlog_error_return_debug_and_user_reason(false, __CLASS_FUNCTION__ << " | factory_type is invalid:"
			<< m_eFactory_type, str_err_reason_for_debug, "不支持的设备厂商类型", str_err_reason_for_user, MSP_ERROR_NOT_SUPPORT);
	}

	bool bSuccess = CCarve_Common_Lib_Tool::instance()->disconnect(json_conn_value, str_err_reason_for_debug, str_err_reason_for_user);
	businlog_error_return(bSuccess, ("%s | fail to disconnect carve, params in json:%s, reason:%s"
		, __CLASS_FUNCTION__, json_conn_value.toStyledString().c_str(), str_err_reason_for_debug.c_str()), MSP_ERROR_FAIL);
	//成功断开，则更新状态
	m_bConnected = false;
	return MSP_SUCCESS;
}

int CCarve::set_continue_status(unsigned char nStatus, unsigned short nMax_wait_time, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	boost::mutex::scoped_lock guard(m_mutex_for_cmd);
	businlog_error_return_debug_and_user_reason(true == m_bConnected, __CLASS_FUNCTION__ << " | carve ip:" << m_str_ip 
		<<" is not connected", str_err_reason_for_debug, "设备未连接", str_err_reason_for_user, MSP_ERROR_INVALID_OPERATION);

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
		businlog_error_return_debug_and_user_reason(false, __CLASS_FUNCTION__ << " | factory_type is invalid:"
			<< m_eFactory_type, str_err_reason_for_debug, "不支持的厂商类型", str_err_reason_for_user, MSP_ERROR_NOT_SUPPORT);
	}

	bool bSuccess = CCarve_Common_Lib_Tool::instance()->set_continue_status(json_conn_value, str_err_reason_for_debug, str_err_reason_for_user);
//	bool bSuccess = CBaoyuan_Lib::instance()->set_continue_status(m_nConn_idx, nStatus, nMax_wait_time, str_kernel_err_reason);
	businlog_error_return(bSuccess, ("%s | fail to set continue status, ip:%s, json info:%s, reason:%s"
		, __CLASS_FUNCTION__, m_str_ip.c_str(), json_conn_value.toStyledString().c_str(), str_err_reason_for_debug.c_str()), MSP_ERROR_FAIL);
	return MSP_SUCCESS;
}

int CCarve::reset(unsigned short nMax_wait_time, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	boost::mutex::scoped_lock guard(m_mutex_for_cmd);
	businlog_error_return_debug_and_user_reason(true == m_bConnected, __CLASS_FUNCTION__ << " | carve ip:" << m_str_ip 
		<<" is not connected", str_err_reason_for_debug, "设备未连接", str_err_reason_for_user, MSP_ERROR_INVALID_OPERATION);

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
		businlog_error_return_debug_and_user_reason(false, __CLASS_FUNCTION__ << " | factory_type is invalid:"
			<< m_eFactory_type, str_err_reason_for_debug, "错误的厂商类型", str_err_reason_for_user, MSP_ERROR_NOT_SUPPORT);
	}


	bool bSuccess = CCarve_Common_Lib_Tool::instance()->reset_carve(json_conn_value, str_err_reason_for_debug, str_err_reason_for_user);
	businlog_error_return(bSuccess, ("%s | fail to reset carve, ip:%s, json info:%s, reason:%s"
		, __CLASS_FUNCTION__, m_str_ip.c_str(), json_conn_value.toStyledString().c_str(), str_err_reason_for_debug.c_str()), MSP_ERROR_FAIL);
	return MSP_SUCCESS;
}


int CCarve::start(const string& str_nc_file_path, unsigned short nMax_wait_time, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	//TODO::额外操作
	//使雕刻机开始雕刻
	boost::mutex::scoped_lock guard(m_mutex_for_cmd);
	businlog_error_return_debug_and_user_reason(true == m_bConnected, __CLASS_FUNCTION__ << " | carve ip:" << m_str_ip 
		<<" is not connected", str_err_reason_for_debug, "设备未连接", str_err_reason_for_user, MSP_ERROR_INVALID_OPERATION);

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
		businlog_error_return_debug_and_user_reason(false, __CLASS_FUNCTION__ << " | factory_type is invalid:"
			<< m_eFactory_type, str_err_reason_for_debug, "错误的设备厂商类型", str_err_reason_for_user, MSP_ERROR_NOT_SUPPORT);
	}

	bool bSuccess = CCarve_Common_Lib_Tool::instance()->start(json_conn_value, str_err_reason_for_debug, str_err_reason_for_user);
	businlog_error_return(bSuccess, ("%s | fail to start carve to engrave, ip:%s, reason:%s"
		, __CLASS_FUNCTION__, m_str_ip.c_str(), str_err_reason_for_debug.c_str()), MSP_ERROR_FAIL);
	return MSP_SUCCESS;
}

int CCarve::pause(unsigned short nMax_wait_time, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	boost::mutex::scoped_lock guard(m_mutex_for_cmd);
	businlog_error_return_debug_and_user_reason(true == m_bConnected, __CLASS_FUNCTION__ << " | carve ip:" << m_str_ip 
		<<" is not connected", str_err_reason_for_debug, "设备未连接", str_err_reason_for_user, MSP_ERROR_INVALID_OPERATION);

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
		businlog_error_return_debug_and_user_reason(false, __CLASS_FUNCTION__ << " | factory_type is invalid:"
			<< m_eFactory_type, str_err_reason_for_debug, "错误的设备厂商类型", str_err_reason_for_user, MSP_ERROR_NOT_SUPPORT);
	}

	bool bSuccess = CCarve_Common_Lib_Tool::instance()->pause(json_conn_value, str_err_reason_for_debug, str_err_reason_for_user);
	businlog_error_return(bSuccess, ("%s | fail to pause carve, ip:%s, json info:%s, reason:%s"
		, __CLASS_FUNCTION__, m_str_ip.c_str(), json_conn_value.toStyledString().c_str(), str_err_reason_for_debug.c_str()), MSP_ERROR_FAIL);
	return MSP_SUCCESS;
}

int CCarve::upload_1_file(const string& str_file_path, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	businlog_tracer_perf(CCarve::upload_1_file);
	boost::mutex::scoped_lock guard(m_mutex_for_cmd);
	businlog_error_return_debug_and_user_reason(true == m_bConnected, __CLASS_FUNCTION__ << " | carve ip:" << m_str_ip 
		<<" is not connected", str_err_reason_for_debug, "设备未连接", str_err_reason_for_user, MSP_ERROR_INVALID_OPERATION);
	
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
		businlog_error_return_debug_and_user_reason(false, __CLASS_FUNCTION__ << " | factory_type is invalid:"
			<< m_eFactory_type, str_err_reason_for_debug, "错误的设备厂商类型", str_err_reason_for_user, MSP_ERROR_NOT_SUPPORT);
	}

	//上传文件
	bool bSuccess = CCarve_Common_Lib_Tool::instance()->upload_1file(json_conn_value, str_err_reason_for_debug, str_err_reason_for_user);
	businlog_error_return(bSuccess, ("%s | fail to upload file, carve ip:%s, file path:%s, json info:%s, reason:%s"
		, __CLASS_FUNCTION__, m_str_ip.c_str(), str_file_path.c_str(), json_conn_value.toStyledString().c_str()
		, str_err_reason_for_debug.c_str()), MSP_ERROR_FAIL);
	return MSP_SUCCESS;
}

int CCarve::get_carve_status(ECARVE_STATUS_TYPE& eCarve_common_status, string& str_err_reason_for_debug, string& str_err_reason_for_user)
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
		businlog_error_return_debug_and_user_reason(false, __CLASS_FUNCTION__ << " | factory_type is invalid:"
			<< m_eFactory_type, str_err_reason_for_debug, "错误的设备厂商类型", str_err_reason_for_user, MSP_ERROR_NOT_SUPPORT);
	}
	bool bSuccess = CCarve_Common_Lib_Tool::instance()->get_carve_status(json_conn_value, eCarve_common_status, str_err_reason_for_debug, str_err_reason_for_user);
	businlog_error_return(bSuccess, ("%s | fail to get baoyuan carve status, json:%s, reason:%s"
		, __CLASS_FUNCTION__, json_conn_value.toStyledString().c_str(), str_err_reason_for_debug.c_str()), MSP_ERROR_FAIL);
	//此时成功获取雕刻机状态
	
	return MSP_SUCCESS;
}

int CCarve::stop_fast(unsigned short nMax_wait_time, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	boost::mutex::scoped_lock guard(m_mutex_for_cmd);
	businlog_error_return_debug_and_user_reason(true == m_bConnected, __CLASS_FUNCTION__ << " | carve ip:" << m_str_ip 
		<<" is not connected", str_err_reason_for_debug, "设备未连接", str_err_reason_for_user, MSP_ERROR_INVALID_OPERATION);

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
		businlog_error_return_debug_and_user_reason(false, __CLASS_FUNCTION__ << " | factory_type is invalid:"
			<< m_eFactory_type, str_err_reason_for_debug, "错误的设备厂商类型", str_err_reason_for_user, MSP_ERROR_NOT_SUPPORT);
	}

	bool bSuccess = CCarve_Common_Lib_Tool::instance()->stop_fast(json_conn_value, str_err_reason_for_debug, str_err_reason_for_user);
	businlog_error_return(bSuccess, ("%s | fail to fast stop carve, ip:%s, json info:%s, reason:%s"
		, __CLASS_FUNCTION__, m_str_ip.c_str(), json_conn_value.toStyledString().c_str(), str_err_reason_for_debug.c_str()), MSP_ERROR_FAIL);
	return MSP_SUCCESS;
}

int CCarve::cancel_fast_stop(unsigned short nMax_wait_time, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	businlog_tracer_perf(CCarve::cancel_fast_stop);
	boost::mutex::scoped_lock guard(m_mutex_for_cmd);
	businlog_error_return_debug_and_user_reason(true == m_bConnected, __CLASS_FUNCTION__ << " | carve ip:" << m_str_ip 
		<<" is not connected", str_err_reason_for_debug, "设备未连接", str_err_reason_for_user, MSP_ERROR_INVALID_OPERATION);

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
		businlog_error_return_debug_and_user_reason(false, __CLASS_FUNCTION__ << " | factory_type is invalid:"
			<< m_eFactory_type, str_err_reason_for_debug, "错误的设备厂商类型", str_err_reason_for_user, MSP_ERROR_NOT_SUPPORT);
	}

	bool bSuccess = CCarve_Common_Lib_Tool::instance()->cancel_fast_stop(json_conn_value, str_err_reason_for_debug, str_err_reason_for_user);
	businlog_error_return(bSuccess, ("%s | fail to cancel fast stop carve, ip:%s, json info:%s, reason:%s"
		, __CLASS_FUNCTION__, m_str_ip.c_str(), json_conn_value.toStyledString().c_str(), str_err_reason_for_debug.c_str()), MSP_ERROR_FAIL);
	return MSP_SUCCESS;
}

int CCarve::delete_1_file(const string& str_file_path, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	businlog_tracer_perf(CCarve::delete_1_file);
	boost::mutex::scoped_lock guard(m_mutex_for_cmd);
	businlog_error_return_debug_and_user_reason(true == m_bConnected, __CLASS_FUNCTION__ << " | carve ip:" << m_str_ip 
		<<" is not connected", str_err_reason_for_debug, "设备未连接", str_err_reason_for_user, MSP_ERROR_INVALID_OPERATION);

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
		businlog_error_return_debug_and_user_reason(false, __CLASS_FUNCTION__ << " | factory_type is invalid:"
			<< m_eFactory_type, str_err_reason_for_debug, "错误的设备厂商类型", str_err_reason_for_user, MSP_ERROR_NOT_SUPPORT);
	}

	//上传文件
	bool bSuccess = CCarve_Common_Lib_Tool::instance()->delete_1file(json_conn_value, str_err_reason_for_debug, str_err_reason_for_user);
	businlog_error_return(bSuccess, ("%s | fail to delete file, carve ip:%s, file path:%s, json info:%s, reason:%s"
		, __CLASS_FUNCTION__, m_str_ip.c_str(), str_file_path.c_str(), json_conn_value.toStyledString().c_str()
		, str_err_reason_for_debug.c_str()), MSP_ERROR_FAIL);
	return MSP_SUCCESS;
}

int CCarve::get_current_line_num(int& nCurrent_line_num, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	businlog_tracer_perf(CCarve::get_current_line_num);
	boost::mutex::scoped_lock guard(m_mutex_for_cmd);
	businlog_error_return_debug_and_user_reason(true == m_bConnected, __CLASS_FUNCTION__ << " | carve ip:" << m_str_ip 
		<<" is not connected", str_err_reason_for_debug, "设备未连接", str_err_reason_for_user, MSP_ERROR_INVALID_OPERATION);

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
		businlog_error_return_debug_and_user_reason(false, __CLASS_FUNCTION__ << " | factory_type is invalid:"
			<< m_eFactory_type, str_err_reason_for_debug, "错误的设备厂商类型", str_err_reason_for_user, MSP_ERROR_NOT_SUPPORT);
	}

	//获取雕刻机中当前文件行号
	bool bSuccess = CCarve_Common_Lib_Tool::instance()->get_current_line_num(json_conn_value, nCurrent_line_num, str_err_reason_for_debug, str_err_reason_for_user);
	businlog_error_return(bSuccess, ("%s | fail to get line num, carve ip:%s, json info:%s, reason:%s"
		, __CLASS_FUNCTION__, m_str_ip.c_str(), json_conn_value.toStyledString().c_str()
		, str_err_reason_for_debug.c_str()), MSP_ERROR_FAIL);
	return MSP_SUCCESS;
}

int CCarve::acquire_resource(string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	businlog_tracer_perf(CCarve::acquire_resource);
	boost::mutex::scoped_lock guard(m_mutex_for_cmd);

	//构造参数
	Json::Value json_conn_value;
	json_conn_value[ms_str_factory_type_key] = m_eFactory_type;
	json_conn_value[ms_str_carve_type_key] = m_str_carve_type;
	if (CARVE_FACTORY_TYPE_BAOYUAN == m_eFactory_type)
	{
		//宝元库所需要的参数
	}
	else if(false)
	{
		//TODO::其他厂商
	}
	else
	{
		businlog_error_return_debug_and_user_reason(false, __CLASS_FUNCTION__ << " | factory_type is invalid:"
			<< m_eFactory_type, str_err_reason_for_debug, "错误的设备厂商类型", str_err_reason_for_user, MSP_ERROR_NOT_SUPPORT);
	}

	//申请资源
	bool bSuccess = CCarve_Common_Lib_Tool::instance()->acquire_resource(json_conn_value, str_err_reason_for_debug, str_err_reason_for_user);
	businlog_error_return(bSuccess, ("%s | fail to acquire resource, carve ip:%s, json info:%s, reason:%s"
		, __CLASS_FUNCTION__, m_str_ip.c_str(), json_conn_value.toStyledString().c_str()
		, str_err_reason_for_debug.c_str()), MSP_ERROR_RES_LOAD);
	m_nConn_idx = json_conn_value[ms_str_conn_idx_key].asInt();
	m_bAcq_Res_Success = true;
	return MSP_SUCCESS;
}

int CCarve::release_resource(string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	businlog_tracer_perf(CCarve::release_resource);
	boost::mutex::scoped_lock guard(m_mutex_for_cmd);

	//构造参数
	Json::Value json_conn_value;
	json_conn_value[ms_str_factory_type_key] = m_eFactory_type;
	json_conn_value[ms_str_carve_type_key] = m_str_carve_type;
	if (CARVE_FACTORY_TYPE_BAOYUAN == m_eFactory_type)
	{
		//宝元库所需要的参数
		json_conn_value[ms_str_conn_idx_key] =  m_nConn_idx;
	}
	else if(false)
	{
		//TODO::其他厂商
	}
	else
	{
		businlog_error_return_debug_and_user_reason(false, __CLASS_FUNCTION__ << " | factory_type is invalid:"
			<< m_eFactory_type, str_err_reason_for_debug, "错误的设备厂商类型", str_err_reason_for_user, MSP_ERROR_NOT_SUPPORT);
	}

	//释放资源
	bool bSuccess = CCarve_Common_Lib_Tool::instance()->release_resource(json_conn_value, str_err_reason_for_debug, str_err_reason_for_user);
	businlog_error_return(bSuccess, ("%s | fail to release resource, carve ip:%s, json info:%s, reason:%s"
		, __CLASS_FUNCTION__, m_str_ip.c_str(), json_conn_value.toStyledString().c_str()
		, str_err_reason_for_debug.c_str()), MSP_ERROR_RES_FREE);
	m_bAcq_Res_Success = false;
	return MSP_SUCCESS;
}

int CCarve::get_total_engraving_time(int& nTotal_engraving_time, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	businlog_tracer_perf(CCarve::get_current_line_num);
	boost::mutex::scoped_lock guard(m_mutex_for_cmd);
	businlog_error_return_debug_and_user_reason(true == m_bConnected, __CLASS_FUNCTION__ << " | carve ip:" << m_str_ip 
		<<" is not connected", str_err_reason_for_debug, "设备未连接", str_err_reason_for_user, MSP_ERROR_INVALID_OPERATION);

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
		businlog_error_return_debug_and_user_reason(false, __CLASS_FUNCTION__ << " | factory_type is invalid:"
			<< m_eFactory_type, str_err_reason_for_debug, "错误的设备厂商类型", str_err_reason_for_user, MSP_ERROR_NOT_SUPPORT);
	}

	//获取总的雕刻时间
	bool bSuccess = false/*CCarve_Common_Lib_Tool::instance()->get_current_line_num(json_conn_value, nCurrent_line_num, str_err_reason_for_debug, str_err_reason_for_user)*/;
	businlog_error_return(bSuccess, ("%s | fail to get total engraving time, carve ip:%s, json info:%s, reason:%s"
		, __CLASS_FUNCTION__, m_str_ip.c_str(), json_conn_value.toStyledString().c_str()
		, str_err_reason_for_debug.c_str()), MSP_ERROR_FAIL);
	return MSP_SUCCESS;
}

int CCarve::get_info(SCarve_Info& carve_info, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	//由于相关接口已经上锁，故这里不用上锁
	carve_info.eCarve_status;
	string str_single_err_for_debug, str_single_err_for_user; //获取单个信息时的错误
	//获取单个信息出错时，不返回。
	bool bHas_error = false; //是否出错过
	//获取雕刻机状态
	int ret = get_carve_status(carve_info.eCarve_status, str_single_err_for_debug, str_single_err_for_user);
	if (ret)
	{
		businlog_error("%s | fail to get carve status, reason:%s", __CLASS_FUNCTION__, str_single_err_for_debug.c_str());
		str_err_reason_for_debug += "." + str_single_err_for_debug;
		str_err_reason_for_user += "." + str_single_err_for_user;
		bHas_error = true;
	}
	//获取当前文件行号
	ret = get_current_line_num(carve_info.nCurrent_line_num, str_single_err_for_debug, str_single_err_for_user);
	if (ret)
	{
		businlog_error("%s | fail to get current line num, reason:%s", __CLASS_FUNCTION__, str_single_err_for_debug.c_str());
		str_err_reason_for_debug += "." + str_single_err_for_debug;
		str_err_reason_for_user += "." + str_single_err_for_user;
		bHas_error = true;
	}
	//todo::获取总的雕刻时间
	//carve_info.str_gCode_no
	carve_info.str_id = m_str_id;
	carve_info.str_machine_ip = m_str_ip;
	//carve_info.str_task_no
	if (bHas_error)
	{
		ret = MSP_ERROR_FAIL;
	}
	else
	{
		ret = MSP_SUCCESS;
	}
	return ret;
}

const string CCarve::ms_str_factory_type_key = "carveExFactory";

const string CCarve::ms_str_carve_type_key = "carveType";

const string CCarve::ms_str_conn_idx_key = "conn_idx";

const string CCarve::ms_str_ip_key = "ip";

const string CCarve::ms_str_file_path_key = "file_path";

const string CCarve::ms_str_status_key = "status";

const string CCarve::ms_str_max_wait_time_key = "max_wait_time";

const string CCarve::ms_str_carve_id_key = "carveId";

CCarve::~CCarve()
{
	businlog_tracer_perf(CCarve::~CCarve);
	__ph__.log("%s | ip:%s.", __CLASS_FUNCTION__, m_str_ip.c_str());
	//由于涉及自己调用自己，故这里m_mutex_for_cmd无需上锁
	//如果处于链接状态，则将其断开
	int ret = 0;
	string str_err_reason_for_debug, str_err_reason_for_user;
	if (m_bConnected)
	{
		ret = disconnect(str_err_reason_for_debug, str_err_reason_for_user);
		if (ret)
		{//断开连接失败
			businlog_error("%s | fail to disconnect, ip:%s, reason:%s", __CLASS_FUNCTION__, m_str_ip.c_str(), str_err_reason_for_debug.c_str());
		}
		else
		{
			m_bConnected = false;
		}
	}
	//如果资源处于占用状态，则将其释放
	if (m_bAcq_Res_Success)
	{
		ret = release_resource(str_err_reason_for_debug, str_err_reason_for_user);
		if (ret)
		{//释放资源失败
			businlog_error("%s | fail to release resource, ip:%s, reason:%s", __CLASS_FUNCTION__, m_str_ip.c_str(), str_err_reason_for_debug.c_str());
		}
		else
		{
			m_bAcq_Res_Success = false;
		}
	}
}

CCarve::CCarve(const Json::Value& json_params)
	: CDevice(ECARVE, json_params)
	, m_bConnected(false)
	, m_nConn_idx(-2)
	, m_bAcq_Res_Success(false)
{
	std::string str_err_reason;
	if (!json_params.isMember(CCarve::ms_str_factory_type_key))
	{
		str_err_reason = string("json:") + json_params.toStyledString() + string(" without key:") + CCarve::ms_str_factory_type_key;
		businlog_error("%s | err reason:%s", __FUNCTION__, str_err_reason.c_str());
		throw std::exception(str_err_reason.c_str());
	}

	if (!json_params.isMember(CCarve::ms_str_carve_type_key))
	{
		str_err_reason = string("json:") + json_params.toStyledString() + string(" without key:") + CCarve::ms_str_carve_type_key;
		businlog_error("%s | err reason:%s", __FUNCTION__, str_err_reason.c_str());
		throw std::exception(str_err_reason.c_str());
	}

	m_str_carve_type = json_params[ms_str_carve_type_key].asString();
	if ("BaoYuan" == json_params[ms_str_factory_type_key].asString())
	{
		m_eFactory_type = CARVE_FACTORY_TYPE_BAOYUAN;
	}
}

SCarve_Info::SCarve_Info() 
	: eCarve_status(CARVE_STATUS_MIN)
	, nTotal_engraving_time(0)
	, nCurrent_line_num(0)
{

}
