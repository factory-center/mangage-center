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
		unsigned short nMax_wait_time = 1000;
		string str_key = "max_wait_time";
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
		if (json_params.isMember(str_key))
		{//参数中含有最大等待时间
			nMax_wait_time = json_params[str_key].asInt();
		}
		ret = ptr_carve->set_continue_status(0, nMax_wait_time, str_err_reason_for_debug, str_err_reason_for_user);
		if (ret)
		{
			businlog_error("%s | fail to set carve continue status, reason:%s.", __CLASS_FUNCTION__, str_err_reason_for_debug.c_str());
			goto Err_exit;
		}

		//重置设备
		ret = ptr_carve->reset(nMax_wait_time, str_err_reason_for_debug, str_err_reason_for_user);
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
		unsigned short nMax_wait_time = 1000;
		string str_key = "max_wait_time";
		if (json_params.isMember(str_key))
		{//参数中含有最大等待时间
			nMax_wait_time = json_params[str_key].asInt();
		}
		//开始雕刻
		int ret = ptr_carve->start(nMax_wait_time, str_err_reason_for_debug, str_err_reason_for_user);
		businlog_error_return(!ret, ("%s | fail to start carve, reason:%s."
			, __CLASS_FUNCTION__, str_err_reason_for_debug.c_str()), ret);
		//开始雕刻成功后则设置开始雕刻时的时间
		ptr_carve->start_count_engraving_time();
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
		//查询雕刻机状态
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
		//TODO: 最大等待时间临时暂定30秒
		int ret = ptr_carve-> stop_fast(30*1000, str_err_reason_for_debug, str_err_reason_for_user);
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
int CCarve_Manager::emergency_stop_all(const Json::Value& json_params, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	//全部雕刻机急停
	try
	{
		int ret = 0;
		//查询所有的已连接设备编号
		boost::shared_ptr<CCarve> ptr_carve;
	
		std::map<string, boost::shared_ptr<CCarve>>::iterator iter; //键为设备编号，值为CCarve的智能指针
		for (iter = m_map_carveId_carvePtr.begin();iter!=m_map_carveId_carvePtr.end();iter++)
		{
			ptr_carve = iter->second;
			Thread_Read_Lock guard(m_rw_carveId_carvePtr);
			//TODO: 最大等待时间临时暂定30秒
			ret= ptr_carve-> stop_fast(30*1000, str_err_reason_for_debug, str_err_reason_for_user);
			businlog_error_return(!ret, ("%s | fail to fast stop carve, reason:%s."
				, __CLASS_FUNCTION__, str_err_reason_for_debug.c_str()), ret);
		}
			
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
						businlog_error("%s | ptr carve is NULL, carve id:%s"
							, __CLASS_FUNCTION__, __CLASS_FUNCTION__, iter->first.c_str());
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
						//TODO::重置设备装备，因为雕刻完成后，就一直是雕刻完成状态
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
