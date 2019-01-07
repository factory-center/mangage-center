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
using std::string;
//�궨��
class CCarve;
//���Ͷ���
class CCarve_Manager
{
public:
	static CCarve_Manager* instance();
	int connect_carve(const Json::Value& json_params, string& str_err_reason);
protected:
	CCarve_Manager();
	~CCarve_Manager();
private:
	CCarve_Manager& operator=(const CCarve_Manager&);
	CCarve_Manager(const CCarve_Manager&);
	class CGarbo // ����Ψһ��������������������ɾ��CSingleton��ʵ�� 
	{
	public:
		~CGarbo();
	};
	// ����һ����̬��Ա���ڳ������ʱ��ϵͳ�������������������ע���������������������Ҫ����Ӧ��cpp�ļ��жԾ�̬��Ա���ж���Ŷ��
	static CGarbo Garbo;
	static CCarve_Manager* ms_pInstance;
	std::map<string, boost::shared_ptr<CCarve>> m_map_carveId_carvePtr; //��Ϊ�豸��ţ�ֵΪCCarve������ָ��
	Thread_WR_Mutex m_rw_carveId_carvePtr;
};
//����ԭ�Ͷ���
