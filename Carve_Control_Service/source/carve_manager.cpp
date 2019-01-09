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

int CCarve_Manager::connect_carve(const Json::Value& json_params, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	try
	{
		//�Ӳ����л�ȡ�豸���
		businlog_error_return_debug_and_user_reason(json_params.isMember(CCarve::ms_str_carve_id_key), __CLASS_FUNCTION__
			<< " | json:" << json_params.toStyledString() << ", without key:" << CCarve::ms_str_carve_id_key
			, str_err_reason_for_debug, "��������", str_err_reason_for_user, MSP_ERROR_INVALID_PARA);
		const string& str_carve_id = json_params[CCarve::ms_str_carve_id_key].asString();
		
		boost::shared_ptr<CCarve> ptr_carve;
		std::pair<TYPE_MAP_ITER, bool> pair_insert_result;
		pair_insert_result.second = false;
		unsigned short nMax_wait_time = 1000;
		string str_key = "max_wait_time";
		int ret = 0;
		//�ж��˱�Ŷ�Ӧ�ĵ�̻��Ƿ��Ѿ�����
		{
			Thread_Write_Lock guard(m_rw_carveId_carvePtr);
			businlog_error_return_debug_and_user_reason(m_map_carveId_carvePtr.find(str_carve_id) == m_map_carveId_carvePtr.end()
				, __CLASS_FUNCTION__ << " | carve id:" << str_carve_id << " already exist.", str_err_reason_for_debug
				, "�豸��Ŷ�Ӧ���豸�Ѿ�����", str_err_reason_for_user, MSP_ERROR_ALREADY_EXIST);
			//map�в����ڶ�Ӧ���豸���
			//�����̻�����
			ptr_carve = boost::make_shared<CCarve>(json_params);
			//������Դ�����ʧ����ֱ�ӷ���
			ret = ptr_carve->acquire_resource(str_err_reason_for_debug, str_err_reason_for_user);
			businlog_error_return(!ret, ("%s | fail to acquire resource, reason:%s."
				, __CLASS_FUNCTION__, str_err_reason_for_debug.c_str()), ret);
			pair_insert_result = m_map_carveId_carvePtr.insert(std::make_pair(str_carve_id, ptr_carve));
			//�ж������������ʧ�ܣ���˵�����Ѿ�������.��map����ʧ�ܣ��ǲ��Ḳ��ԭ����value��
			businlog_error_return_debug_and_user_reason(pair_insert_result.second, __CLASS_FUNCTION__ << " | fail to insert, carve id:" << str_carve_id << " alread exist."
				, str_err_reason_for_debug, "�ظ������豸", str_err_reason_for_user, MSP_ERROR_INVALID_PARA);
		}
		//���й��ڵ�̻��Ĳ����������һ�����������map��ɾ������������
		//���ӵ�̻�
		ret = ptr_carve->connect(str_err_reason_for_debug, str_err_reason_for_user);
		if (ret)
		{
			businlog_error("%s | fail to connect carve, reason:%s.", __CLASS_FUNCTION__, str_err_reason_for_debug.c_str());
			goto Err_exit;
		}

		//����continue״̬
		if (json_params.isMember(str_key))
		{//�����к������ȴ�ʱ��
			nMax_wait_time = json_params[str_key].asInt();
		}
		ret = ptr_carve->set_continue_status(0, nMax_wait_time, str_err_reason_for_debug, str_err_reason_for_user);
		if (ret)
		{
			businlog_error("%s | fail to set carve continue status, reason:%s.", __CLASS_FUNCTION__, str_err_reason_for_debug.c_str());
			goto Err_exit;
		}

		//�����豸
		ret = ptr_carve->reset(nMax_wait_time, str_err_reason_for_debug, str_err_reason_for_user);
		if (ret)
		{
			businlog_error("%s | fail to reset carve, reason:%s.", __CLASS_FUNCTION__, str_err_reason_for_debug.c_str());
			goto Err_exit;
		}
		return MSP_SUCCESS;
Err_exit: //�����Ե�̻����������ˣ��򽫵�̻��Ƴ�
		//���֮ǰ��ӳɹ��ˣ������Ƴ�
		if(pair_insert_result.second)
		{
			//�Ƴ�����Դ�����ӵȽ������������������ã�
			Thread_Write_Lock guard(m_rw_carveId_carvePtr);
			m_map_carveId_carvePtr.erase(pair_insert_result.first);
		}
		return ret;
	}
	catch (std::exception& e)
	{
		str_err_reason_for_debug = string("Has exception:") + string(e.what());
		str_err_reason_for_user = "����˷����쳣";
		businlog_error("%s | err reason:%s.", __CLASS_FUNCTION__, str_err_reason_for_debug.c_str());
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
