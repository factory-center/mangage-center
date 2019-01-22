/******************************************************************
* Copyright (c) 2016-2017,HIIRI Inc.
* All rights reserved.
* @file: CCarve.h
* @brief: ���˵���ļ����ܡ���; (Comment)��
* @author:	minglu2
* @version: 1.0
* @date: 2018/12/14
* 
* @see
* 
* <b>�汾��¼��</b><br>
* <table>
*  <tr> <th>�汾	<th>����		<th>����	<th>��ע </tr>
*  <tr> <td>1.0	    <td>2018/12/14	<td>minglu	<td>Create head file </tr>
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
#include "CDevice.h"
#include "boost_common.h"
#include "carve_common_lib.h"
//�궨��
//#define  SERVER_WITH_CONTROL_LOGIC //�˷���߱������߼�
//��̻���Ϣ
struct SCarve_Info
{
	string str_task_no; //������
	string str_machine_ip; //�豸ip��ַ
	ECARVE_STATUS_TYPE eCarve_status; //��̻�״̬
	size_t nTotal_engraving_time; //��̻��ܵĵ��ʱ�䣬��λ����
	size_t nSingle_engraving_time; //��̻����μӹ�ʱ�䣬��λ����
	string str_gCode_no; //G������
	int nCurrent_line_num; //��ǰ��̵�G�����к�
	string str_id; //�豸���
	SCarve_Info();
};
//���Ͷ���
class  CCarve : public CDevice
{
public:
	CCarve(const Json::Value& json_params);
	int connect(string& str_err_reason_for_debug, string& str_err_reason_for_user);
	int disconnect(string& str_err_reason_for_debug, string& str_err_reason_for_user);
	int set_continue_status(const Json::Value& json_params, string& str_err_reason_for_debug, string& str_err_reason_for_user);
	int reset(const Json::Value& json_params, string& str_err_reason_for_debug, string& str_err_reason_for_user);
	int start(const Json::Value& json_params, string& str_err_reason_for_debug, string& str_err_reason_for_user);
	int pause(const Json::Value& json_params, string& str_err_reason_for_debug, string& str_err_reason_for_user);
	int upload_1_file(const Json::Value& json_params, string& str_err_reason_for_debug, string& str_err_reason_for_user);
	int get_carve_status(ECARVE_STATUS_TYPE& eCarve_common_status, string& str_err_reason_for_debug, string& str_err_reason_for_user);
	int stop_fast(const Json::Value& json_params,string& str_err_reason_for_debug, string& str_err_reason_for_user);
	int cancel_fast_stop(const Json::Value& json_params, string& str_err_reason_for_debug, string& str_err_reason_for_user);
	int delete_1_file(const Json::Value& json_params, string& str_err_reason_for_debug, string& str_err_reason_for_user);
	int get_current_line_num(int& nCurrent_line_num, string& str_err_reason_for_debug, string& str_err_reason_for_user);
	int acquire_resource(string& str_err_reason_for_debug, string& str_err_reason_for_user);
	int release_resource(string& str_err_reason_for_debug, string& str_err_reason_for_user);
	int get_info(SCarve_Info& carve_info, string& str_err_reason_for_debug, string& str_err_reason_for_user);
	int adjust_speed(const Json::Value& json_params,string& str_err_reason_for_debug, string& str_err_reason_for_user);
	int  get_engraving_time(const Json::Value& json_params, size_t& nTotal_engraving_time_minute, size_t& nSingle_engraving_time_minute, string& str_err_reason_for_debug, string& str_err_reason_for_user);
	//��̳ɹ������
	void start_count_engraving_time();
	//�����ɺ����
	int pause_count_engraving_time(string& str_err_reason_for_debug, string& str_err_reason_for_user);
	unsigned short Conn_idx() const { return m_nConn_idx; }
	void Conn_idx(unsigned short val) { m_nConn_idx = val; }
	static const string ms_str_factory_type_key; 
	static const string ms_str_carve_type_key;
	static const string ms_str_conn_idx_key;
	static const string ms_str_ip_key;
	static const string ms_str_file_path_key;
	static const string ms_str_status_key;
	static const string ms_str_max_wait_time_key;
	static const string ms_str_carve_id_key;
	static const string ms_str_task_no_key;
	static const string ms_str_gCode_no_key;
	static const string ms_str_worktime_key;
	virtual ~CCarve();
protected:
	CCarve();
	CCarve(const CCarve&);
	CCarve& operator=(const CCarve&);
private:
	int m_nConn_idx;//һ����̻���Ӧһ�����ӱ�ţ�Ψһ��ʶһ������������ֵ����С��ConnectNum��ȡֵ��Χ[0, ConnectNum-1]
    Uni_Mutex m_mutex_for_cmd; //ͬһʱ��ֻ��ִ��һ������
	bool m_bConnected; //��̻���ǰ�Ƿ���������������ÿ��ֻ��ִ��һ��������������������
	bool m_bAcq_Res_Success; //������Դ�Ƿ�ɹ�
	ECARVE_FACTORY_TYPE m_eFactory_type; //��̻���������
	string m_str_carve_type; //�豸�ͺ�
	string m_str_task_no; //������
	string m_str_gCode_no; //G������
	string m_str_file_path; //G����·��
#ifdef SERVER_WITH_CONTROL_LOGIC
	boost::posix_time::ptime m_time_last_start; //�ϴο�ʼ���ʱ��
	size_t m_nTotal_engraving_time; //�ܵĵ��ʱ�䣬��λ���� ��һ��Ҫ��Ͽ����߼�����������ȷ��
	                                //��������������ͣ����ᵼ��ʱ����󡣵���������ѯʱ�����Ĵ���
                                    //	�� ������Ϣ�и��ӳ٣�
	ECARVE_STATUS_TYPE m_eCarve_status; //��̻�״̬
#endif
	
};
//����ԭ�Ͷ���
