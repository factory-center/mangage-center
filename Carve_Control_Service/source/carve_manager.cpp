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

int CCarve_Manager::add_carve(const Json::Value& json_params, const boost::shared_ptr<CCarve>& ptr_carve, string& str_err_reason)
{
	try
	{
		businlog_error_return_err_reason(ptr_carve, __CLASS_FUNCTION__ << " | ptr of carve is NULL."
			, str_err_reason, MSP_ERROR_INVALID_HANDLE);
		const string& str_carve_id = ptr_carve->get_id();
		//将数据插入map中
		Thread_Write_Lock guard(m_rw_carveId_carvePtr);
		bool bSuccess = m_map_carveId_carvePtr.insert(std::make_pair(str_carve_id, ptr_carve)).second;
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
