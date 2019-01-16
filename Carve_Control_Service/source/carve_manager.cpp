/******************************************************************
* Copyright (c) 2016-2017,HIIRI Inc.
* All rights reserved.
* @file: carve_manager.cpp
* @brief: 简短说明文件功能、用途 (Comment)。
* @author:	minglu2
* @version: 1.0
* @date: 2019/01/07
* 
* @see
* 
* <b>版本记录：</b><br>
* <table>
*  <tr> <th>版本	<th>日期		<th>作者	<th>备注 </tr>
*  <tr> <td>1.0	    <td>2019/01/07	<td>minglu	<td>Create head file </tr>
* </table>
*****************************************************************/
#include "carve_manager.h"
#include <cstdlib>
#include "busin_log.h"
#include "CCarve.h"
#include <memory>
#include "utils/msp_errors.h"
#ifdef _WINDOWS
#define __CLASS_FUNCTION__ ((std::string(__FUNCTION__)).c_str()) 
#else
#define __CLASS_FUNCTION__ ((std::string("CCarve_Manager::") + std::string(__FUNCTION__)).c_str()) 
#endif
CCarve_Manager* CCarve_Manager::instance()
{
	return ms_pInstance;
}

int CCarve_Manager::connect_carve(const Json::Value& json_params, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	try
	{
		//从参数中获取设备编号
		businlog_error_return_debug_and_user_reason(json_params.isMember(CCarve::ms_str_carve_id_key), __CLASS_FUNCTION__
			<< " | json:" << json_params.toStyledString() << ", without key:" << CCarve::ms_str_carve_id_key
			, str_err_reason_for_debug, "参数错误", str_err_reason_for_user, MSP_ERROR_INVALID_PARA);

		const string& str_carve_id = json_params[CCarve::ms_str_carve_id_key].asString();

		boost::shared_ptr<CCarve> ptr_carve;
		std::pair<TYPE_MAP_ITER, bool> pair_insert_result;
		pair_insert_result.second = false;
		//自我调用时的参数
		Json::Value json_local_params = json_params;

		int ret = 0;

		{
			Thread_Write_Lock guard(m_rw_carveId_carvePtr);
			//判定此编号对应的雕刻机是否已经存在
			businlog_error_return_debug_and_user_reason(m_map_carveId_carvePtr.find(str_carve_id) == m_map_carveId_carvePtr.end()
				, __CLASS_FUNCTION__ << " | carve id:" << str_carve_id << " already exist.", str_err_reason_for_debug
				, "设备编号对应的设备已经连接", str_err_reason_for_user, MSP_ERROR_ALREADY_EXIST);
			//map中不存在对应的设备编号
			//构造雕刻机对象
			ptr_carve = boost::make_shared<CCarve>(json_params);
			//申请资源，如果失败则直接返回
			ret = ptr_carve->acquire_resource(str_err_reason_for_debug, str_err_reason_for_user);
			businlog_error_return(!ret, ("%s | fail to acquire resource, reason:%s."
				, __CLASS_FUNCTION__, str_err_reason_for_debug.c_str()), ret);
			pair_insert_result = m_map_carveId_carvePtr.insert(std::make_pair(str_carve_id, ptr_carve));
			//判定插入结果，如果失败，则说明键已经存在了.（map插入失败，是不会覆盖原来的value）
			businlog_error_return_debug_and_user_reason(pair_insert_result.second, __CLASS_FUNCTION__ << " | fail to insert, carve id:" << str_carve_id << " alread exist."
				, str_err_reason_for_debug, "重复连接设备", str_err_reason_for_user, MSP_ERROR_INVALID_PARA);
		}
		//所有关于雕刻机的操作，如果有一个出错，则将其从map中删除，并出错返回
		//连接雕刻机
		ret = ptr_carve->connect(str_err_reason_for_debug, str_err_reason_for_user);
		if (ret)
		{
			businlog_error("%s | fail to connect carve, reason:%s.", __CLASS_FUNCTION__, str_err_reason_for_debug.c_str());
			goto Err_exit;
		}

		//设置continue状态
		json_local_params[CCarve::ms_str_status_key] = 0;
		ret = ptr_carve->set_continue_status(json_local_params, str_err_reason_for_debug, str_err_reason_for_user);
		if (ret)
		{
			businlog_error("%s | fail to set carve continue status, reason:%s.", __CLASS_FUNCTION__, str_err_reason_for_debug.c_str());
			goto Err_exit;
		}

		//重置设备
		ret = ptr_carve->reset(json_local_params, str_err_reason_for_debug, str_err_reason_for_user);
		if (ret)
		{
			businlog_error("%s | fail to reset carve, reason:%s.", __CLASS_FUNCTION__, str_err_reason_for_debug.c_str());
			goto Err_exit;
		}
		return MSP_SUCCESS;
Err_exit: //仅当对雕刻机操作出错了，则将雕刻机移除
		//如果之前添加成功了，则将其移除
		if(pair_insert_result.second)
		{
			//移除（资源、连接等交给其析构函数来调用）
			Thread_Write_Lock guard(m_rw_carveId_carvePtr);
			m_map_carveId_carvePtr.erase(pair_insert_result.first);
		}
		return ret;
	}
	catch (std::exception& e)
	{
		str_err_reason_for_debug = string("Has exception:") + string(e.what());
		str_err_reason_for_user = "服务端发生异常";
		businlog_error("%s | err reason:%s.", __CLASS_FUNCTION__, str_err_reason_for_debug.c_str());
		return MSP_ERROR_EXCEPTION;
	}
}

int CCarve_Manager::disconnect_carve(const Json::Value& json_params, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	try
	{
		//从参数中获取设备编号
		businlog_error_return_debug_and_user_reason(json_params.isMember(CCarve::ms_str_carve_id_key), __CLASS_FUNCTION__
			<< " | json:" << json_params.toStyledString() << ", without key:" << CCarve::ms_str_carve_id_key
			, str_err_reason_for_debug, "参数错误", str_err_reason_for_user, MSP_ERROR_INVALID_PARA);
		
		const string& str_carve_id = json_params[CCarve::ms_str_carve_id_key].asString();

		int ret = 0;

		{
			Thread_Write_Lock guard(m_rw_carveId_carvePtr);
			TYPE_MAP_ITER iter = m_map_carveId_carvePtr.find(str_carve_id);

			//判定此编号对应的雕刻机是否已经存在,如果不存在，直接返回
			businlog_error_return_debug_and_user_reason(iter!= m_map_carveId_carvePtr.end()
				, __CLASS_FUNCTION__ << " | carve id:" << str_carve_id << " not exist.", str_err_reason_for_debug
				, "设备未连接", str_err_reason_for_user, MSP_ERROR_NOT_FOUND);

			//断开设备
			ret = iter->second->disconnect(str_err_reason_for_debug, str_err_reason_for_user);
			businlog_error_return(!ret, ("%s | fail to disconnect, reason:%s."
				, __CLASS_FUNCTION__, str_err_reason_for_debug.c_str()), ret);

			//释放资源
			ret = iter->second->release_resource(str_err_reason_for_debug,str_err_reason_for_user);
			businlog_error_return(!ret, ("%s | fail to release resource, reason:%s."
				, __CLASS_FUNCTION__, str_err_reason_for_debug.c_str()), ret);

			//从map中移除对应的断开设备
			m_map_carveId_carvePtr.erase(iter);
		}
		
		return MSP_SUCCESS;
	}
	catch (std::exception& e)
	{
		str_err_reason_for_debug = string("Has exception:") + string(e.what());
		str_err_reason_for_user = "服务端发生异常";
		businlog_error("%s | err reason:%s.", __CLASS_FUNCTION__, str_err_reason_for_debug.c_str());
		return MSP_ERROR_EXCEPTION;
	}
}

int CCarve_Manager::get_carve_status(const Json::Value& json_params, ECARVE_STATUS_TYPE& eCarve_common_status, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	try
	{
		//从参数中获取设备编号
		businlog_error_return_debug_and_user_reason(json_params.isMember(CCarve::ms_str_carve_id_key), __CLASS_FUNCTION__
			<< " | json:" << json_params.toStyledString() << ", without key:" << CCarve::ms_str_carve_id_key
			, str_err_reason_for_debug, "参数错误", str_err_reason_for_user, MSP_ERROR_INVALID_PARA);
		const string& str_carve_id = json_params[CCarve::ms_str_carve_id_key].asString();
		boost::shared_ptr<CCarve> ptr_carve;
		//根据设备编号查找对应的雕刻机
		{
			Thread_Read_Lock guard(m_rw_carveId_carvePtr);
			TYPE_MAP_ITER iter = m_map_carveId_carvePtr.find(str_carve_id);
			businlog_error_return_debug_and_user_reason(iter != m_map_carveId_carvePtr.end(), __CLASS_FUNCTION__ 
				<< " | Can not find carve id:" << str_carve_id << " in map", str_err_reason_for_debug
				, "设备编号对应的设备未连接", str_err_reason_for_user, MSP_ERROR_NOT_FOUND);
			ptr_carve = iter->second;
		}
		//走到这里，说明找到了对应的雕刻机
		//查询雕刻机状态
		int ret = ptr_carve->get_carve_status(eCarve_common_status, str_err_reason_for_debug, str_err_reason_for_user);
		businlog_error_return(!ret, ("%s | fail to get carve status, reason:%s."
			, __CLASS_FUNCTION__, str_err_reason_for_debug.c_str()), ret);
		return MSP_SUCCESS;
	}
	catch (std::exception& e)
	{
		str_err_reason_for_debug = string("Has exception:") + string(e.what());
		str_err_reason_for_user = "服务异常";
		businlog_error("%s | err reason:%s.", __CLASS_FUNCTION__, str_err_reason_for_debug.c_str());
		return MSP_ERROR_EXCEPTION;
	}
}

int CCarve_Manager::get_carve_info(const Json::Value& json_params, SCarve_Info& carve_info, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	try
	{
		//从参数中获取设备编号
		businlog_error_return_debug_and_user_reason(json_params.isMember(CCarve::ms_str_carve_id_key), __CLASS_FUNCTION__
			<< " | json:" << json_params.toStyledString() << ", without key:" << CCarve::ms_str_carve_id_key
			, str_err_reason_for_debug, "参数错误", str_err_reason_for_user, MSP_ERROR_INVALID_PARA);
		const string& str_carve_id = json_params[CCarve::ms_str_carve_id_key].asString();
		boost::shared_ptr<CCarve> ptr_carve;
		//根据设备编号查找对应的雕刻机
		{
			Thread_Read_Lock guard(m_rw_carveId_carvePtr);
			TYPE_MAP_ITER iter = m_map_carveId_carvePtr.find(str_carve_id);
			businlog_error_return_debug_and_user_reason(iter != m_map_carveId_carvePtr.end(), __CLASS_FUNCTION__ 
				<< " | Can not find carve id:" << str_carve_id << " in map", str_err_reason_for_debug
				, "设备编号对应的设备未连接", str_err_reason_for_user, MSP_ERROR_NOT_FOUND);
			ptr_carve = iter->second;
		}
		//走到这里，说明找到了对应的雕刻机
		//查询设备信息
		int ret = ptr_carve->get_info(carve_info, str_err_reason_for_debug, str_err_reason_for_user);
		businlog_error_return(!ret, ("%s | fail to get carve info, reason:%s."
			, __CLASS_FUNCTION__, str_err_reason_for_debug.c_str()), ret);
		return MSP_SUCCESS;
	}
	catch (std::exception& e)
	{
		str_err_reason_for_debug = string("Has exception:") + string(e.what());
		str_err_reason_for_user = "服务异常";
		businlog_error("%s | err reason:%s.", __CLASS_FUNCTION__, str_err_reason_for_debug.c_str());
		return MSP_ERROR_EXCEPTION;
	}
}

/************************************
* Method:    get_all_carves_info
* Brief:  获取所有雕刻机的信息
* Access:    public 
* Returns:   int 0:调用成功（可能有部分雕刻机出错）非0：调用失败
* Qualifier:
*Parameter: const Json::Value & json_params -[in]  
*Parameter: Json::Value & json_result -[out]  所有雕刻机信息结果数组对应的json
*Parameter: string & str_err_reason_for_debug -[out]  
*Parameter: string & str_err_reason_for_user -[out]  
************************************/
int CCarve_Manager::get_all_carves_info(const Json::Value& json_params, Json::Value& json_result, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	businlog_tracer_perf(CCarve_Manager::get_all_carves_info);
	//TODO::当设备很多时，此接口会很耗时，后面需要确认并优化
	try
	{
		//遍历容器，对获取所有雕刻机的信息
		{
			Thread_Read_Lock guard(m_rw_carveId_carvePtr);
			//判定容器是否为空
			businlog_error_return_debug_and_user_reason(false == m_map_carveId_carvePtr.empty()
				, __CLASS_FUNCTION__ << " | there is no carve connected, please connect carve first."
				, str_err_reason_for_debug, "没有设备被成功连接，请先连接设备", str_err_reason_for_user, MSP_ERROR_NOT_FOUND);
			for (TYPE_MAP_ITER iter = m_map_carveId_carvePtr.begin(); iter != m_map_carveId_carvePtr.end(); ++iter)
			{
				if (!iter->second)
				{//指针为空
					businlog_error("%s | there is null in map.", __CLASS_FUNCTION__);
					continue;
				}
				SCarve_Info single_carve_info;
				string str_single_err_for_debug, str_single_err_for_user;
				//查询设备信息
				int ret = iter->second->get_info(single_carve_info, str_single_err_for_debug, str_single_err_for_user);
				//无论结果如何，都报错并构造响应结果，而不返回
				if (ret)
				{
					businlog_error("%s | fail to get carve info, carve id:%s, reason:%s."
						, __CLASS_FUNCTION__, iter->second->get_id().c_str(), str_single_err_for_debug.c_str());
				}
				Json::Value json_single_resp;
				//构造结果，
				json_single_resp["ret"] = ret;
				json_single_resp["errmsg"] = str_single_err_for_debug;
				json_single_resp["errmsg_for_user"] = sp::toutf8(str_single_err_for_user);
				json_single_resp[CCarve::ms_str_carve_id_key] = iter->second->get_id();
				json_single_resp["taskNo"] = single_carve_info.str_task_no;
				json_single_resp["machine_ip"] = single_carve_info.str_machine_ip;
				json_single_resp["currentStatus"] = single_carve_info.eCarve_status;
#ifdef SERVER_WITH_CONTROL_LOGIC
				json_single_resp["worktime"] = single_carve_info.nTotal_engraving_time;
#endif
				json_single_resp["gNo"] = single_carve_info.str_gCode_no;
				json_single_resp["rowNo"] = single_carve_info.nCurrent_line_num;
				//将单个结果添加到结果数组中
				json_result["All_machines"].append(json_single_resp);
				//如果出错了，则将错误信息累加
				if (ret)
				{
					str_err_reason_for_debug += "." + str_single_err_for_debug;
					str_err_reason_for_user += "。" + str_single_err_for_user;
				}
			}
		}
		//整体调用成功，尽管可能存在单个失败
		return MSP_SUCCESS;
	}
	catch (std::exception& e)
	{
		str_err_reason_for_debug = string("Has exception:") + string(e.what());
		str_err_reason_for_user = "服务异常";
		businlog_error("%s | err reason:%s.", __CLASS_FUNCTION__, str_err_reason_for_debug.c_str());
		return MSP_ERROR_EXCEPTION;
	}
}

/************************************
* Method:    start_poll_carve_status
* Brief:  开启线程以轮询雕刻机状态
* Access:    public 
* Returns:   int
* Qualifier:
*Parameter: string & str_err_reason_for_debug -[in/out]  
*Parameter: string & str_err_reason_for_user -[in/out]  
************************************/
int CCarve_Manager::start_poll_carve_status(string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	try
	{
		//创建线程：轮询雕刻机状态
		m_bStop_poll_status = false;
		m_thread_poll_status = boost::thread(boost::bind(&CCarve_Manager::svc, this));
		return MSP_SUCCESS;
	}
	catch (std::exception& e)
	{
		str_err_reason_for_debug = string("Starting poll carve status has exception, reason:") + string(e.what());
		str_err_reason_for_user = "服务发生异常";
		businlog_error("%s | err reason:%s.", __CLASS_FUNCTION__, str_err_reason_for_debug.c_str());
		return MSP_ERROR_EXCEPTION;
	}
}

/************************************
* Method:    stop_poll_carve_status
* Brief:  停止轮询雕刻机状态的线程，并等待轮询线程退出
* Access:    public 
* Returns:   int
* Qualifier:
*Parameter: string & str_err_reason_for_debug -[in/out]  
*Parameter: string & str_err_reason_for_user -[in/out]  
************************************/
int CCarve_Manager::stop_poll_carve_status(string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	m_bStop_poll_status = true;
	if (m_thread_poll_status.joinable())
	{
		m_thread_poll_status.interrupt();
		m_thread_poll_status.join();
		return MSP_SUCCESS;
	}
	else
	{
		str_err_reason_for_debug =  "the thread of poll carve status  is not joinable";
		str_err_reason_for_user = "服务异常";
		businlog_error("%s | err reason:%s.", __CLASS_FUNCTION__, str_err_reason_for_debug.c_str());
		return MSP_ERROR_INVALID_OPERATION;
	}
}

int CCarve_Manager::start_engraving(const Json::Value& json_params, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	try
	{
		//从参数中获取设备编号
		businlog_error_return_debug_and_user_reason(json_params.isMember(CCarve::ms_str_carve_id_key), __CLASS_FUNCTION__
			<< " | json:" << json_params.toStyledString() << ", without key:" << CCarve::ms_str_carve_id_key
			, str_err_reason_for_debug, "参数错误", str_err_reason_for_user, MSP_ERROR_INVALID_PARA);
		const string& str_carve_id = json_params[CCarve::ms_str_carve_id_key].asString();
		boost::shared_ptr<CCarve> ptr_carve;
		//根据设备编号查找对应的雕刻机
		{
			Thread_Read_Lock guard(m_rw_carveId_carvePtr);
			TYPE_MAP_ITER iter = m_map_carveId_carvePtr.find(str_carve_id);
			businlog_error_return_debug_and_user_reason(iter != m_map_carveId_carvePtr.end(), __CLASS_FUNCTION__ 
				<< " | Can not find carve id:" << str_carve_id << " in map", str_err_reason_for_debug
				, "设备编号对应的设备未连接", str_err_reason_for_user, MSP_ERROR_NOT_FOUND);
			ptr_carve = iter->second;
		}
		//走到这里，说明找到了对应的雕刻机
		//开始雕刻
		int ret = ptr_carve->start(json_params, str_err_reason_for_debug, str_err_reason_for_user);
		businlog_error_return(!ret, ("%s | fail to start carve, reason:%s."
			, __CLASS_FUNCTION__, str_err_reason_for_debug.c_str()), ret);
#ifdef SERVER_WITH_CONTROL_LOGIC
		//开始雕刻成功后则设置开始雕刻时的时间
		ptr_carve->start_count_engraving_time();
#endif
		return MSP_SUCCESS;
	}
	catch (std::exception& e)
	{
		str_err_reason_for_debug = string("Has exception:") + string(e.what());
		str_err_reason_for_user = "服务异常";
		businlog_error("%s | err reason:%s.", __CLASS_FUNCTION__, str_err_reason_for_debug.c_str());
		return MSP_ERROR_EXCEPTION;
	}
}

int CCarve_Manager::upload_1_file(const Json::Value& json_params, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	try
	{
		//从参数中获取设备编号
		businlog_error_return_debug_and_user_reason(json_params.isMember(CCarve::ms_str_carve_id_key), __CLASS_FUNCTION__
			<< " | json:" << json_params.toStyledString() << ", without key:" << CCarve::ms_str_carve_id_key
			, str_err_reason_for_debug, "参数错误", str_err_reason_for_user, MSP_ERROR_INVALID_PARA);
		const string& str_carve_id = json_params[CCarve::ms_str_carve_id_key].asString();
		boost::shared_ptr<CCarve> ptr_carve;
		//根据设备编号查找对应的雕刻机
		{
			Thread_Read_Lock guard(m_rw_carveId_carvePtr);
			TYPE_MAP_ITER iter = m_map_carveId_carvePtr.find(str_carve_id);
			businlog_error_return_debug_and_user_reason(iter != m_map_carveId_carvePtr.end(), __CLASS_FUNCTION__ 
				<< " | Can not find carve id:" << str_carve_id << " in map", str_err_reason_for_debug
				, "设备编号对应的设备未连接", str_err_reason_for_user, MSP_ERROR_NOT_FOUND);
			ptr_carve = iter->second;
		}
		//走到这里，说明找到了对应的雕刻机
		//下在G代码文件
		int ret = ptr_carve->upload_1_file(json_params, str_err_reason_for_debug, str_err_reason_for_user);
		businlog_error_return(!ret, ("%s | fail to start carve, reason:%s."
			, __CLASS_FUNCTION__, str_err_reason_for_debug.c_str()), ret);
		return MSP_SUCCESS;
	}
	catch (std::exception& e)
	{
		str_err_reason_for_debug = string("Has exception:") + string(e.what());
		str_err_reason_for_user = "服务异常";
		businlog_error("%s | err reason:%s.", __CLASS_FUNCTION__, str_err_reason_for_debug.c_str());
		return MSP_ERROR_EXCEPTION;
	}
}

int CCarve_Manager::delete_1_file(const Json::Value& json_params, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	try
	{
		//从参数中获取设备编号
		businlog_error_return_debug_and_user_reason(json_params.isMember(CCarve::ms_str_carve_id_key), __CLASS_FUNCTION__
			<< " | json:" << json_params.toStyledString() << ", without key:" << CCarve::ms_str_carve_id_key
			, str_err_reason_for_debug, "参数错误", str_err_reason_for_user, MSP_ERROR_INVALID_PARA);
		const string& str_carve_id = json_params[CCarve::ms_str_carve_id_key].asString();
		boost::shared_ptr<CCarve> ptr_carve;
		//根据设备编号查找对应的雕刻机
		{
			Thread_Read_Lock guard(m_rw_carveId_carvePtr);
			TYPE_MAP_ITER iter = m_map_carveId_carvePtr.find(str_carve_id);
			businlog_error_return_debug_and_user_reason(iter != m_map_carveId_carvePtr.end(), __CLASS_FUNCTION__ 
				<< " | Can not find carve id:" << str_carve_id << " in map", str_err_reason_for_debug
				, "设备编号对应的设备未连接", str_err_reason_for_user, MSP_ERROR_NOT_FOUND);
			ptr_carve = iter->second;
		}
		//走到这里，说明找到了对应的雕刻机
		//删除G代码文件
		int ret = ptr_carve->delete_1_file(json_params, str_err_reason_for_debug, str_err_reason_for_user);
		businlog_error_return(!ret, ("%s | fail to start carve, reason:%s."
			, __CLASS_FUNCTION__, str_err_reason_for_debug.c_str()), ret);
		return MSP_SUCCESS;
	}
	catch (std::exception& e)
	{
		str_err_reason_for_debug = string("Has exception:") + string(e.what());
		str_err_reason_for_user = "服务异常";
		businlog_error("%s | err reason:%s.", __CLASS_FUNCTION__, str_err_reason_for_debug.c_str());
		return MSP_ERROR_EXCEPTION;
	}
}

int CCarve_Manager::emergency_stop_one(const Json::Value& json_params, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	//雕刻机急停
	try
	{
		//从参数中获取设备编号
		businlog_error_return_debug_and_user_reason(json_params.isMember(CCarve::ms_str_carve_id_key), __CLASS_FUNCTION__
			<< " | json:" << json_params.toStyledString() << ", without key:" << CCarve::ms_str_carve_id_key
			, str_err_reason_for_debug, "参数错误", str_err_reason_for_user, MSP_ERROR_INVALID_PARA);
		const string& str_carve_id = json_params[CCarve::ms_str_carve_id_key].asString();
		boost::shared_ptr<CCarve> ptr_carve;
		//根据设备编号查找对应的雕刻机
		{
			Thread_Read_Lock guard(m_rw_carveId_carvePtr);
			TYPE_MAP_ITER iter = m_map_carveId_carvePtr.find(str_carve_id);
			businlog_error_return_debug_and_user_reason(iter != m_map_carveId_carvePtr.end(), __CLASS_FUNCTION__ 
				<< " | Can not find carve id:" << str_carve_id << " in map", str_err_reason_for_debug
				, "设备编号对应的设备未连接", str_err_reason_for_user, MSP_ERROR_NOT_FOUND);
			ptr_carve = iter->second;
		}
		//走到这里，说明找到了对应的雕刻机
		int ret = ptr_carve-> stop_fast(json_params, str_err_reason_for_debug, str_err_reason_for_user);
		businlog_error_return(!ret, ("%s | fail to fast stop carve, reason:%s."
			, __CLASS_FUNCTION__, str_err_reason_for_debug.c_str()), ret);
		return MSP_SUCCESS;
	}
	catch (std::exception& e)
	{
		str_err_reason_for_debug = string("Has exception:") + string(e.what());
		str_err_reason_for_user = "服务异常";
		businlog_error("%s | err reason:%s.", __CLASS_FUNCTION__, str_err_reason_for_debug.c_str());
		return MSP_ERROR_EXCEPTION;
	}
}
int CCarve_Manager::emergency_stop_all(const Json::Value& json_params,Json::Value& json_result,string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	//全部雕刻机急停
	try
	{
		{
			//查询所有的已连接设备编号
			Thread_Read_Lock guard(m_rw_carveId_carvePtr);
			//判定容器是否为空
			businlog_error_return_debug_and_user_reason(false == m_map_carveId_carvePtr.empty()
				, __CLASS_FUNCTION__ << " | there is no carve connected, please connect carve first."
				, str_err_reason_for_debug, "没有设备被成功连接，请先连接设备", str_err_reason_for_user, MSP_ERROR_NOT_FOUND);
			for (TYPE_MAP_ITER iter = m_map_carveId_carvePtr.begin();iter!=m_map_carveId_carvePtr.end(); ++iter)
			{	
				//判定对象是否合法
				if (!iter->second)
				{
					businlog_error("%s | ptr carve is NULL, carve id:%s" , __CLASS_FUNCTION__, iter->first.c_str());
					continue;
				}
				string str_single_err_for_debug; 
				string str_single_err_for_user;
				int ret= iter->second->stop_fast(json_params, str_single_err_for_debug, str_single_err_for_user);
				if (ret!=0)
				{
					businlog_error("%s | fail to emergency stop, carve id:%s, reason:%s."
						, __CLASS_FUNCTION__, iter->second->get_id().c_str(), str_single_err_for_debug.c_str());
				}
				Json::Value json_single_resp;
				//构造结果
				json_single_resp["ret"] = ret;
				json_single_resp["errmsg"] = str_single_err_for_debug;
				json_single_resp["errmsg_for_user"] = sp::toutf8(str_single_err_for_user);
				json_single_resp[CCarve::ms_str_carve_id_key] = iter->second->get_id();

				//将单个结果添加到结果数组中
				json_result["results"].append(json_single_resp);
				//如果出错了，则将错误信息累加
				if (ret)
				{
					str_err_reason_for_debug += "." + str_single_err_for_debug;
					str_err_reason_for_user += "。" + str_single_err_for_user;
				}
			}
		}
		
		//函数调用完成,返回成功，单个失败由上层json "results" 节点判断
		return MSP_SUCCESS;
	}
	catch (std::exception& e)
	{
		str_err_reason_for_debug = string("Has exception:") + string(e.what());
		str_err_reason_for_user = "服务异常";
		businlog_error("%s | err reason:%s.", __CLASS_FUNCTION__, str_err_reason_for_debug.c_str());
		return MSP_ERROR_EXCEPTION;
	}
}
int CCarve_Manager::cancel_emergency_stop_one(const Json::Value& json_params, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	//雕刻机急停
	try
	{
		//从参数中获取设备编号
		businlog_error_return_debug_and_user_reason(json_params.isMember(CCarve::ms_str_carve_id_key), __CLASS_FUNCTION__
			<< " | json:" << json_params.toStyledString() << ", without key:" << CCarve::ms_str_carve_id_key
			, str_err_reason_for_debug, "参数错误", str_err_reason_for_user, MSP_ERROR_INVALID_PARA);
		const string& str_carve_id = json_params[CCarve::ms_str_carve_id_key].asString();
		boost::shared_ptr<CCarve> ptr_carve;
		//根据设备编号查找对应的雕刻机
		{
			Thread_Read_Lock guard(m_rw_carveId_carvePtr);
			TYPE_MAP_ITER iter = m_map_carveId_carvePtr.find(str_carve_id);
			businlog_error_return_debug_and_user_reason(iter != m_map_carveId_carvePtr.end(), __CLASS_FUNCTION__ 
				<< " | Can not find carve id:" << str_carve_id << " in map", str_err_reason_for_debug
				, "设备编号对应的设备未连接", str_err_reason_for_user, MSP_ERROR_NOT_FOUND);
			ptr_carve = iter->second;
		}
		//走到这里，说明找到了对应的雕刻机
		int ret = ptr_carve-> cancel_fast_stop(json_params, str_err_reason_for_debug, str_err_reason_for_user);
		businlog_error_return(!ret, ("%s | fail to fast stop carve, reason:%s."
			, __CLASS_FUNCTION__, str_err_reason_for_debug.c_str()), ret);
		return MSP_SUCCESS;
	}
	catch (std::exception& e)
	{
		str_err_reason_for_debug = string("Has exception:") + string(e.what());
		str_err_reason_for_user = "服务异常";
		businlog_error("%s | err reason:%s.", __CLASS_FUNCTION__, str_err_reason_for_debug.c_str());
		return MSP_ERROR_EXCEPTION;
	}
}
int CCarve_Manager::cancel_emergency_stop_all(const Json::Value& json_params,Json::Value& json_result,string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	//全部雕刻机急停
	try
	{
		{
			//查询所有的已连接设备编号
			Thread_Read_Lock guard(m_rw_carveId_carvePtr);
			//判定容器是否为空
			businlog_error_return_debug_and_user_reason(false == m_map_carveId_carvePtr.empty()
				, __CLASS_FUNCTION__ << " | there is no carve connected, please connect carve first."
				, str_err_reason_for_debug, "没有设备被成功连接，请先连接设备", str_err_reason_for_user, MSP_ERROR_NOT_FOUND);
			for (TYPE_MAP_ITER iter = m_map_carveId_carvePtr.begin();iter!=m_map_carveId_carvePtr.end(); ++iter)
			{	
				//判定对象是否合法
				if (!iter->second)
				{
					businlog_error("%s | ptr carve is NULL, carve id:%s" , __CLASS_FUNCTION__, iter->first.c_str());
					continue;
				}
				string str_single_err_for_debug; 
				string str_single_err_for_user;
				int ret= iter->second->cancel_fast_stop(json_params, str_single_err_for_debug, str_single_err_for_user);
				if (ret!=0)
				{
					businlog_error("%s | fail to emergency stop, carve id:%s, reason:%s."
						, __CLASS_FUNCTION__, iter->second->get_id().c_str(), str_single_err_for_debug.c_str());
				}
				Json::Value json_single_resp;
				//构造结果
				json_single_resp["ret"] = ret;
				json_single_resp["errmsg"] = str_single_err_for_debug;
				json_single_resp["errmsg_for_user"] = sp::toutf8(str_single_err_for_user);
				json_single_resp[CCarve::ms_str_carve_id_key] = iter->second->get_id();

				//将单个结果添加到结果数组中
				json_result["results"].append(json_single_resp);
				//如果出错了，则将错误信息累加
				if (ret)
				{
					str_err_reason_for_debug += "." + str_single_err_for_debug;
					str_err_reason_for_user += "。" + str_single_err_for_user;
				}
			}
		}

		//函数调用完成,返回成功，单个失败由上层json "results" 节点判断
		return MSP_SUCCESS;
	}
	catch (std::exception& e)
	{
		str_err_reason_for_debug = string("Has exception:") + string(e.what());
		str_err_reason_for_user = "服务异常";
		businlog_error("%s | err reason:%s.", __CLASS_FUNCTION__, str_err_reason_for_debug.c_str());
		return MSP_ERROR_EXCEPTION;
	}
}

int CCarve_Manager::adjust_speed(const Json::Value& json_params, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	//调整雕刻机运行速度
	try
	{
		//从参数中获取设备编号
		businlog_error_return_debug_and_user_reason(json_params.isMember(CCarve::ms_str_carve_id_key), __CLASS_FUNCTION__
			<< " | json:" << json_params.toStyledString() << ", without key:" << CCarve::ms_str_carve_id_key
			, str_err_reason_for_debug, "参数错误", str_err_reason_for_user, MSP_ERROR_INVALID_PARA);
		const string& str_carve_id = json_params[CCarve::ms_str_carve_id_key].asString();
		boost::shared_ptr<CCarve> ptr_carve;
		//根据设备编号查找对应的雕刻机
		{
			Thread_Read_Lock guard(m_rw_carveId_carvePtr);
			TYPE_MAP_ITER iter = m_map_carveId_carvePtr.find(str_carve_id);
			businlog_error_return_debug_and_user_reason(iter != m_map_carveId_carvePtr.end(), __CLASS_FUNCTION__ 
				<< " | Can not find carve id:" << str_carve_id << " in map", str_err_reason_for_debug
				, "设备编号对应的设备未连接", str_err_reason_for_user, MSP_ERROR_NOT_FOUND);
			ptr_carve = iter->second;
		}
		//走到这里，说明找到了对应的雕刻机
		int ret = ptr_carve-> adjust_speed(json_params, str_err_reason_for_debug, str_err_reason_for_user);
		businlog_error_return(!ret, ("%s | fail to fast stop carve, reason:%s."
			, __CLASS_FUNCTION__, str_err_reason_for_debug.c_str()), ret);
		return MSP_SUCCESS;
	}
	catch (std::exception& e)
	{
		str_err_reason_for_debug = string("Has exception:") + string(e.what());
		str_err_reason_for_user = "服务异常";
		businlog_error("%s | err reason:%s.", __CLASS_FUNCTION__, str_err_reason_for_debug.c_str());
		return MSP_ERROR_EXCEPTION;
	}
}


CCarve_Manager::CCarve_Manager()
	: m_bStop_poll_status(true)
{

}

CCarve_Manager::~CCarve_Manager()
{
	businlog_tracer_perf(CCarve_Manager);
	businlog_warn("%s | Notice", __CLASS_FUNCTION__);
	string str_err_for_debug, str_err_for_user;
	//先停止线程后释放函数库
	if (false == m_bStop_poll_status)
	{//线程仍然开始着
		int ret = stop_poll_carve_status(str_err_for_debug, str_err_for_user);
		if (ret)
		{
			businlog_error("%s | fail to stop polling carve status, reason:%s"
				, __CLASS_FUNCTION__, str_err_for_debug.c_str());
		}
	}
}

void CCarve_Manager::svc()
{
	businlog_crit("%s | start thread to poll status of all carves successfully", __CLASS_FUNCTION__);
	size_t nWait_time_minute = 1; //每次休眠时间,TODO::目前设置小点，实际中设置大点
	businlog_warn("%s | please read the param from configure file", __CLASS_FUNCTION__);
	try
	{
		while (false == m_bStop_poll_status)
		{
			boost::this_thread::sleep(boost::posix_time::minutes(nWait_time_minute));
			int ret = 0;
			//遍历雕刻机列表
			{ 
				Thread_Read_Lock guard(m_rw_carveId_carvePtr);
				for (TYPE_MAP_ITER iter = m_map_carveId_carvePtr.begin()
					; iter != m_map_carveId_carvePtr.end(); ++ iter)
				{
					//判定对象是否合法
					if (!iter->second)
					{
						businlog_error("%s | ptr carve is NULL, carve id:%s" , __CLASS_FUNCTION__, iter->first.c_str());
						continue;
					}
					//获取设备状态
					ECARVE_STATUS_TYPE eCarve_common_status = CARVE_STATUS_MIN;
					string str_err_reason_for_debug, str_err_reason_for_user;
					ret = iter->second->get_carve_status(eCarve_common_status, str_err_reason_for_debug
						, str_err_reason_for_user);
					if (ret)
					{//获取状态出错
						businlog_error("%s | fail to get carve status, carve id:%s, reason:%s"
							, __CLASS_FUNCTION__, iter->first.c_str(), str_err_reason_for_debug.c_str());
						continue;
					}
					//此时成功获取设备状态
					if (CARVE_STATUS_COMPLETED == eCarve_common_status)
					{
						//如果为雕刻完成，则暂停累加雕刻时间
						ret = iter->second->pause_count_engraving_time(str_err_reason_for_debug, str_err_reason_for_user);
						if (ret)
						{
							businlog_error("%s | fail to pause count engraving time, carve id:%s, reason:%s."
								, __CLASS_FUNCTION__, iter->first.c_str(), str_err_reason_for_debug.c_str());
						}
					}
				}//end for 遍历map
			}
		}//end whiel 线程
	}
	catch (boost::thread_interrupted& ) //被中断时，会抛出此异常
	{
	}

	businlog_warn("%s | ++++++++++++++++ finish to poll status of all carves+++++++++++++", __CLASS_FUNCTION__);
}

CCarve_Manager::CGarbo CCarve_Manager::Garbo;

CCarve_Manager* CCarve_Manager::ms_pInstance = new CCarve_Manager();

CCarve_Manager::CGarbo::~CGarbo()
{
	if (NULL != CCarve_Manager::ms_pInstance)
	{
		delete CCarve_Manager::ms_pInstance;
		CCarve_Manager::ms_pInstance = NULL;
	}
}
