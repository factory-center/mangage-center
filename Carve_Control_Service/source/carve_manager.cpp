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
		//���ҵ���ʱ�Ĳ���
		Json::Value json_local_params = json_params;

		int ret = 0;

		{
			Thread_Write_Lock guard(m_rw_carveId_carvePtr);
			//�ж��˱�Ŷ�Ӧ�ĵ�̻��Ƿ��Ѿ�����
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
		json_local_params[CCarve::ms_str_status_key] = 0;
		ret = ptr_carve->set_continue_status(json_local_params, str_err_reason_for_debug, str_err_reason_for_user);
		if (ret)
		{
			businlog_error("%s | fail to set carve continue status, reason:%s.", __CLASS_FUNCTION__, str_err_reason_for_debug.c_str());
			goto Err_exit;
		}

		//�����豸
		ret = ptr_carve->reset(json_local_params, str_err_reason_for_debug, str_err_reason_for_user);
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

int CCarve_Manager::disconnect_carve(const Json::Value& json_params, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	try
	{
		//�Ӳ����л�ȡ�豸���
		businlog_error_return_debug_and_user_reason(json_params.isMember(CCarve::ms_str_carve_id_key), __CLASS_FUNCTION__
			<< " | json:" << json_params.toStyledString() << ", without key:" << CCarve::ms_str_carve_id_key
			, str_err_reason_for_debug, "��������", str_err_reason_for_user, MSP_ERROR_INVALID_PARA);
		
		const string& str_carve_id = json_params[CCarve::ms_str_carve_id_key].asString();

		int ret = 0;

		{
			Thread_Write_Lock guard(m_rw_carveId_carvePtr);
			TYPE_MAP_ITER iter = m_map_carveId_carvePtr.find(str_carve_id);

			//�ж��˱�Ŷ�Ӧ�ĵ�̻��Ƿ��Ѿ�����,��������ڣ�ֱ�ӷ���
			businlog_error_return_debug_and_user_reason(iter!= m_map_carveId_carvePtr.end()
				, __CLASS_FUNCTION__ << " | carve id:" << str_carve_id << " not exist.", str_err_reason_for_debug
				, "�豸δ����", str_err_reason_for_user, MSP_ERROR_NOT_FOUND);

			//�Ͽ��豸
			ret = iter->second->disconnect(str_err_reason_for_debug, str_err_reason_for_user);
			businlog_error_return(!ret, ("%s | fail to disconnect, reason:%s."
				, __CLASS_FUNCTION__, str_err_reason_for_debug.c_str()), ret);

			//�ͷ���Դ
			ret = iter->second->release_resource(str_err_reason_for_debug,str_err_reason_for_user);
			businlog_error_return(!ret, ("%s | fail to release resource, reason:%s."
				, __CLASS_FUNCTION__, str_err_reason_for_debug.c_str()), ret);

			//��map���Ƴ���Ӧ�ĶϿ��豸
			m_map_carveId_carvePtr.erase(iter);
		}
		
		return MSP_SUCCESS;
	}
	catch (std::exception& e)
	{
		str_err_reason_for_debug = string("Has exception:") + string(e.what());
		str_err_reason_for_user = "����˷����쳣";
		businlog_error("%s | err reason:%s.", __CLASS_FUNCTION__, str_err_reason_for_debug.c_str());
		return MSP_ERROR_EXCEPTION;
	}
}

int CCarve_Manager::get_carve_status(const Json::Value& json_params, ECARVE_STATUS_TYPE& eCarve_common_status, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	try
	{
		//�Ӳ����л�ȡ�豸���
		businlog_error_return_debug_and_user_reason(json_params.isMember(CCarve::ms_str_carve_id_key), __CLASS_FUNCTION__
			<< " | json:" << json_params.toStyledString() << ", without key:" << CCarve::ms_str_carve_id_key
			, str_err_reason_for_debug, "��������", str_err_reason_for_user, MSP_ERROR_INVALID_PARA);
		const string& str_carve_id = json_params[CCarve::ms_str_carve_id_key].asString();
		boost::shared_ptr<CCarve> ptr_carve;
		//�����豸��Ų��Ҷ�Ӧ�ĵ�̻�
		{
			Thread_Read_Lock guard(m_rw_carveId_carvePtr);
			TYPE_MAP_ITER iter = m_map_carveId_carvePtr.find(str_carve_id);
			businlog_error_return_debug_and_user_reason(iter != m_map_carveId_carvePtr.end(), __CLASS_FUNCTION__ 
				<< " | Can not find carve id:" << str_carve_id << " in map", str_err_reason_for_debug
				, "�豸��Ŷ�Ӧ���豸δ����", str_err_reason_for_user, MSP_ERROR_NOT_FOUND);
			ptr_carve = iter->second;
		}
		//�ߵ����˵���ҵ��˶�Ӧ�ĵ�̻�
		//��ѯ��̻�״̬
		int ret = ptr_carve->get_carve_status(eCarve_common_status, str_err_reason_for_debug, str_err_reason_for_user);
		businlog_error_return(!ret, ("%s | fail to get carve status, reason:%s."
			, __CLASS_FUNCTION__, str_err_reason_for_debug.c_str()), ret);
		return MSP_SUCCESS;
	}
	catch (std::exception& e)
	{
		str_err_reason_for_debug = string("Has exception:") + string(e.what());
		str_err_reason_for_user = "�����쳣";
		businlog_error("%s | err reason:%s.", __CLASS_FUNCTION__, str_err_reason_for_debug.c_str());
		return MSP_ERROR_EXCEPTION;
	}
}

int CCarve_Manager::get_carve_info(const Json::Value& json_params, SCarve_Info& carve_info, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	try
	{
		//�Ӳ����л�ȡ�豸���
		businlog_error_return_debug_and_user_reason(json_params.isMember(CCarve::ms_str_carve_id_key), __CLASS_FUNCTION__
			<< " | json:" << json_params.toStyledString() << ", without key:" << CCarve::ms_str_carve_id_key
			, str_err_reason_for_debug, "��������", str_err_reason_for_user, MSP_ERROR_INVALID_PARA);
		const string& str_carve_id = json_params[CCarve::ms_str_carve_id_key].asString();
		boost::shared_ptr<CCarve> ptr_carve;
		//�����豸��Ų��Ҷ�Ӧ�ĵ�̻�
		{
			Thread_Read_Lock guard(m_rw_carveId_carvePtr);
			TYPE_MAP_ITER iter = m_map_carveId_carvePtr.find(str_carve_id);
			businlog_error_return_debug_and_user_reason(iter != m_map_carveId_carvePtr.end(), __CLASS_FUNCTION__ 
				<< " | Can not find carve id:" << str_carve_id << " in map", str_err_reason_for_debug
				, "�豸��Ŷ�Ӧ���豸δ����", str_err_reason_for_user, MSP_ERROR_NOT_FOUND);
			ptr_carve = iter->second;
		}
		//�ߵ����˵���ҵ��˶�Ӧ�ĵ�̻�
		//��ѯ�豸��Ϣ
		int ret = ptr_carve->get_info(carve_info, str_err_reason_for_debug, str_err_reason_for_user);
		businlog_error_return(!ret, ("%s | fail to get carve info, reason:%s."
			, __CLASS_FUNCTION__, str_err_reason_for_debug.c_str()), ret);
		return MSP_SUCCESS;
	}
	catch (std::exception& e)
	{
		str_err_reason_for_debug = string("Has exception:") + string(e.what());
		str_err_reason_for_user = "�����쳣";
		businlog_error("%s | err reason:%s.", __CLASS_FUNCTION__, str_err_reason_for_debug.c_str());
		return MSP_ERROR_EXCEPTION;
	}
}

/************************************
* Method:    get_all_carves_info
* Brief:  ��ȡ���е�̻�����Ϣ
* Access:    public 
* Returns:   int 0:���óɹ��������в��ֵ�̻�������0������ʧ��
* Qualifier:
*Parameter: const Json::Value & json_params -[in]  
*Parameter: Json::Value & json_result -[out]  ���е�̻���Ϣ��������Ӧ��json
*Parameter: string & str_err_reason_for_debug -[out]  
*Parameter: string & str_err_reason_for_user -[out]  
************************************/
int CCarve_Manager::get_all_carves_info(const Json::Value& json_params, Json::Value& json_result, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	businlog_tracer_perf(CCarve_Manager::get_all_carves_info);
	//TODO::���豸�ܶ�ʱ���˽ӿڻ�ܺ�ʱ��������Ҫȷ�ϲ��Ż�
	try
	{
		//�����������Ի�ȡ���е�̻�����Ϣ
		{
			Thread_Read_Lock guard(m_rw_carveId_carvePtr);
			//�ж������Ƿ�Ϊ��
			businlog_error_return_debug_and_user_reason(false == m_map_carveId_carvePtr.empty()
				, __CLASS_FUNCTION__ << " | there is no carve connected, please connect carve first."
				, str_err_reason_for_debug, "û���豸���ɹ����ӣ����������豸", str_err_reason_for_user, MSP_ERROR_NOT_FOUND);
			for (TYPE_MAP_ITER iter = m_map_carveId_carvePtr.begin(); iter != m_map_carveId_carvePtr.end(); ++iter)
			{
				if (!iter->second)
				{//ָ��Ϊ��
					businlog_error("%s | there is null in map.", __CLASS_FUNCTION__);
					continue;
				}
				SCarve_Info single_carve_info;
				string str_single_err_for_debug, str_single_err_for_user;
				//��ѯ�豸��Ϣ
				int ret = iter->second->get_info(single_carve_info, str_single_err_for_debug, str_single_err_for_user);
				//���۽����Σ�������������Ӧ�������������
				if (ret)
				{
					businlog_error("%s | fail to get carve info, carve id:%s, reason:%s."
						, __CLASS_FUNCTION__, iter->second->get_id().c_str(), str_single_err_for_debug.c_str());
				}
				Json::Value json_single_resp;
				//��������
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
				//�����������ӵ����������
				json_result["All_machines"].append(json_single_resp);
				//��������ˣ��򽫴�����Ϣ�ۼ�
				if (ret)
				{
					str_err_reason_for_debug += "." + str_single_err_for_debug;
					str_err_reason_for_user += "��" + str_single_err_for_user;
				}
			}
		}
		//������óɹ������ܿ��ܴ��ڵ���ʧ��
		return MSP_SUCCESS;
	}
	catch (std::exception& e)
	{
		str_err_reason_for_debug = string("Has exception:") + string(e.what());
		str_err_reason_for_user = "�����쳣";
		businlog_error("%s | err reason:%s.", __CLASS_FUNCTION__, str_err_reason_for_debug.c_str());
		return MSP_ERROR_EXCEPTION;
	}
}

/************************************
* Method:    start_poll_carve_status
* Brief:  �����߳�����ѯ��̻�״̬
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
		//�����̣߳���ѯ��̻�״̬
		m_bStop_poll_status = false;
		m_thread_poll_status = boost::thread(boost::bind(&CCarve_Manager::svc, this));
		return MSP_SUCCESS;
	}
	catch (std::exception& e)
	{
		str_err_reason_for_debug = string("Starting poll carve status has exception, reason:") + string(e.what());
		str_err_reason_for_user = "�������쳣";
		businlog_error("%s | err reason:%s.", __CLASS_FUNCTION__, str_err_reason_for_debug.c_str());
		return MSP_ERROR_EXCEPTION;
	}
}

/************************************
* Method:    stop_poll_carve_status
* Brief:  ֹͣ��ѯ��̻�״̬���̣߳����ȴ���ѯ�߳��˳�
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
		str_err_reason_for_user = "�����쳣";
		businlog_error("%s | err reason:%s.", __CLASS_FUNCTION__, str_err_reason_for_debug.c_str());
		return MSP_ERROR_INVALID_OPERATION;
	}
}

int CCarve_Manager::start_engraving(const Json::Value& json_params, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	try
	{
		//�Ӳ����л�ȡ�豸���
		businlog_error_return_debug_and_user_reason(json_params.isMember(CCarve::ms_str_carve_id_key), __CLASS_FUNCTION__
			<< " | json:" << json_params.toStyledString() << ", without key:" << CCarve::ms_str_carve_id_key
			, str_err_reason_for_debug, "��������", str_err_reason_for_user, MSP_ERROR_INVALID_PARA);
		const string& str_carve_id = json_params[CCarve::ms_str_carve_id_key].asString();
		boost::shared_ptr<CCarve> ptr_carve;
		//�����豸��Ų��Ҷ�Ӧ�ĵ�̻�
		{
			Thread_Read_Lock guard(m_rw_carveId_carvePtr);
			TYPE_MAP_ITER iter = m_map_carveId_carvePtr.find(str_carve_id);
			businlog_error_return_debug_and_user_reason(iter != m_map_carveId_carvePtr.end(), __CLASS_FUNCTION__ 
				<< " | Can not find carve id:" << str_carve_id << " in map", str_err_reason_for_debug
				, "�豸��Ŷ�Ӧ���豸δ����", str_err_reason_for_user, MSP_ERROR_NOT_FOUND);
			ptr_carve = iter->second;
		}
		//�ߵ����˵���ҵ��˶�Ӧ�ĵ�̻�
		//��ʼ���
		int ret = ptr_carve->start(json_params, str_err_reason_for_debug, str_err_reason_for_user);
		businlog_error_return(!ret, ("%s | fail to start carve, reason:%s."
			, __CLASS_FUNCTION__, str_err_reason_for_debug.c_str()), ret);
#ifdef SERVER_WITH_CONTROL_LOGIC
		//��ʼ��̳ɹ��������ÿ�ʼ���ʱ��ʱ��
		ptr_carve->start_count_engraving_time();
#endif
		return MSP_SUCCESS;
	}
	catch (std::exception& e)
	{
		str_err_reason_for_debug = string("Has exception:") + string(e.what());
		str_err_reason_for_user = "�����쳣";
		businlog_error("%s | err reason:%s.", __CLASS_FUNCTION__, str_err_reason_for_debug.c_str());
		return MSP_ERROR_EXCEPTION;
	}
}

int CCarve_Manager::upload_1_file(const Json::Value& json_params, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	try
	{
		//�Ӳ����л�ȡ�豸���
		businlog_error_return_debug_and_user_reason(json_params.isMember(CCarve::ms_str_carve_id_key), __CLASS_FUNCTION__
			<< " | json:" << json_params.toStyledString() << ", without key:" << CCarve::ms_str_carve_id_key
			, str_err_reason_for_debug, "��������", str_err_reason_for_user, MSP_ERROR_INVALID_PARA);
		const string& str_carve_id = json_params[CCarve::ms_str_carve_id_key].asString();
		boost::shared_ptr<CCarve> ptr_carve;
		//�����豸��Ų��Ҷ�Ӧ�ĵ�̻�
		{
			Thread_Read_Lock guard(m_rw_carveId_carvePtr);
			TYPE_MAP_ITER iter = m_map_carveId_carvePtr.find(str_carve_id);
			businlog_error_return_debug_and_user_reason(iter != m_map_carveId_carvePtr.end(), __CLASS_FUNCTION__ 
				<< " | Can not find carve id:" << str_carve_id << " in map", str_err_reason_for_debug
				, "�豸��Ŷ�Ӧ���豸δ����", str_err_reason_for_user, MSP_ERROR_NOT_FOUND);
			ptr_carve = iter->second;
		}
		//�ߵ����˵���ҵ��˶�Ӧ�ĵ�̻�
		//����G�����ļ�
		int ret = ptr_carve->upload_1_file(json_params, str_err_reason_for_debug, str_err_reason_for_user);
		businlog_error_return(!ret, ("%s | fail to start carve, reason:%s."
			, __CLASS_FUNCTION__, str_err_reason_for_debug.c_str()), ret);
		return MSP_SUCCESS;
	}
	catch (std::exception& e)
	{
		str_err_reason_for_debug = string("Has exception:") + string(e.what());
		str_err_reason_for_user = "�����쳣";
		businlog_error("%s | err reason:%s.", __CLASS_FUNCTION__, str_err_reason_for_debug.c_str());
		return MSP_ERROR_EXCEPTION;
	}
}

int CCarve_Manager::delete_1_file(const Json::Value& json_params, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	try
	{
		//�Ӳ����л�ȡ�豸���
		businlog_error_return_debug_and_user_reason(json_params.isMember(CCarve::ms_str_carve_id_key), __CLASS_FUNCTION__
			<< " | json:" << json_params.toStyledString() << ", without key:" << CCarve::ms_str_carve_id_key
			, str_err_reason_for_debug, "��������", str_err_reason_for_user, MSP_ERROR_INVALID_PARA);
		const string& str_carve_id = json_params[CCarve::ms_str_carve_id_key].asString();
		boost::shared_ptr<CCarve> ptr_carve;
		//�����豸��Ų��Ҷ�Ӧ�ĵ�̻�
		{
			Thread_Read_Lock guard(m_rw_carveId_carvePtr);
			TYPE_MAP_ITER iter = m_map_carveId_carvePtr.find(str_carve_id);
			businlog_error_return_debug_and_user_reason(iter != m_map_carveId_carvePtr.end(), __CLASS_FUNCTION__ 
				<< " | Can not find carve id:" << str_carve_id << " in map", str_err_reason_for_debug
				, "�豸��Ŷ�Ӧ���豸δ����", str_err_reason_for_user, MSP_ERROR_NOT_FOUND);
			ptr_carve = iter->second;
		}
		//�ߵ����˵���ҵ��˶�Ӧ�ĵ�̻�
		//ɾ��G�����ļ�
		int ret = ptr_carve->delete_1_file(json_params, str_err_reason_for_debug, str_err_reason_for_user);
		businlog_error_return(!ret, ("%s | fail to start carve, reason:%s."
			, __CLASS_FUNCTION__, str_err_reason_for_debug.c_str()), ret);
		return MSP_SUCCESS;
	}
	catch (std::exception& e)
	{
		str_err_reason_for_debug = string("Has exception:") + string(e.what());
		str_err_reason_for_user = "�����쳣";
		businlog_error("%s | err reason:%s.", __CLASS_FUNCTION__, str_err_reason_for_debug.c_str());
		return MSP_ERROR_EXCEPTION;
	}
}

int CCarve_Manager::emergency_stop_one(const Json::Value& json_params, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	//��̻���ͣ
	try
	{
		//�Ӳ����л�ȡ�豸���
		businlog_error_return_debug_and_user_reason(json_params.isMember(CCarve::ms_str_carve_id_key), __CLASS_FUNCTION__
			<< " | json:" << json_params.toStyledString() << ", without key:" << CCarve::ms_str_carve_id_key
			, str_err_reason_for_debug, "��������", str_err_reason_for_user, MSP_ERROR_INVALID_PARA);
		const string& str_carve_id = json_params[CCarve::ms_str_carve_id_key].asString();
		boost::shared_ptr<CCarve> ptr_carve;
		//�����豸��Ų��Ҷ�Ӧ�ĵ�̻�
		{
			Thread_Read_Lock guard(m_rw_carveId_carvePtr);
			TYPE_MAP_ITER iter = m_map_carveId_carvePtr.find(str_carve_id);
			businlog_error_return_debug_and_user_reason(iter != m_map_carveId_carvePtr.end(), __CLASS_FUNCTION__ 
				<< " | Can not find carve id:" << str_carve_id << " in map", str_err_reason_for_debug
				, "�豸��Ŷ�Ӧ���豸δ����", str_err_reason_for_user, MSP_ERROR_NOT_FOUND);
			ptr_carve = iter->second;
		}
		//�ߵ����˵���ҵ��˶�Ӧ�ĵ�̻�
		int ret = ptr_carve-> stop_fast(json_params, str_err_reason_for_debug, str_err_reason_for_user);
		businlog_error_return(!ret, ("%s | fail to fast stop carve, reason:%s."
			, __CLASS_FUNCTION__, str_err_reason_for_debug.c_str()), ret);
		return MSP_SUCCESS;
	}
	catch (std::exception& e)
	{
		str_err_reason_for_debug = string("Has exception:") + string(e.what());
		str_err_reason_for_user = "�����쳣";
		businlog_error("%s | err reason:%s.", __CLASS_FUNCTION__, str_err_reason_for_debug.c_str());
		return MSP_ERROR_EXCEPTION;
	}
}
int CCarve_Manager::emergency_stop_all(const Json::Value& json_params,Json::Value& json_result,string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	//ȫ����̻���ͣ
	try
	{
		{
			//��ѯ���е��������豸���
			Thread_Read_Lock guard(m_rw_carveId_carvePtr);
			//�ж������Ƿ�Ϊ��
			businlog_error_return_debug_and_user_reason(false == m_map_carveId_carvePtr.empty()
				, __CLASS_FUNCTION__ << " | there is no carve connected, please connect carve first."
				, str_err_reason_for_debug, "û���豸���ɹ����ӣ����������豸", str_err_reason_for_user, MSP_ERROR_NOT_FOUND);
			for (TYPE_MAP_ITER iter = m_map_carveId_carvePtr.begin();iter!=m_map_carveId_carvePtr.end(); ++iter)
			{	
				//�ж������Ƿ�Ϸ�
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
				//������
				json_single_resp["ret"] = ret;
				json_single_resp["errmsg"] = str_single_err_for_debug;
				json_single_resp["errmsg_for_user"] = sp::toutf8(str_single_err_for_user);
				json_single_resp[CCarve::ms_str_carve_id_key] = iter->second->get_id();

				//�����������ӵ����������
				json_result["results"].append(json_single_resp);
				//��������ˣ��򽫴�����Ϣ�ۼ�
				if (ret)
				{
					str_err_reason_for_debug += "." + str_single_err_for_debug;
					str_err_reason_for_user += "��" + str_single_err_for_user;
				}
			}
		}
		
		//�����������,���سɹ�������ʧ�����ϲ�json "results" �ڵ��ж�
		return MSP_SUCCESS;
	}
	catch (std::exception& e)
	{
		str_err_reason_for_debug = string("Has exception:") + string(e.what());
		str_err_reason_for_user = "�����쳣";
		businlog_error("%s | err reason:%s.", __CLASS_FUNCTION__, str_err_reason_for_debug.c_str());
		return MSP_ERROR_EXCEPTION;
	}
}
int CCarve_Manager::cancel_emergency_stop_one(const Json::Value& json_params, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	//��̻���ͣ
	try
	{
		//�Ӳ����л�ȡ�豸���
		businlog_error_return_debug_and_user_reason(json_params.isMember(CCarve::ms_str_carve_id_key), __CLASS_FUNCTION__
			<< " | json:" << json_params.toStyledString() << ", without key:" << CCarve::ms_str_carve_id_key
			, str_err_reason_for_debug, "��������", str_err_reason_for_user, MSP_ERROR_INVALID_PARA);
		const string& str_carve_id = json_params[CCarve::ms_str_carve_id_key].asString();
		boost::shared_ptr<CCarve> ptr_carve;
		//�����豸��Ų��Ҷ�Ӧ�ĵ�̻�
		{
			Thread_Read_Lock guard(m_rw_carveId_carvePtr);
			TYPE_MAP_ITER iter = m_map_carveId_carvePtr.find(str_carve_id);
			businlog_error_return_debug_and_user_reason(iter != m_map_carveId_carvePtr.end(), __CLASS_FUNCTION__ 
				<< " | Can not find carve id:" << str_carve_id << " in map", str_err_reason_for_debug
				, "�豸��Ŷ�Ӧ���豸δ����", str_err_reason_for_user, MSP_ERROR_NOT_FOUND);
			ptr_carve = iter->second;
		}
		//�ߵ����˵���ҵ��˶�Ӧ�ĵ�̻�
		int ret = ptr_carve-> cancel_fast_stop(json_params, str_err_reason_for_debug, str_err_reason_for_user);
		businlog_error_return(!ret, ("%s | fail to fast stop carve, reason:%s."
			, __CLASS_FUNCTION__, str_err_reason_for_debug.c_str()), ret);
		return MSP_SUCCESS;
	}
	catch (std::exception& e)
	{
		str_err_reason_for_debug = string("Has exception:") + string(e.what());
		str_err_reason_for_user = "�����쳣";
		businlog_error("%s | err reason:%s.", __CLASS_FUNCTION__, str_err_reason_for_debug.c_str());
		return MSP_ERROR_EXCEPTION;
	}
}
int CCarve_Manager::cancel_emergency_stop_all(const Json::Value& json_params,Json::Value& json_result,string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	//ȫ����̻���ͣ
	try
	{
		{
			//��ѯ���е��������豸���
			Thread_Read_Lock guard(m_rw_carveId_carvePtr);
			//�ж������Ƿ�Ϊ��
			businlog_error_return_debug_and_user_reason(false == m_map_carveId_carvePtr.empty()
				, __CLASS_FUNCTION__ << " | there is no carve connected, please connect carve first."
				, str_err_reason_for_debug, "û���豸���ɹ����ӣ����������豸", str_err_reason_for_user, MSP_ERROR_NOT_FOUND);
			for (TYPE_MAP_ITER iter = m_map_carveId_carvePtr.begin();iter!=m_map_carveId_carvePtr.end(); ++iter)
			{	
				//�ж������Ƿ�Ϸ�
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
				//������
				json_single_resp["ret"] = ret;
				json_single_resp["errmsg"] = str_single_err_for_debug;
				json_single_resp["errmsg_for_user"] = sp::toutf8(str_single_err_for_user);
				json_single_resp[CCarve::ms_str_carve_id_key] = iter->second->get_id();

				//�����������ӵ����������
				json_result["results"].append(json_single_resp);
				//��������ˣ��򽫴�����Ϣ�ۼ�
				if (ret)
				{
					str_err_reason_for_debug += "." + str_single_err_for_debug;
					str_err_reason_for_user += "��" + str_single_err_for_user;
				}
			}
		}

		//�����������,���سɹ�������ʧ�����ϲ�json "results" �ڵ��ж�
		return MSP_SUCCESS;
	}
	catch (std::exception& e)
	{
		str_err_reason_for_debug = string("Has exception:") + string(e.what());
		str_err_reason_for_user = "�����쳣";
		businlog_error("%s | err reason:%s.", __CLASS_FUNCTION__, str_err_reason_for_debug.c_str());
		return MSP_ERROR_EXCEPTION;
	}
}

int CCarve_Manager::adjust_speed(const Json::Value& json_params, string& str_err_reason_for_debug, string& str_err_reason_for_user)
{
	//������̻������ٶ�
	try
	{
		//�Ӳ����л�ȡ�豸���
		businlog_error_return_debug_and_user_reason(json_params.isMember(CCarve::ms_str_carve_id_key), __CLASS_FUNCTION__
			<< " | json:" << json_params.toStyledString() << ", without key:" << CCarve::ms_str_carve_id_key
			, str_err_reason_for_debug, "��������", str_err_reason_for_user, MSP_ERROR_INVALID_PARA);
		const string& str_carve_id = json_params[CCarve::ms_str_carve_id_key].asString();
		boost::shared_ptr<CCarve> ptr_carve;
		//�����豸��Ų��Ҷ�Ӧ�ĵ�̻�
		{
			Thread_Read_Lock guard(m_rw_carveId_carvePtr);
			TYPE_MAP_ITER iter = m_map_carveId_carvePtr.find(str_carve_id);
			businlog_error_return_debug_and_user_reason(iter != m_map_carveId_carvePtr.end(), __CLASS_FUNCTION__ 
				<< " | Can not find carve id:" << str_carve_id << " in map", str_err_reason_for_debug
				, "�豸��Ŷ�Ӧ���豸δ����", str_err_reason_for_user, MSP_ERROR_NOT_FOUND);
			ptr_carve = iter->second;
		}
		//�ߵ����˵���ҵ��˶�Ӧ�ĵ�̻�
		int ret = ptr_carve-> adjust_speed(json_params, str_err_reason_for_debug, str_err_reason_for_user);
		businlog_error_return(!ret, ("%s | fail to fast stop carve, reason:%s."
			, __CLASS_FUNCTION__, str_err_reason_for_debug.c_str()), ret);
		return MSP_SUCCESS;
	}
	catch (std::exception& e)
	{
		str_err_reason_for_debug = string("Has exception:") + string(e.what());
		str_err_reason_for_user = "�����쳣";
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
	//��ֹͣ�̺߳��ͷź�����
	if (false == m_bStop_poll_status)
	{//�߳���Ȼ��ʼ��
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
	size_t nWait_time_minute = 1; //ÿ������ʱ��,TODO::Ŀǰ����С�㣬ʵ�������ô��
	businlog_warn("%s | please read the param from configure file", __CLASS_FUNCTION__);
	try
	{
		while (false == m_bStop_poll_status)
		{
			boost::this_thread::sleep(boost::posix_time::minutes(nWait_time_minute));
			int ret = 0;
			//������̻��б�
			{ 
				Thread_Read_Lock guard(m_rw_carveId_carvePtr);
				for (TYPE_MAP_ITER iter = m_map_carveId_carvePtr.begin()
					; iter != m_map_carveId_carvePtr.end(); ++ iter)
				{
					//�ж������Ƿ�Ϸ�
					if (!iter->second)
					{
						businlog_error("%s | ptr carve is NULL, carve id:%s" , __CLASS_FUNCTION__, iter->first.c_str());
						continue;
					}
					//��ȡ�豸״̬
					ECARVE_STATUS_TYPE eCarve_common_status = CARVE_STATUS_MIN;
					string str_err_reason_for_debug, str_err_reason_for_user;
					ret = iter->second->get_carve_status(eCarve_common_status, str_err_reason_for_debug
						, str_err_reason_for_user);
					if (ret)
					{//��ȡ״̬����
						businlog_error("%s | fail to get carve status, carve id:%s, reason:%s"
							, __CLASS_FUNCTION__, iter->first.c_str(), str_err_reason_for_debug.c_str());
						continue;
					}
					//��ʱ�ɹ���ȡ�豸״̬
					if (CARVE_STATUS_COMPLETED == eCarve_common_status)
					{
						//���Ϊ�����ɣ�����ͣ�ۼӵ��ʱ��
						ret = iter->second->pause_count_engraving_time(str_err_reason_for_debug, str_err_reason_for_user);
						if (ret)
						{
							businlog_error("%s | fail to pause count engraving time, carve id:%s, reason:%s."
								, __CLASS_FUNCTION__, iter->first.c_str(), str_err_reason_for_debug.c_str());
						}
					}
				}//end for ����map
			}
		}//end whiel �߳�
	}
	catch (boost::thread_interrupted& ) //���ж�ʱ�����׳����쳣
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
