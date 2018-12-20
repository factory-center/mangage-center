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
	bool bSuccess = CBaoyuan_Lib::instance()->create_connection(m_nConn_idx, m_str_ip, str_kernel_err_reason);
	businlog_error_return(bSuccess, ("%s | fail to connect carve, ip:%s, conn idx:%d, reason:%s"
		, __CLASS_FUNCTION__, m_str_ip.c_str(), m_nConn_idx, str_kernel_err_reason.c_str()), MSP_ERROR_FAIL);
	m_bConnected = true;
	return MSP_SUCCESS;
}

int CCarve::disconnect(string& str_kernel_err_reason)
{
	boost::mutex::scoped_lock guard(m_mutex_for_cmd);
	//判定当前状态
	businlog_error_return_err_reason(true == m_bConnected, __CLASS_FUNCTION__ << " | carve ip:" << m_str_ip 
		<<" is not connected", str_kernel_err_reason, MSP_ERROR_FAIL);
	bool bSuccess = CBaoyuan_Lib::instance()->disconnect(m_nConn_idx, str_kernel_err_reason);
	businlog_error_return(bSuccess, ("%s | fail to disconnect carve, ip:%s, conn idx:%d, reason:%s"
		, __CLASS_FUNCTION__, m_str_ip.c_str(), m_nConn_idx, str_kernel_err_reason.c_str()), MSP_ERROR_FAIL);
	m_bConnected =false;
	return MSP_SUCCESS;
}

int CCarve::set_continue_status(unsigned char nStatus, unsigned short nMax_wait_time, string& str_kernel_err_reason)
{
	boost::mutex::scoped_lock guard(m_mutex_for_cmd);
	businlog_error_return_err_reason(true == m_bConnected, __CLASS_FUNCTION__ << " | carve ip:" << m_str_ip 
		<<" is not connected", str_kernel_err_reason, MSP_ERROR_INVALID_OPERATION);

	bool bSuccess = CBaoyuan_Lib::instance()->set_continue_status(m_nConn_idx, nStatus, nMax_wait_time, str_kernel_err_reason);
	businlog_error_return(bSuccess, ("%s | fail to set continue status, ip:%s, reason:%s"
		, __CLASS_FUNCTION__, m_str_ip.c_str(), str_kernel_err_reason.c_str()), MSP_ERROR_FAIL);
	return MSP_SUCCESS;
}

int CCarve::reset_carve(unsigned short nMax_wait_time, string& str_kernel_err_reason)
{
	boost::mutex::scoped_lock guard(m_mutex_for_cmd);
	businlog_error_return_err_reason(true == m_bConnected, __CLASS_FUNCTION__ << " | carve ip:" << m_str_ip 
		<<" is not connected", str_kernel_err_reason, MSP_ERROR_INVALID_OPERATION);

	bool bSuccess = CBaoyuan_Lib::instance()->reset_carve(m_nConn_idx, nMax_wait_time, str_kernel_err_reason);
	businlog_error_return(bSuccess, ("%s | fail to reset carve, ip:%s", __CLASS_FUNCTION__, m_str_ip.c_str()), MSP_ERROR_FAIL);
	return MSP_SUCCESS;
}

CCarve::CCarve(unsigned short nConn_idx, const string& str_ip) 
	: CDevice(ECARVE, str_ip)
	, m_nConn_idx(nConn_idx)
	, m_bConnected(false)
{

}
