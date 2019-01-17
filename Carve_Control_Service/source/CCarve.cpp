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
	if (CCarve_Common_Lib_Tool::instance()->is_baoyuan(m_eFactory_type, m_str_carve_type))
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

	bool bSuccess = CCarve_Common_Lib_Tool::instance()->connect(json_conn_value, str_err_reason_for_debug, str_err_reason_for_user);
	businlog_error_return(bSuccess, ("%s | fail to connect carve, param in json:%s, reason:%s"
		, __CLASS_FUNCTION__, json_conn_value.toStyledString().c_str(), str_err_reason_for_debug.c_str()), MSP_ERROR_FAIL);
	m_bConnected = true;
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
	if (CCarve_Common_Lib_Tool::instance()->is_baoyuan(m_eFactory_type, m_str_carve_type))
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

int CCarve::set_continue_status(const Json::Value& json_params, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	boost::mutex::scoped_lock guard(m_mutex_for_cmd);
	businlog_error_return_debug_and_user_reason(true == m_bConnected, __CLASS_FUNCTION__ << " | carve ip:" << m_str_ip 
		<<" is not connected", str_err_reason_for_debug, "设备未连接", str_err_reason_for_user, MSP_ERROR_INVALID_OPERATION);
	
	//构造参数
	Json::Value json_conn_value = json_params;
	json_conn_value[ms_str_factory_type_key] = m_eFactory_type;
	json_conn_value[ms_str_carve_type_key] = m_str_carve_type;
	if (CCarve_Common_Lib_Tool::instance()->is_baoyuan(m_eFactory_type, m_str_carve_type))
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
			<< m_eFactory_type, str_err_reason_for_debug, "不支持的厂商类型", str_err_reason_for_user, MSP_ERROR_NOT_SUPPORT);
	}

	bool bSuccess = CCarve_Common_Lib_Tool::instance()->set_continue_status(json_conn_value, str_err_reason_for_debug, str_err_reason_for_user);
//	bool bSuccess = CBaoyuan_Lib::instance()->set_continue_status(m_nConn_idx, nStatus, nMax_wait_time, str_kernel_err_reason);
	businlog_error_return(bSuccess, ("%s | fail to set continue status, ip:%s, json info:%s, reason:%s"
		, __CLASS_FUNCTION__, m_str_ip.c_str(), json_conn_value.toStyledString().c_str(), str_err_reason_for_debug.c_str()), MSP_ERROR_FAIL);
	return MSP_SUCCESS;
}

int CCarve::reset(const Json::Value& json_params, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	boost::mutex::scoped_lock guard(m_mutex_for_cmd);
	businlog_error_return_debug_and_user_reason(true == m_bConnected, __CLASS_FUNCTION__ << " | carve ip:" << m_str_ip 
		<<" is not connected", str_err_reason_for_debug, "设备未连接", str_err_reason_for_user, MSP_ERROR_INVALID_OPERATION);

	//构造参数
	Json::Value json_conn_value = json_params;
	json_conn_value[ms_str_factory_type_key] = m_eFactory_type;
	json_conn_value[ms_str_carve_type_key] = m_str_carve_type;
	if (CCarve_Common_Lib_Tool::instance()->is_baoyuan(m_eFactory_type, m_str_carve_type))
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
			<< m_eFactory_type, str_err_reason_for_debug, "错误的厂商类型", str_err_reason_for_user, MSP_ERROR_NOT_SUPPORT);
	}


	bool bSuccess = CCarve_Common_Lib_Tool::instance()->reset_carve(json_conn_value, str_err_reason_for_debug, str_err_reason_for_user);
	businlog_error_return(bSuccess, ("%s | fail to reset carve, ip:%s, json info:%s, reason:%s"
		, __CLASS_FUNCTION__, m_str_ip.c_str(), json_conn_value.toStyledString().c_str(), str_err_reason_for_debug.c_str()), MSP_ERROR_FAIL);
	return MSP_SUCCESS;
}


int CCarve::start(const Json::Value& json_params, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	//TODO::额外操作
	//使雕刻机开始雕刻
	boost::mutex::scoped_lock guard(m_mutex_for_cmd);
	businlog_error_return_debug_and_user_reason(true == m_bConnected, __CLASS_FUNCTION__ << " | carve ip:" << m_str_ip 
		<<" is not connected", str_err_reason_for_debug, "设备未连接", str_err_reason_for_user, MSP_ERROR_INVALID_OPERATION);

	//构造参数
	Json::Value json_conn_value = json_params;
	json_conn_value[ms_str_factory_type_key] = m_eFactory_type;
	json_conn_value[ms_str_carve_type_key] = m_str_carve_type;
	if (CCarve_Common_Lib_Tool::instance()->is_baoyuan(m_eFactory_type, m_str_carve_type))
	{
		//宝元库所需要的参数
		json_conn_value[ms_str_conn_idx_key] = m_nConn_idx;
		json_conn_value[ms_str_file_path_key] =  m_str_file_path;
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

int CCarve::pause(const Json::Value& json_params, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	boost::mutex::scoped_lock guard(m_mutex_for_cmd);
	businlog_error_return_debug_and_user_reason(true == m_bConnected, __CLASS_FUNCTION__ << " | carve ip:" << m_str_ip 
		<<" is not connected", str_err_reason_for_debug, "设备未连接", str_err_reason_for_user, MSP_ERROR_INVALID_OPERATION);

	//构造参数
	Json::Value json_conn_value = json_params;
	json_conn_value[ms_str_factory_type_key] = m_eFactory_type;
	json_conn_value[ms_str_carve_type_key] = m_str_carve_type;
	if (CCarve_Common_Lib_Tool::instance()->is_baoyuan(m_eFactory_type, m_str_carve_type))
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

	bool bSuccess = CCarve_Common_Lib_Tool::instance()->pause(json_conn_value, str_err_reason_for_debug, str_err_reason_for_user);
	businlog_error_return(bSuccess, ("%s | fail to pause carve, ip:%s, json info:%s, reason:%s"
		, __CLASS_FUNCTION__, m_str_ip.c_str(), json_conn_value.toStyledString().c_str(), str_err_reason_for_debug.c_str()), MSP_ERROR_FAIL);
#ifdef SERVER_WITH_CONTROL_LOGIC
	//累加时间，停止计时，从下次“开始雕刻”再重新计时 (必须配合控制逻辑，结果才正确，否则结果可能错误)
	boost::posix_time::millisec_posix_time_system_config::time_duration_type time_elapse = boost::posix_time::second_clock::universal_time() - m_time_last_start;
	m_nTotal_engraving_time += time_elapse.total_seconds() / 60.0 + 0.5; //按分钟算时四舍五入
#endif
	return MSP_SUCCESS;
}

int CCarve::upload_1_file(const Json::Value& json_params, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	businlog_tracer_perf(CCarve::upload_1_file);
	string str_key = CCarve::ms_str_file_path_key;
	//判定参数合法性
	businlog_error_return_debug_and_user_reason(json_params.isMember(str_key), __CLASS_FUNCTION__ << " | json:" 
		<< json_params.toStyledString() << " without key:" <<  str_key, str_err_reason_for_debug
		, "参数错误", str_err_reason_for_user, MSP_ERROR_INVALID_PARA);

	str_key = ms_str_task_no_key;
	businlog_error_return_debug_and_user_reason(json_params.isMember(str_key), __CLASS_FUNCTION__ << " | json:" 
		<< json_params.toStyledString() << " without key:" <<  str_key, str_err_reason_for_debug
		, "参数错误", str_err_reason_for_user, MSP_ERROR_INVALID_PARA);

	str_key = ms_str_gCode_no_key;
	businlog_error_return_debug_and_user_reason(json_params.isMember(str_key), __CLASS_FUNCTION__ << " | json:" 
		<< json_params.toStyledString() << " without key:" <<  str_key, str_err_reason_for_debug
		, "参数错误", str_err_reason_for_user, MSP_ERROR_INVALID_PARA);

	boost::mutex::scoped_lock guard(m_mutex_for_cmd);
	businlog_error_return_debug_and_user_reason(true == m_bConnected, __CLASS_FUNCTION__ << " | carve ip:" << m_str_ip 
		<<" is not connected", str_err_reason_for_debug, "设备未连接", str_err_reason_for_user, MSP_ERROR_INVALID_OPERATION);
	//获取参数
	m_str_file_path = json_params[ms_str_file_path_key].asString();
	m_str_task_no = json_params[ms_str_task_no_key].asString();
	m_str_gCode_no = json_params[ms_str_gCode_no_key].asString();
	//构造参数
	Json::Value json_conn_value = json_params;
	json_conn_value[ms_str_factory_type_key] = m_eFactory_type;
	json_conn_value[ms_str_carve_type_key] = m_str_carve_type;

	if (CCarve_Common_Lib_Tool::instance()->is_baoyuan(m_eFactory_type, m_str_carve_type))
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

	//上传文件
	bool bSuccess = CCarve_Common_Lib_Tool::instance()->upload_1file(json_conn_value, str_err_reason_for_debug, str_err_reason_for_user);
	businlog_error_return(bSuccess, ("%s | fail to upload file, carve ip:%s, file path:%s, json info:%s, reason:%s"
		, __CLASS_FUNCTION__, m_str_ip.c_str(), m_str_file_path.c_str(), json_conn_value.toStyledString().c_str()
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
	if (CCarve_Common_Lib_Tool::instance()->is_baoyuan(m_eFactory_type, m_str_carve_type))
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
#ifdef SERVER_WITH_CONTROL_LOGIC
	m_eCarve_status = eCarve_common_status;
#endif
	
	return MSP_SUCCESS;
}

int CCarve::stop_fast(const Json::Value& json_params,string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	boost::mutex::scoped_lock guard(m_mutex_for_cmd);
	businlog_error_return_debug_and_user_reason(true == m_bConnected, __CLASS_FUNCTION__ << " | carve ip:" << m_str_ip 
		<<" is not connected", str_err_reason_for_debug, "设备未连接", str_err_reason_for_user, MSP_ERROR_INVALID_OPERATION);

	//构造参数
	Json::Value json_conn_value = json_params;
	json_conn_value[ms_str_factory_type_key] = m_eFactory_type;
	json_conn_value[ms_str_carve_type_key] = m_str_carve_type;
	if (CCarve_Common_Lib_Tool::instance()->is_baoyuan(m_eFactory_type, m_str_carve_type))
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

	bool bSuccess = CCarve_Common_Lib_Tool::instance()->stop_fast(json_conn_value, str_err_reason_for_debug, str_err_reason_for_user);
	businlog_error_return(bSuccess, ("%s | fail to fast stop carve, ip:%s, json info:%s, reason:%s"
		, __CLASS_FUNCTION__, m_str_ip.c_str(), json_conn_value.toStyledString().c_str(), str_err_reason_for_debug.c_str()), MSP_ERROR_FAIL);
#ifdef SERVER_WITH_CONTROL_LOGIC
	//累加时间，停止计时，从下次“开始雕刻”再重新计时(必须配合控制逻辑，结果才正确，否则结果可能错误)
	boost::posix_time::millisec_posix_time_system_config::time_duration_type time_elapse = boost::posix_time::second_clock::universal_time() - m_time_last_start;
	m_nTotal_engraving_time += time_elapse.total_seconds() / 60.0 + 0.5; //按分钟算时四舍五入
#endif
	return MSP_SUCCESS;
}

int CCarve::cancel_fast_stop(const Json::Value& json_params, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	businlog_tracer_perf(CCarve::cancel_fast_stop);
	boost::mutex::scoped_lock guard(m_mutex_for_cmd);
	businlog_error_return_debug_and_user_reason(true == m_bConnected, __CLASS_FUNCTION__ << " | carve ip:" << m_str_ip 
		<<" is not connected", str_err_reason_for_debug, "设备未连接", str_err_reason_for_user, MSP_ERROR_INVALID_OPERATION);
	
	//构造参数
	Json::Value json_conn_value = json_params;
	json_conn_value[ms_str_factory_type_key] = m_eFactory_type;
	json_conn_value[ms_str_carve_type_key] = m_str_carve_type;
	if (CCarve_Common_Lib_Tool::instance()->is_baoyuan(m_eFactory_type, m_str_carve_type))
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

	bool bSuccess = CCarve_Common_Lib_Tool::instance()->cancel_fast_stop(json_conn_value, str_err_reason_for_debug, str_err_reason_for_user);
	businlog_error_return(bSuccess, ("%s | fail to cancel fast stop carve, ip:%s, json info:%s, reason:%s"
		, __CLASS_FUNCTION__, m_str_ip.c_str(), json_conn_value.toStyledString().c_str(), str_err_reason_for_debug.c_str()), MSP_ERROR_FAIL);
	return MSP_SUCCESS;
}

int CCarve::delete_1_file(const Json::Value& json_params, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	businlog_tracer_perf(CCarve::delete_1_file);
	boost::mutex::scoped_lock guard(m_mutex_for_cmd);
	businlog_error_return_debug_and_user_reason(true == m_bConnected, __CLASS_FUNCTION__ << " | carve ip:" << m_str_ip 
		<<" is not connected", str_err_reason_for_debug, "设备未连接", str_err_reason_for_user, MSP_ERROR_INVALID_OPERATION);

	//构造参数
	Json::Value json_conn_value = json_params;
	json_conn_value[ms_str_factory_type_key] = m_eFactory_type;
	json_conn_value[ms_str_carve_type_key] = m_str_carve_type;
	if (CCarve_Common_Lib_Tool::instance()->is_baoyuan(m_eFactory_type, m_str_carve_type))
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

	//删除文件
	bool bSuccess = CCarve_Common_Lib_Tool::instance()->delete_1file(json_conn_value, str_err_reason_for_debug, str_err_reason_for_user);
	businlog_error_return(bSuccess, ("%s | fail to delete file, carve ip:%s, json info:%s, reason:%s"
		, __CLASS_FUNCTION__, m_str_ip.c_str(), json_conn_value.toStyledString().c_str()
		, str_err_reason_for_debug.c_str()), MSP_ERROR_FAIL);

	//文件删除成处理
	//TODO: ----------------待确认
	//参数恢复初始
	m_str_file_path = "";
	m_str_task_no = "";
	m_str_gCode_no = "";

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
	if (CCarve_Common_Lib_Tool::instance()->is_baoyuan(m_eFactory_type, m_str_carve_type))
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
	if (CCarve_Common_Lib_Tool::instance()->is_baoyuan(m_eFactory_type, m_str_carve_type))
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
	if (CCarve_Common_Lib_Tool::instance()->is_baoyuan(m_eFactory_type, m_str_carve_type))
	{
		m_nConn_idx = json_conn_value[ms_str_conn_idx_key].asInt();
	}
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
	if (CCarve_Common_Lib_Tool::instance()->is_baoyuan(m_eFactory_type, m_str_carve_type))
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


int CCarve::get_info(SCarve_Info& carve_info, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	//由于如下接口已经上锁，故在此之前不要上锁
	carve_info.eCarve_status;
	string str_single_err_for_debug, str_single_err_for_user; //获取单个信息时的错误
	//获取单个信息出错时，不返回。
	bool bHas_error = false; //是否出错过
	//获取雕刻机状态
	int ret = MSP_SUCCESS;
#ifdef SERVER_WITH_CONTROL_LOGIC
	{
		//带有逻辑时，会有一个线程不断轮询所有雕刻机，以获取它们的状态。
		//完成状态可能已经被轮询线程读取了，故从成员变量中读取，而不可从机器中来读取。
		boost::mutex::scoped_lock guard(m_mutex_for_cmd);
		carve_info.eCarve_status = m_eCarve_status;
	}
#else 
	ret = get_carve_status(carve_info.eCarve_status, str_single_err_for_debug, str_single_err_for_user);
	if (ret)
	{
		businlog_error("%s | fail to get carve status, reason:%s", __CLASS_FUNCTION__, str_single_err_for_debug.c_str());
		str_err_reason_for_debug += "." + str_single_err_for_debug;
		str_err_reason_for_user += "." + str_single_err_for_user;
		bHas_error = true;
	}
#endif

	//获取当前文件行号
	ret = get_current_line_num(carve_info.nCurrent_line_num, str_single_err_for_debug, str_single_err_for_user);
	if (ret)
	{
		businlog_error("%s | fail to get current line num, reason:%s", __CLASS_FUNCTION__, str_single_err_for_debug.c_str());
		str_err_reason_for_debug += "." + str_single_err_for_debug;
		str_err_reason_for_user += "." + str_single_err_for_user;
		bHas_error = true;
	}
	boost::mutex::scoped_lock guard(m_mutex_for_cmd);
#ifdef SERVER_WITH_CONTROL_LOGIC
	//获取总雕刻时间
	if (CARVE_STATUS_ENGRAVING == m_eCarve_status)
	{//如果处于雕刻中
		boost::posix_time::millisec_posix_time_system_config::time_duration_type time_elapse 
			= boost::posix_time::second_clock::universal_time() - m_time_last_start;
		carve_info.nTotal_engraving_time = m_nTotal_engraving_time + time_elapse.total_seconds() / 60.0 + 0.5;//按分钟算时四舍五入
	}
	else
	{
		carve_info.nTotal_engraving_time = m_nTotal_engraving_time;
	}
#endif
	carve_info.str_gCode_no = m_str_gCode_no;
	carve_info.str_id = m_str_id;
	carve_info.str_machine_ip = m_str_ip;
	carve_info.str_task_no = m_str_task_no;
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

int CCarve::adjust_speed(const Json::Value& json_params,string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	boost::mutex::scoped_lock guard(m_mutex_for_cmd);
	businlog_error_return_debug_and_user_reason(true == m_bConnected, __CLASS_FUNCTION__ << " | carve ip:" << m_str_ip 
		<<" is not connected", str_err_reason_for_debug, "设备未连接", str_err_reason_for_user, MSP_ERROR_INVALID_OPERATION);
	
	//获取参数--速度
	unsigned int n_speed_percent = json_params["speed_percent"].asInt();

	//构造参数
	Json::Value json_conn_value;
	json_conn_value[ms_str_factory_type_key] = m_eFactory_type;
	json_conn_value[ms_str_carve_type_key] = m_str_carve_type;

	//设置最大等待时间
	unsigned short nMax_wait_time = 1000;
	string str_key = "max_wait_time";
	if (json_params.isMember(str_key))
	{//参数中含有最大等待时间
		nMax_wait_time = json_params[str_key].asInt();
	}

	json_conn_value[CCarve::ms_str_max_wait_time_key] = nMax_wait_time;

	if (CCarve_Common_Lib_Tool::instance()->is_baoyuan(m_eFactory_type, m_str_carve_type))
	{
		//宝元库所需要的参数
		json_conn_value[ms_str_conn_idx_key] = m_nConn_idx;
		json_conn_value["speed_percent"] = n_speed_percent;
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

	//调整速度
	bool bSuccess = CCarve_Common_Lib_Tool::instance()->adjust_speed(json_conn_value, str_err_reason_for_debug, str_err_reason_for_user);
	businlog_error_return(bSuccess, ("%s | fail to adjust_speed, carve ip:%s, speed percent:%d, json info:%s, reason:%s"
		, __CLASS_FUNCTION__, m_str_ip.c_str(), n_speed_percent, json_conn_value.toStyledString().c_str()
		, str_err_reason_for_debug.c_str()), MSP_ERROR_FAIL);
	return MSP_SUCCESS;
}


void CCarve::start_count_engraving_time()
{
#ifdef SERVER_WITH_CONTROL_LOGIC
	businlog_crit("%s",  __CLASS_FUNCTION__);
	boost::mutex::scoped_lock guard(m_mutex_for_cmd);
	m_time_last_start =  boost::posix_time::second_clock::universal_time();
#endif
}

int CCarve::pause_count_engraving_time(string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
#ifdef SERVER_WITH_CONTROL_LOGIC
	businlog_crit("%s",  __CLASS_FUNCTION__);
	boost::mutex::scoped_lock guard(m_mutex_for_cmd);
	//累加时间，停止计时，从下次“开始雕刻”再重新计时(必须配合控制逻辑，结果才正确，否则结果可能错误)
	boost::posix_time::millisec_posix_time_system_config::time_duration_type time_elapse = boost::posix_time::second_clock::universal_time() - m_time_last_start;
	m_nTotal_engraving_time += time_elapse.total_seconds() / 60.0 + 0.5; //按分钟算时四舍五入
#endif
	return 0;
}

const string CCarve::ms_str_factory_type_key = "carveExFactory";

const string CCarve::ms_str_carve_type_key = "carveType";

const string CCarve::ms_str_conn_idx_key = "conn_idx";

const string CCarve::ms_str_ip_key = "ip";

const string CCarve::ms_str_file_path_key = "filepath";

const string CCarve::ms_str_status_key = "status";

const string CCarve::ms_str_max_wait_time_key = "max_wait_time";

const string CCarve::ms_str_carve_id_key = "carveId";

const string CCarve::ms_str_task_no_key = "taskNo";

const string CCarve::ms_str_gCode_no_key = "gNo";

const string CCarve::ms_str_worktime_key = "worktime";

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
#ifdef SERVER_WITH_CONTROL_LOGIC
	, m_nTotal_engraving_time(0)
#endif
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
#ifdef  SERVER_WITH_CONTROL_LOGIC
	if (!json_params.isMember(ms_str_worktime_key))
	{
		str_err_reason = string("json:") + json_params.toStyledString() + string(" without key:") + CCarve::ms_str_worktime_key;
		businlog_error("%s | err reason:%s", __FUNCTION__, str_err_reason.c_str());
		throw std::exception(str_err_reason.c_str());
	}
	m_nTotal_engraving_time = json_params[ms_str_worktime_key].asInt();
#endif
}

SCarve_Info::SCarve_Info() 
	: eCarve_status(CARVE_STATUS_MIN)
	, nTotal_engraving_time(0)
	, nCurrent_line_num(0)
{

}
