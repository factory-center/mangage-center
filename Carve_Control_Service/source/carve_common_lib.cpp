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
