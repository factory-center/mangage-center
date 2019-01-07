/******************************************************************
* Copyright (c) 2016-2017,HIIRI Inc.
* All rights reserved.
* @file: carve_manager.cpp
* @brief: ���˵���ļ����ܡ���; (Comment)��
* @author:	minglu2
* @version: 1.0
* @date: 2019/01/07
* 
* @see
* 
* <b>�汾��¼��</b><br>
* <table>
*  <tr> <th>�汾	<th>����		<th>����	<th>��ע </tr>
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
		//�����̻�����
		boost::shared_ptr<CCarve> ptr_carve = boost::make_shared<CCarve>(json_params);
		//���ӵ�̻�
		int ret = ptr_carve->connect(str_err_reason);
		businlog_error_return(0 == ret, ("%s | fail to connect carve, reason:%s.", __CLASS_FUNCTION__, str_err_reason.c_str()), ret);

		//����continue״̬
		unsigned short nMax_wait_time = 1000;
		string str_key = "max_wait_time";
		if (json_params.isMember(str_key))
		{//�����к������ȴ�ʱ��
			nMax_wait_time = json_params[str_key].asInt();
		}
		ret = ptr_carve->set_continue_status(0, nMax_wait_time, str_err_reason);
		businlog_error_return(!ret, ("%s | fail to set carve continue status, reason:%s.", __CLASS_FUNCTION__, str_err_reason.c_str()), ret);

		//�����豸
		ret = ptr_carve->reset(nMax_wait_time, str_err_reason);
		businlog_error_return(!ret, ("%s | fail to reset carve, reason:%s.", __CLASS_FUNCTION__, str_err_reason.c_str()), ret);
	
		//���й��ڵ�̻��Ĳ������ɹ��󣬲Ž������map��
		const string& str_carve_id = ptr_carve->get_id();
		bool bSuccess = false;
		//�����ݲ���map��
		{
			Thread_Write_Lock guard(m_rw_carveId_carvePtr);
			bSuccess = m_map_carveId_carvePtr.insert(std::make_pair(str_carve_id, ptr_carve)).second;
		}
		//�ж������������ʧ�ܣ���˵�����Ѿ�������.��map����ʧ�ܣ��ǲ��Ḳ��ԭ����value��
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
