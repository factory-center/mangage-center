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
#include "../source/CSpdLog.h"
#include "CBaoyuan_Lib_Tool.h"
#include "CCarve.h"
#include "utils/msp_errors.h"
#ifdef _WINDOWS
#define __CLASS_FUNCTION__ ((std::string(__FUNCTION__)).c_str()) 
#else
#define __CLASS_FUNCTION__ ((std::string("CCarve_Common_Lib_Tool::") + std::string(__FUNCTION__)).c_str()) 
#endif
//TODO::后面新增读取配置文件并且构建json参数
bool CCarve_Common_Lib_Tool::init(string& str_kernel_err_reason)
{
	//初始化宝元库
	int nMakeId = 1111;
	string str_key = "79A08F845B1BCADC25EF0B396062EE91C06EB78EFFE16A7A";
	bool bSuccess = CBaoyuan_Lib::instance()->init(nMakeId, str_key, 100, 4000000);
	if (bSuccess == false)
	{
		LError("fail to init baoyuan lib, makeId:{}, key:{}", nMakeId, str_key);
		return false;
	}

	LCritical("init baoyuan lib successfully.");
	//TODO::初始化其他雕刻机函数库
	return true; 
}

bool CCarve_Common_Lib_Tool::is_baoyuan(int nfactory_type, const string& str_carve_type_key)
{
	if (!(nfactory_type > CARVE_FACTORY_TYPE_MIN && nfactory_type < CARVE_FACTORY_TYPE_MAX))
	{
		LError("factory type is invalid:{}", nfactory_type);
		return false;
	}

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

bool CCarve_Common_Lib_Tool::connect(const Json::Value& json_conn_value, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	//获取雕刻机厂商和设备型号信息
	int nfactory_type = CARVE_FACTORY_TYPE_MAX;
	string str_carve_type_key;
	bool bSuccess = get_carve_factory_and_type(json_conn_value, nfactory_type
		, str_carve_type_key, str_err_reason_for_debug, str_err_reason_for_user);
	if (bSuccess == false)
	{
		LError("fail to get carve factory and type, reason:{}", str_err_reason_for_debug);
		return false;
	}

	if (is_baoyuan(nfactory_type, str_carve_type_key))
	{//是宝元
		return CBaoyuan_Lib::instance()->create_connection(json_conn_value, str_err_reason_for_debug, str_err_reason_for_user);
	}
	//TODO::判定是否为其他的库
	return false;
}

bool CCarve_Common_Lib_Tool::disconnect(const Json::Value& json_conn_value, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	//获取雕刻机厂商和设备型号信息
	int nfactory_type = CARVE_FACTORY_TYPE_MAX;
	string str_carve_type_key;
	if (!get_carve_factory_and_type(json_conn_value, nfactory_type, str_carve_type_key, str_err_reason_for_debug, str_err_reason_for_user))
	{
		LError("fail to get carve factory and type, reason:{}", str_err_reason_for_debug);
		return false;
	}

	if (is_baoyuan(nfactory_type, str_carve_type_key))
	{//是宝元
		return CBaoyuan_Lib::instance()->disconnect(json_conn_value, str_err_reason_for_debug, str_err_reason_for_user);
	}
	//TODO::判定是否为其他的库
	return false;
}

bool CCarve_Common_Lib_Tool::upload_1file(const Json::Value& json_conn_value, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	//获取雕刻机厂商和设备型号信息
	int nfactory_type = CARVE_FACTORY_TYPE_MAX;
	string str_carve_type_key;
	if (!get_carve_factory_and_type(json_conn_value, nfactory_type, str_carve_type_key, str_err_reason_for_debug, str_err_reason_for_user))
	{
		LError("fail to get carve factory and type, reason:{}", str_err_reason_for_debug);
		return false;
	}

	if (is_baoyuan(nfactory_type, str_carve_type_key))
	{//是宝元
		return CBaoyuan_Lib::instance()->upload_1file(json_conn_value, str_err_reason_for_debug, str_err_reason_for_user);
	}
	//TODO::判定是否为其他的库
	return false;
}

bool CCarve_Common_Lib_Tool::set_continue_status(const Json::Value& json_conn_value, string& str_err_reason_for_debug, string& str__err_reason_for_user)
{
	//获取雕刻机厂商和设备型号信息
	int nfactory_type = CARVE_FACTORY_TYPE_MAX;
	string str_carve_type_key;
	bool bSuccess = get_carve_factory_and_type(json_conn_value, nfactory_type, str_carve_type_key, str_err_reason_for_debug, str__err_reason_for_user);
	if (bSuccess == false)
	{
		LError("fail to get carve factory and type, reason:{}", str_err_reason_for_debug);
		return false;
	}

	if (is_baoyuan(nfactory_type, str_carve_type_key))
	{//是宝元
		return CBaoyuan_Lib::instance()->set_continue_status(json_conn_value, str_err_reason_for_debug, str__err_reason_for_user);
	}
	//TODO::判定是否为其他的库
	return false;
}

bool CCarve_Common_Lib_Tool::reset_carve(const Json::Value& json_conn_value, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	//获取雕刻机厂商和设备型号信息
	int nfactory_type = CARVE_FACTORY_TYPE_MAX;
	string str_carve_type_key;
	bool bSuccess = get_carve_factory_and_type(json_conn_value, nfactory_type, str_carve_type_key
		, str_err_reason_for_debug, str_err_reason_for_user);
	if (bSuccess == false)
	{
		LError("fail to get carve factory and type, reason:{}", str_err_reason_for_debug);
		return false;
	}
	if (is_baoyuan(nfactory_type, str_carve_type_key))
	{//是宝元
		return CBaoyuan_Lib::instance()->reset_carve(json_conn_value, str_err_reason_for_debug, str_err_reason_for_user);
	}
	//TODO::判定是否为其他的库
	return false;
}

bool CCarve_Common_Lib_Tool::pause(const Json::Value& json_conn_value, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	//获取雕刻机厂商和设备型号信息
	int nfactory_type = CARVE_FACTORY_TYPE_MAX;
	string str_carve_type_key;
	bool bSuccess = get_carve_factory_and_type(json_conn_value, nfactory_type
		, str_carve_type_key, str_err_reason_for_debug, str_err_reason_for_user);
	if (bSuccess == false)
	{
		LError("fail to get carve factory and type, reason:{}", str_err_reason_for_debug);
		return false;
	}
	if (is_baoyuan(nfactory_type, str_carve_type_key))
	{//是宝元
		return CBaoyuan_Lib::instance()->pause(json_conn_value, str_err_reason_for_debug, str_err_reason_for_user);
	}
	//TODO::判定是否为其他的库
	return false;
}

bool CCarve_Common_Lib_Tool::start(const Json::Value& json_conn_value, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	//获取雕刻机厂商和设备型号信息
	int nfactory_type = CARVE_FACTORY_TYPE_MAX;
	string str_carve_type_key;
	bool bSuccess = get_carve_factory_and_type(json_conn_value, nfactory_type, str_carve_type_key
		, str_err_reason_for_debug, str_err_reason_for_user);
	if (bSuccess == false)
	{
		LError("fail to get carve factory and type, reason:{}", str_err_reason_for_debug);
		return false;
	}
	if (is_baoyuan(nfactory_type, str_carve_type_key))
	{//是宝元
		return CBaoyuan_Lib::instance()->start(json_conn_value, str_err_reason_for_debug, str_err_reason_for_user);
	}
	//TODO::判定是否为其他的库
	return false;
}

bool CCarve_Common_Lib_Tool::get_carve_status(const Json::Value& json_conn_value, ECARVE_STATUS_TYPE& eCommon_carver_status, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	//获取雕刻机厂商和设备型号信息
	int nfactory_type = CARVE_FACTORY_TYPE_MAX;
	string str_carve_type_key;
	bool bSuccess = get_carve_factory_and_type(json_conn_value, nfactory_type, str_carve_type_key, str_err_reason_for_debug, str_err_reason_for_user);
	if (bSuccess == false)
	{
		LError("fail to get carve factory and type, reason:{}", str_err_reason_for_debug);
		return false;
	}

	if (is_baoyuan(nfactory_type, str_carve_type_key))
	{//是宝元
		int nStatus_baoyuan = 0;
		bSuccess = CBaoyuan_Lib::instance()->get_carve_status(json_conn_value, nStatus_baoyuan, str_err_reason_for_debug, str_err_reason_for_user);
		if (bSuccess == false)
		{
			LError("failed to get carve status of baoyuan, reason:{}", str_err_reason_for_debug);
			return false;
		}
	    //转换状态码
		bSuccess = parse_baoyuan_carve_status_to_common(nStatus_baoyuan, eCommon_carver_status, str_err_reason_for_debug, str_err_reason_for_user);
		if (bSuccess == false)
		{
			LError("fail to parse baoyuan:{} carve status to common status, reason:{}", nStatus_baoyuan, str_err_reason_for_debug);
			return false;
		}
		return true;
	}
	//TODO::判定是否为其他的库
	return false;
}

bool CCarve_Common_Lib_Tool::get_carve_status_description(const ECARVE_STATUS_TYPE& eCommon_carver_status
	, string& str_carve_status_description, string& str_err_reason_for_debug, string& str_err_reason_for_user)
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
		LError("Not support carve common status:{}, should be [{},{}]", eCommon_carver_status, CARVE_STATUS_MIN + 1, CARVE_STATUS_MAX - 1);
		str_err_reason_for_debug = "Not support carve common status : "+ std::to_string(eCommon_carver_status)
			+ ", should be["+ std::to_string(CARVE_STATUS_MIN + 1) +", "+ std::to_string(CARVE_STATUS_MAX - 1) +"]";
		str_err_reason_for_user = "错误的设备状态";
		return false;
	}
	return true;
}

bool CCarve_Common_Lib_Tool::stop_fast(const Json::Value& json_conn_value
				, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	//获取雕刻机厂商和设备型号信息
	int nfactory_type = CARVE_FACTORY_TYPE_MAX;
	string str_carve_type_key;
	bool bSuccess = get_carve_factory_and_type(json_conn_value, nfactory_type
		, str_carve_type_key, str_err_reason_for_debug, str_err_reason_for_user);
	if (bSuccess == false)
	{
		LError("fail to get carve factory and type, reason:{}", str_err_reason_for_debug);
		return false;
	}

	if (is_baoyuan(nfactory_type, str_carve_type_key))
	{//是宝元
		return CBaoyuan_Lib::instance()->stop_fast(json_conn_value, str_err_reason_for_debug, str_err_reason_for_user);
	}
	//TODO::判定是否为其他的库
	return false;
}

bool CCarve_Common_Lib_Tool::cancel_fast_stop(const Json::Value& json_conn_value, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	//获取雕刻机厂商和设备型号信息
	int nfactory_type = CARVE_FACTORY_TYPE_MAX;
	string str_carve_type_key;
	bool bSuccess = get_carve_factory_and_type(json_conn_value, nfactory_type
		, str_carve_type_key, str_err_reason_for_debug, str_err_reason_for_user);
	if (bSuccess == false)
	{
		LError("fail to get carve factory and type, reason:{}", str_err_reason_for_debug);
		return false;
	}
	if (is_baoyuan(nfactory_type, str_carve_type_key))
	{//是宝元
		return CBaoyuan_Lib::instance()->cancel_fast_stop(json_conn_value, str_err_reason_for_debug, str_err_reason_for_user);
	}
	//TODO::判定是否为其他的库
	return false;
}

bool CCarve_Common_Lib_Tool::delete_1file(const Json::Value& json_conn_value, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	//获取雕刻机厂商和设备型号信息
	int nfactory_type = CARVE_FACTORY_TYPE_MAX;
	string str_carve_type_key;
	bool bSuccess = get_carve_factory_and_type(json_conn_value, nfactory_type
		, str_carve_type_key, str_err_reason_for_debug, str_err_reason_for_user);
	if (bSuccess == false)
	{
		LError("fail to get carve factory and type, reason:{}", str_err_reason_for_debug);
		return false;
	}
	if (is_baoyuan(nfactory_type, str_carve_type_key))
	{//是宝元
		return CBaoyuan_Lib::instance()->delete_1file(json_conn_value, str_err_reason_for_debug, str_err_reason_for_user);
	}
	//TODO::判定是否为其他的库
	return false;
}

bool CCarve_Common_Lib_Tool::get_current_line_num(const Json::Value& json_conn_value, int& nCurrent_line_num, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	//获取雕刻机厂商和设备型号信息
	int nfactory_type = CARVE_FACTORY_TYPE_MAX;
	string str_carve_type_key;
	bool bSuccess = get_carve_factory_and_type(json_conn_value, nfactory_type, str_carve_type_key, str_err_reason_for_debug, str_err_reason_for_user);
	if (bSuccess == false)
	{
		LError("fail to get carve factory and type, reason:{}", str_err_reason_for_debug);
		return false;
	}

	if (is_baoyuan(nfactory_type, str_carve_type_key))
	{//是宝元
		 return CBaoyuan_Lib::instance()->get_current_line_num(json_conn_value, nCurrent_line_num, str_err_reason_for_debug, str_err_reason_for_user);
	}
	//TODO::判定是否为其他的库
	return false;
}
bool CCarve_Common_Lib_Tool::adjust_speed(const Json::Value& json_conn_value, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	//获取雕刻机厂商和设备型号信息
	int nfactory_type = CARVE_FACTORY_TYPE_MAX;
	string str_carve_type_key;
	bool bSuccess = get_carve_factory_and_type(json_conn_value, nfactory_type, str_carve_type_key, str_err_reason_for_debug, str_err_reason_for_user);
	if (bSuccess == false)
	{
		LError("fail to get carve factory and type, reason:{}", str_err_reason_for_debug);
		return false;
	}
	if (is_baoyuan(nfactory_type, str_carve_type_key))
	{//是宝元
		return CBaoyuan_Lib::instance()->adjust_speed(json_conn_value, str_err_reason_for_debug, str_err_reason_for_user);
	}
	//TODO::判定是否为其他的库
	return false;
}

bool CCarve_Common_Lib_Tool::get_engraving_time(const Json::Value& json_conn_value, size_t& nTotal_engraving_time_minute, size_t& nSingle_engraving_time_minute, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	//获取雕刻机厂商和设备型号信息
	int nfactory_type = CARVE_FACTORY_TYPE_MAX;
	string str_carve_type_key;
	bool bSuccess = get_carve_factory_and_type(json_conn_value, nfactory_type, str_carve_type_key, str_err_reason_for_debug, str_err_reason_for_user);
	if (bSuccess == false)
	{
		LError("fail to get carve factory and type, reason:{}", str_err_reason_for_debug);
		return false;
	}

	if (is_baoyuan(nfactory_type, str_carve_type_key))
	{//是宝元
		return CBaoyuan_Lib::instance()->get_engraving_time(json_conn_value, nTotal_engraving_time_minute, nSingle_engraving_time_minute, str_err_reason_for_debug, str_err_reason_for_user);
	}
	else
	{
		//TODO::判定是否为其他的库
	}
	return false;
}

/************************************
* Method:    acquire_resource
* Brief:  申请资源，并放入josn中
* Access:    public 
* Returns:   bool
* Qualifier:
*Parameter: Json::Value & json_conn_value -[in/out]  
*Parameter: string & str_err_reason_for_debug -[in/out]  
*Parameter: string & str_err_reason_for_user -[in/out]  
************************************/
bool CCarve_Common_Lib_Tool::acquire_resource(Json::Value& json_conn_value, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	//获取雕刻机厂商和设备型号信息
	int nfactory_type = CARVE_FACTORY_TYPE_MAX;
	string str_carve_type_key;
	bool bSuccess = get_carve_factory_and_type(json_conn_value, nfactory_type
		, str_carve_type_key, str_err_reason_for_debug, str_err_reason_for_user);
	if (bSuccess == false)
	{
		LError("fail to get carve factory and type, reason:{}", str_err_reason_for_debug);
		return false;
	}
	if (is_baoyuan(nfactory_type, str_carve_type_key))
	{
		//从宝元库申请索引
		int nConn_idx = -2;
		bool bSuccess = CBaoyuan_Lib::instance()->acquire_conn_idx(nConn_idx, str_err_reason_for_debug, str_err_reason_for_user);
		if (bSuccess == false)
		{
			LError("fail to acquire conn index, reason:{}", str_err_reason_for_debug);
			return false;
		}
		json_conn_value[CCarve::ms_str_conn_idx_key] = nConn_idx;
		return true;
	}
	//TODO::判定是否为其他的库
	return false;
}

bool CCarve_Common_Lib_Tool::release_resource(const Json::Value& json_conn_value, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	//获取雕刻机厂商和设备型号信息
	int nfactory_type = CARVE_FACTORY_TYPE_MAX;
	string str_carve_type_key;
	if (!get_carve_factory_and_type(json_conn_value, nfactory_type, str_carve_type_key, str_err_reason_for_debug, str_err_reason_for_user))
	{
		LError("fail to get carve factory and type, reason:{}", str_err_reason_for_debug);
		return false;
	}

	if (is_baoyuan(nfactory_type, str_carve_type_key))
	{
		//释放索引
		return CBaoyuan_Lib::instance()->release_conn_idx(json_conn_value, str_err_reason_for_debug, str_err_reason_for_user);
	}
	//TODO::判定是否为其他的库
	return false;
}

bool CCarve_Common_Lib_Tool::get_carve_factory_and_type(const Json::Value& json_conn_value, int& nfactory_type, string& str_str_carve_type_key, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	//获取雕刻机厂商
	if (!json_conn_value.isMember(CCarve::ms_str_factory_type_key))
	{
		LError("json:{}, without key:{}", json_conn_value.toStyledString(), CCarve::ms_str_factory_type_key);
		str_err_reason_for_debug = "json:" + json_conn_value.toStyledString() + ", without key:" + CCarve::ms_str_factory_type_key;
		str_err_reason_for_user = "参数中不含有厂商类型或者设备类型对应的键值名不为" + CCarve::ms_str_factory_type_key;
		return false;
	}
	nfactory_type = json_conn_value[CCarve::ms_str_factory_type_key].asInt();

	//获取设备型号
	if (!json_conn_value.isMember(CCarve::ms_str_carve_type_key))
	{
		LError("json:{}, without key:{}", json_conn_value.toStyledString(), CCarve::ms_str_carve_type_key);
		str_err_reason_for_debug = "json:" + json_conn_value.toStyledString() + ", without key:" + CCarve::ms_str_carve_type_key;
		str_err_reason_for_user = "参数中不含有设备类型或者设备类型对应的键值名不为" + CCarve::ms_str_carve_type_key;
		return false;
	}
	str_str_carve_type_key = json_conn_value[CCarve::ms_str_carve_type_key].asString();
	return true;
}

CCarve_Common_Lib_Tool::~CCarve_Common_Lib_Tool()
{
	//这个对象必须在其他雕刻机函数库对象析构后再析构。
	LWarn("release source, must be released after other carve libs.");
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
bool CCarve_Common_Lib_Tool::parse_baoyuan_carve_status_to_common(int nBaoyuan_carve_status, ECARVE_STATUS_TYPE& eCarve_common_status, string& str_err_reason_for_debug, string& str_err_reason_for_user)
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
		LError("Not support baoyuan carve status:{}", nBaoyuan_carve_status);
		str_err_reason_for_debug = "Not support baoyuan carve status:" + std::to_string(nBaoyuan_carve_status);
		str_err_reason_for_user = "不支持的雕刻机状态";
		return false;
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
