/******************************************************************
* Copyright (c) 2016-2017,HIIRI Inc.
* All rights reserved.
* @file: carve_common_lib.cpp
* @brief: 简短说明文件功能、用途 (Comment)。
* @author:	minglu2
* @version: 1.0
* @date: 2018/12/26
* 
* @see
* 
* <b>版本记录：</b><br>
* <table>
*  <tr> <th>版本	<th>日期		<th>作者	<th>备注 </tr>
*  <tr> <td>1.0	    <td>2018/12/26	<td>minglu	<td>Create head file </tr>
* </table>
*****************************************************************/
#include "carve_common_lib.h"
#include <cstdlib>
#include "busin_log.h"
#include "CBaoyuan_Lib_Tool.h"
#include "CCarve.h"
#ifdef _WINDOWS
#define __CLASS_FUNCTION__ ((std::string(__FUNCTION__)).c_str()) 
#else
#define __CLASS_FUNCTION__ ((std::string("CCarve_Common_Lib_Tool::") + std::string(__FUNCTION__)).c_str()) 
#endif

bool CCarve_Common_Lib_Tool::init(string& str_kernel_err_reason)
{
	//初始化宝元库
	int nMakeId = 1111;
	string str_key = "79A08F845B1BCADC25EF0B396062EE91C06EB78EFFE16A7A";
	bool bSuccess = CBaoyuan_Lib::instance()->init(nMakeId, str_key, 100, 1000000);
	businlog_error_return_err_reason(bSuccess, __CLASS_FUNCTION__ << " | fail to init baoyuan lib, makeId:" 
		<< nMakeId << ", key:" << str_key, str_kernel_err_reason, false);
	businlog_crit("%s | init baoyuan lib successfully.", __CLASS_FUNCTION__);
	//TODO::初始化其他雕刻机函数库
	return true; 
}

bool CCarve_Common_Lib_Tool::is_baoyuan(int nfactory_type, const string& str_str_carve_type_key)
{
	businlog_error_return(nfactory_type >CARVE_FACTORY_TYPE_MIN && nfactory_type < CARVE_FACTORY_TYPE_MAX
		, ("%s | factory type is invalid:%d", __CLASS_FUNCTION__, nfactory_type), false);
	//TODO::临时这么写
	if (CARVE_FACTORY_TYPE_BAOYUAN == nfactory_type)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool CCarve_Common_Lib_Tool::connect(const Json::Value& json_conn_value, string& str_kernel_err_reason)
{
	//获取雕刻机厂商和设备型号信息
	int nfactory_type = CARVE_FACTORY_TYPE_MAX;
	string str_carve_type_key;
	businlog_error_return(get_carve_factory_and_type(json_conn_value, nfactory_type, str_carve_type_key, str_kernel_err_reason)
		, ("%s | fail to get carve factory and type, reason:%s"
		, __CLASS_FUNCTION__, str_kernel_err_reason.c_str()), false);
	if (is_baoyuan(nfactory_type, str_carve_type_key))
	{//是宝元
		return CBaoyuan_Lib::instance()->create_connection(json_conn_value, str_kernel_err_reason);
	}
	//TODO::判定是否为其他的库
	return false;
}

bool CCarve_Common_Lib_Tool::disconnect(const Json::Value& json_conn_value, string& str_kernel_err_reason)
{
	//获取雕刻机厂商和设备型号信息
	int nfactory_type = CARVE_FACTORY_TYPE_MAX;
	string str_carve_type_key;
	businlog_error_return(get_carve_factory_and_type(json_conn_value, nfactory_type, str_carve_type_key, str_kernel_err_reason)
		, ("%s | fail to get carve factory and type, reason:%s"
		, __CLASS_FUNCTION__, str_kernel_err_reason.c_str()), false);
	if (is_baoyuan(nfactory_type, str_carve_type_key))
	{//是宝元
		return CBaoyuan_Lib::instance()->disconnect(json_conn_value, str_kernel_err_reason);
	}
	//TODO::判定是否为其他的库
	return false;
}

bool CCarve_Common_Lib_Tool::upload_1file(const Json::Value& json_conn_value, string& str_kernel_err_reason)
{
	//获取雕刻机厂商和设备型号信息
	int nfactory_type = CARVE_FACTORY_TYPE_MAX;
	string str_carve_type_key;
	businlog_error_return(get_carve_factory_and_type(json_conn_value, nfactory_type, str_carve_type_key, str_kernel_err_reason)
		, ("%s | fail to get carve factory and type, reason:%s"
		, __CLASS_FUNCTION__, str_kernel_err_reason.c_str()), false);
	if (is_baoyuan(nfactory_type, str_carve_type_key))
	{//是宝元
		return CBaoyuan_Lib::instance()->upload_1file(json_conn_value, str_kernel_err_reason);
	}
	//TODO::判定是否为其他的库
	return false;
}

bool CCarve_Common_Lib_Tool::set_continue_status(const Json::Value& json_conn_value, string& str_kernel_err_reason)
{
	//获取雕刻机厂商和设备型号信息
	int nfactory_type = CARVE_FACTORY_TYPE_MAX;
	string str_carve_type_key;
	businlog_error_return(get_carve_factory_and_type(json_conn_value, nfactory_type, str_carve_type_key, str_kernel_err_reason)
		, ("%s | fail to get carve factory and type, reason:%s"
		, __CLASS_FUNCTION__, str_kernel_err_reason.c_str()), false);
	if (is_baoyuan(nfactory_type, str_carve_type_key))
	{//是宝元
		return CBaoyuan_Lib::instance()->set_continue_status(json_conn_value, str_kernel_err_reason);
	}
	//TODO::判定是否为其他的库
	return false;
}

bool CCarve_Common_Lib_Tool::reset_carve(const Json::Value& json_conn_value, string& str_kernel_err_reason)
{
	//获取雕刻机厂商和设备型号信息
	int nfactory_type = CARVE_FACTORY_TYPE_MAX;
	string str_carve_type_key;
	businlog_error_return(get_carve_factory_and_type(json_conn_value, nfactory_type, str_carve_type_key, str_kernel_err_reason)
		, ("%s | fail to get carve factory and type, reason:%s"
		, __CLASS_FUNCTION__, str_kernel_err_reason.c_str()), false);
	if (is_baoyuan(nfactory_type, str_carve_type_key))
	{//是宝元
		return CBaoyuan_Lib::instance()->reset_carve(json_conn_value, str_kernel_err_reason);
	}
	//TODO::判定是否为其他的库
	return false;
}

bool CCarve_Common_Lib_Tool::pause(const Json::Value& json_conn_value, string& str_kernel_err_reason)
{
	//获取雕刻机厂商和设备型号信息
	int nfactory_type = CARVE_FACTORY_TYPE_MAX;
	string str_carve_type_key;
	businlog_error_return(get_carve_factory_and_type(json_conn_value, nfactory_type, str_carve_type_key, str_kernel_err_reason)
		, ("%s | fail to get carve factory and type, reason:%s"
		, __CLASS_FUNCTION__, str_kernel_err_reason.c_str()), false);
	if (is_baoyuan(nfactory_type, str_carve_type_key))
	{//是宝元
		return CBaoyuan_Lib::instance()->pause(json_conn_value, str_kernel_err_reason);
	}
	//TODO::判定是否为其他的库
	return false;
}

bool CCarve_Common_Lib_Tool::start(const Json::Value& json_conn_value, string& str_kernel_err_reason)
{
	//获取雕刻机厂商和设备型号信息
	int nfactory_type = CARVE_FACTORY_TYPE_MAX;
	string str_carve_type_key;
	businlog_error_return(get_carve_factory_and_type(json_conn_value, nfactory_type, str_carve_type_key, str_kernel_err_reason)
		, ("%s | fail to get carve factory and type, reason:%s"
		, __CLASS_FUNCTION__, str_kernel_err_reason.c_str()), false);
	if (is_baoyuan(nfactory_type, str_carve_type_key))
	{//是宝元
		return CBaoyuan_Lib::instance()->start(json_conn_value, str_kernel_err_reason);
	}
	//TODO::判定是否为其他的库
	return false;
}

bool CCarve_Common_Lib_Tool::get_carve_status(const Json::Value& json_conn_value, ECARVE_STATUS_TYPE& eCommon_carver_status, string& str_kernel_err_reason)
{
	//获取雕刻机厂商和设备型号信息
	int nfactory_type = CARVE_FACTORY_TYPE_MAX;
	string str_carve_type_key;
	bool bSuccess = get_carve_factory_and_type(json_conn_value, nfactory_type, str_carve_type_key, str_kernel_err_reason);
	businlog_error_return(bSuccess, ("%s | fail to get carve factory and type, reason:%s"
		, __CLASS_FUNCTION__, str_kernel_err_reason.c_str()), false);

	if (is_baoyuan(nfactory_type, str_carve_type_key))
	{//是宝元
		int nStatus_baoyuan = 0;
		bSuccess = CBaoyuan_Lib::instance()->get_carve_status(json_conn_value, nStatus_baoyuan, str_kernel_err_reason);
		businlog_error_return(bSuccess, ("%s | failed to get carve status of baoyuan, reason:%s", __CLASS_FUNCTION__, str_kernel_err_reason.c_str()), false)
	    //转换状态码
		bSuccess = parse_baoyuan_carve_status_to_common(nStatus_baoyuan, eCommon_carver_status, str_kernel_err_reason);
		businlog_error_return(bSuccess, ("%s | fail to parse baoyuan:%d carve status to common status, reason:%s"
			, __CLASS_FUNCTION__, nStatus_baoyuan, str_kernel_err_reason.c_str()), false);
		return true;
	}
	//TODO::判定是否为其他的库
	return false;
}

bool CCarve_Common_Lib_Tool::get_carve_status_description(const ECARVE_STATUS_TYPE& eCommon_carver_status, string& str_carve_status_description, string& str_kernel_err_reason)
{
	str_carve_status_description += "Carve ";
	switch (eCommon_carver_status)
	{
	case CARVE_STATUS_OFFLINE:
		str_carve_status_description += "Offline";
		break;
	case CARVE_STATUS_ONLINE:
		str_carve_status_description += "Online";
		break;
	case CARVE_STATUS_NOT_READY:
		str_carve_status_description += "Not Ready";
		break;
	case CARVE_STATUS_READY:
		str_carve_status_description += "Ready";
		break;
	case CARVE_STATUS_ENGRAVING:
		str_carve_status_description += "Engraving";
		break;
	case CARVE_STATUS_PAUSE:
		str_carve_status_description += "Pause";
		break;
	case CARVE_STATUS_COMPLETED:
		str_carve_status_description += "Completed";
		break;
	case CARVE_STATUS_ERR:
		str_carve_status_description += "Error";
		break;
	default:
		businlog_error_return_err_reason(false, __CLASS_FUNCTION__ << " | Not support carve common status:" << eCommon_carver_status 
			<< ", should be [" << CARVE_STATUS_MIN + 1 << "," << CARVE_STATUS_MAX -1 << "]", str_kernel_err_reason, false);
	}
	return true;
}

bool CCarve_Common_Lib_Tool::stop_fast(const Json::Value& json_conn_value, string& str_kernel_err_reason)
{
	//获取雕刻机厂商和设备型号信息
	int nfactory_type = CARVE_FACTORY_TYPE_MAX;
	string str_carve_type_key;
	businlog_error_return(get_carve_factory_and_type(json_conn_value, nfactory_type, str_carve_type_key, str_kernel_err_reason)
		, ("%s | fail to get carve factory and type, reason:%s"
		, __CLASS_FUNCTION__, str_kernel_err_reason.c_str()), false);
	if (is_baoyuan(nfactory_type, str_carve_type_key))
	{//是宝元
		return CBaoyuan_Lib::instance()->stop_fast(json_conn_value, str_kernel_err_reason);
	}
	//TODO::判定是否为其他的库
	return false;
}

bool CCarve_Common_Lib_Tool::cancel_fast_stop(const Json::Value& json_conn_value, string& str_kernel_err_reason)
{
	//获取雕刻机厂商和设备型号信息
	int nfactory_type = CARVE_FACTORY_TYPE_MAX;
	string str_carve_type_key;
	businlog_error_return(get_carve_factory_and_type(json_conn_value, nfactory_type, str_carve_type_key, str_kernel_err_reason)
		, ("%s | fail to get carve factory and type, reason:%s"
		, __CLASS_FUNCTION__, str_kernel_err_reason.c_str()), false);
	if (is_baoyuan(nfactory_type, str_carve_type_key))
	{//是宝元
		return CBaoyuan_Lib::instance()->cancel_fast_stop(json_conn_value, str_kernel_err_reason);
	}
	//TODO::判定是否为其他的库
	return false;
}

bool CCarve_Common_Lib_Tool::delete_1file(const Json::Value& json_conn_value, string& str_kernel_err_reason)
{
	//获取雕刻机厂商和设备型号信息
	int nfactory_type = CARVE_FACTORY_TYPE_MAX;
	string str_carve_type_key;
	businlog_error_return(get_carve_factory_and_type(json_conn_value, nfactory_type, str_carve_type_key, str_kernel_err_reason)
		, ("%s | fail to get carve factory and type, reason:%s"
		, __CLASS_FUNCTION__, str_kernel_err_reason.c_str()), false);
	if (is_baoyuan(nfactory_type, str_carve_type_key))
	{//是宝元
		return CBaoyuan_Lib::instance()->delete_1file(json_conn_value, str_kernel_err_reason);
	}
	//TODO::判定是否为其他的库
	return false;
}

bool CCarve_Common_Lib_Tool::get_carve_factory_and_type(const Json::Value& json_conn_value, int& nfactory_type, string& str_str_carve_type_key, string& str_kernel_err_reason)
{
	//获取雕刻机厂商
	businlog_error_return_err_reason(json_conn_value.isMember(CCarve::ms_str_factory_type_key), __CLASS_FUNCTION__ << " | json:" 
		<< json_conn_value.toStyledString() << " without key:" <<  CCarve::ms_str_factory_type_key, str_kernel_err_reason, false);
	nfactory_type = json_conn_value[CCarve::ms_str_factory_type_key].asInt();
	//获取设备型号
	businlog_error_return_err_reason(json_conn_value.isMember(CCarve::ms_str_carve_type_key), __CLASS_FUNCTION__ << " | json:" 
		<< json_conn_value.toStyledString() << " without key:" <<  CCarve::ms_str_carve_type_key, str_kernel_err_reason, false);
	str_str_carve_type_key = json_conn_value[CCarve::ms_str_carve_type_key].asString();
	return true;
}

CCarve_Common_Lib_Tool::~CCarve_Common_Lib_Tool()
{
	//这个对象必须在其他雕刻机函数库对象析构后再析构。
	businlog_warn("%s | release source, must be released after other carve libs.", __CLASS_FUNCTION__);
	//无需去逆初始化那些真正的雕刻机函数库，他们自己应该在析构时，释放自己的资源。
}

/************************************
* Method:    parse_baoyuan_carve_status_to_common
* Brief:  将宝元雕刻机的状态码转换为通用的雕刻机状态码
* Access:    protected 
* Returns:   bool
* Qualifier:
*Parameter: int nBaoyuan_carve_status -[in/out]  
*Parameter: ECARVE_STATUS_TYPE & eCarve_common_status -[in/out]  
*Parameter: string & str_kernel_err_reason -[in/out]  
************************************/
bool CCarve_Common_Lib_Tool::parse_baoyuan_carve_status_to_common(int nBaoyuan_carve_status, ECARVE_STATUS_TYPE& eCarve_common_status, string& str_kernel_err_reason)
{
	switch (nBaoyuan_carve_status)
	{
	case 0: //未就绪
		eCarve_common_status = CARVE_STATUS_NOT_READY; 
		break;
	case 1:
		eCarve_common_status = CARVE_STATUS_READY;
		break;
	case 2:
		eCarve_common_status = CARVE_STATUS_ENGRAVING;
		break;
	case 3:
		eCarve_common_status = CARVE_STATUS_PAUSE;
		break;
	case 4:
		eCarve_common_status = CARVE_STATUS_ERR;
		break;
	case 5:
		eCarve_common_status = CARVE_STATUS_COMPLETED;
		break;
	default:
		businlog_error_return_err_reason(false, __CLASS_FUNCTION__ <<" | Not support baoyuan carve status:" 
			<< nBaoyuan_carve_status, str_kernel_err_reason, false);
	}
	return true;
}

CCarve_Common_Lib_Tool::CCarve_Common_Lib_Tool()
{

}

CCarve_Common_Lib_Tool::CGarbo CCarve_Common_Lib_Tool::Garbo;

CCarve_Common_Lib_Tool* CCarve_Common_Lib_Tool::ms_pInstance = new CCarve_Common_Lib_Tool();

CCarve_Common_Lib_Tool::CGarbo::~CGarbo()
{
	if (NULL != CCarve_Common_Lib_Tool::ms_pInstance)
	{
		delete CCarve_Common_Lib_Tool::ms_pInstance;
		CCarve_Common_Lib_Tool::ms_pInstance = NULL;
	}
}
