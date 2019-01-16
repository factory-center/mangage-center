/******************************************************************
* Copyright (c) 2016-2017,HIIRI Inc.
* All rights reserved.
* @file: carve_manger.h
* @brief: ��̻������࣬���ڹ������е�̻�
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
#pragma once

#ifdef __cplusplus  
extern "C" {  
//����C���Խӿڡ������ͷ�ļ�
#endif  
#ifdef __cplusplus  
}  
#endif  
//����C++ͷ�ļ������Ǳ�׼��ͷ�ļ���������Ŀͷ�ļ�
#include <boost_common.h>
#include <string>
#include <json/json.h>
#include "carve_common_lib.h"
#include "CCarve.h"
using std::string;
//�궨��
//���Ͷ���
class CCarve_Manager
{
public:
	static CCarve_Manager* instance();
	int connect_carve(const Json::Value& json_params, string& str_err_reason_for_debug, string& str_err_reason_for_user);
	int disconnect_carve(const Json::Value& json_params, string& str_err_reason_for_debug, string& str_err_reason_for_user);
	int get_carve_status(const Json::Value& json_params, ECARVE_STATUS_TYPE& eCarve_common_status, string& str_err_reason_for_debug, string& str_err_reason_for_user);
    int get_carve_info(const Json::Value& json_params, SCarve_Info& carve_info, string& str_err_reason_for_debug, string& str_err_reason_for_user);
	int get_all_carves_info(const Json::Value& json_params, Json::Value& json_result
		, string& str_err_reason_for_debug, string& str_err_reason_for_user);
	int start_poll_carve_status(string& str_err_reason_for_debug, string& str_err_reason_for_user);
	int stop_poll_carve_status(string& str_err_reason_for_debug, string& str_err_reason_for_user);
	int start_engraving(const Json::Value& json_params, string& str_err_reason_for_debug, string& str_err_reason_for_user);
    int upload_1_file(const Json::Value& json_params, string& str_err_reason_for_debug, string& str_err_reason_for_user);
	int emergency_stop_one(const Json::Value& json_params, string& str_err_reason_for_debug, string& str_err_reason_for_user);
	int emergency_stop_all(const Json::Value& json_params,Json::Value& json_result,string& str_err_reason_for_debug, string& str_err_reason_for_user);
	int adjust_speed(const Json::Value& json_params, string& str_err_reason_for_debug, string& str_err_reason_for_user);
	
protected:
	CCarve_Manager();
	~CCarve_Manager();
	void svc();
private:
	CCarve_Manager& operator=(const CCarve_Manager&);
	CCarve_Manager(const CCarve_Manager&);
	class CGarbo // ����Ψһ��������������������ɾ��CSingleton��ʵ�� 
	{
	public:
		~CGarbo();
	};
	typedef std::map<string, boost::shared_ptr<CCarve>>::iterator TYPE_MAP_ITER;
	// ����һ����̬��Ա���ڳ������ʱ��ϵͳ�������������������ע���������������������Ҫ����Ӧ��cpp�ļ��жԾ�̬��Ա���ж���Ŷ��
	static CGarbo Garbo;
	static CCarve_Manager* ms_pInstance;
	std::map<string, boost::shared_ptr<CCarve>> m_map_carveId_carvePtr; //��Ϊ�豸��ţ�ֵΪCCarve������ָ��
	Thread_WR_Mutex m_rw_carveId_carvePtr;
	boost::thread m_thread_poll_status;
	bool m_bStop_poll_status; //״̬��ѯ����
};
//����ԭ�Ͷ���
