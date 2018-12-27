/******************************************************************
* Copyright (c) 2016-2017,HIIRI Inc.
* All rights reserved.
* @file: carve_common_lib.h
* @brief: ��̻��豸�ײ�ͨ�ÿ�
* @author:	minglu2
* @version: 1.0
* @date: 2018/12/26
* 
* @see
* 
* <b>�汾��¼��</b><br>
* <table>
*  <tr> <th>�汾	<th>����		<th>����	<th>��ע </tr>
*  <tr> <td>1.0	    <td>2018/12/26	<td>minglu	<td>Create head file </tr>
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
#include <string>
#include <json/json.h>
using std::string;
//�궨��
enum ECARVE_FACTORY_TYPE
{
	CARVE_FACTORY_TYPE_MIN = 0,
	CARVE_FACTORY_TYPE_BAOYUAN, //��Ԫǧϵ
	CARVE_FACTORY_TYPE_MAX,
};

//���Ͷ���
//��̻��ײ�ͨ�ÿ⣬������豸��Ϣ��������ʹ���ĸ�������
class CCarve_Common_Lib_Tool
{
public:
	static CCarve_Common_Lib_Tool* instance()
	{
		return ms_pInstance;
	}
	bool init(string& str_kernel_err_reason);
	bool is_baoyuan(int nfactory_type, const string& str_carve_type_key);
	bool connect(const Json::Value& json_conn_value, string& str_kernel_err_reason);
	bool disconnect(const Json::Value& json_conn_value, string& str_kernel_err_reason);
	bool upload_1file(const Json::Value& json_conn_value, string& str_kernel_err_reason);
	bool set_continue_status(const Json::Value& json_conn_value, string& str_kernel_err_reason);
	bool reset_carve(const Json::Value& json_conn_value, string& str_kernel_err_reason);
	bool pause(const Json::Value& json_conn_value, string& str_kernel_err_reason);
	bool start(const Json::Value& json_conn_value, string& str_kernel_err_reason);
protected:
	bool get_carve_factory_and_type(const Json::Value& json_conn_value, int& nfactory_type, string& str_str_carve_type_key, string& str_kernel_err_reason);
	CCarve_Common_Lib_Tool()
	{

	}
	~CCarve_Common_Lib_Tool();
private:
	CCarve_Common_Lib_Tool& operator=(const CCarve_Common_Lib_Tool&);
	CCarve_Common_Lib_Tool(const CCarve_Common_Lib_Tool&);
	class CGarbo // ����Ψһ��������������������ɾ��CSingleton��ʵ�� 
	{
	public:
		~CGarbo();
	};
	// ����һ����̬��Ա���ڳ������ʱ��ϵͳ�������������������ע���������������������Ҫ����Ӧ��cpp�ļ��жԾ�̬��Ա���ж���Ŷ��
	static CGarbo Garbo;
	static CCarve_Common_Lib_Tool* ms_pInstance;
};
//����ԭ�Ͷ���
