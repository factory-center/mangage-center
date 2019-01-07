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

int CCarve_Manager::connect_carve(const Json::Value& json_params, string& str_err_reason)
{
	try
	{
		//构造雕刻机对象
		boost::shared_ptr<CCarve> ptr_carve = boost::make_shared<CCarve>(json_params);
		//连接雕刻机
		int ret = ptr_carve->connect(str_err_reason);
		businlog_error_return(0 == ret, ("%s | fail to connect carve, reason:%s.", __CLASS_FUNCTION__, str_err_reason.c_str()), ret);

		//设置continue状态
		unsigned short nMax_wait_time = 1000;
		string str_key = "max_wait_time";
		if (json_params.isMember(str_key))
		{//参数中含有最大等待时间
			nMax_wait_time = json_params[str_key].asInt();
		}
		ret = ptr_carve->set_continue_status(0, nMax_wait_time, str_err_reason);
		businlog_error_return(!ret, ("%s | fail to set carve continue status, reason:%s.", __CLASS_FUNCTION__, str_err_reason.c_str()), ret);

		//重置设备
		ret = ptr_carve->reset(nMax_wait_time, str_err_reason);
		businlog_error_return(!ret, ("%s | fail to reset carve, reason:%s.", __CLASS_FUNCTION__, str_err_reason.c_str()), ret);
	
		//所有关于雕刻机的操作都成功后，才将其插入map中
		const string& str_carve_id = ptr_carve->get_id();
		bool bSuccess = false;
		//将数据插入map中
		{
			Thread_Write_Lock guard(m_rw_carveId_carvePtr);
			bSuccess = m_map_carveId_carvePtr.insert(std::make_pair(str_carve_id, ptr_carve)).second;
		}
		//判定插入结果，如果失败，则说明键已经存在了.（map插入失败，是不会覆盖原来的value）
		businlog_error_return_err_reason(bSuccess, __CLASS_FUNCTION__ << " | fail to insert, carve id:" << str_carve_id << " alread exist."
			, str_err_reason, MSP_ERROR_INVALID_PARA);
		return MSP_SUCCESS;
	}
	catch (std::exception& e)
	{
		str_err_reason = string("Has exception:") + string(e.what());
		businlog_error("%s | err reason:%s.", __CLASS_FUNCTION__, str_err_reason.c_str());
		return MSP_ERROR_EXCEPTION;
	}
}

CCarve_Manager::CCarve_Manager()
{

}

CCarve_Manager::~CCarve_Manager()
{
	businlog_tracer_perf(CCarve_Manager);
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
