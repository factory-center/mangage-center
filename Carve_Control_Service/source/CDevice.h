/******************************************************************
* Copyright (c) 2016-2017,HIIRI Inc.
* All rights reserved.
* @file: CDevice.h
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
#include <vector>
#include <string>
using std::string;
using std::vector;
//�궨��

enum EDevice_Type
{
	ECARVE = 0, //��̻�
	ECLOULD = 1, //������
	EHOST = 2, //��������
};

//���Ͷ���
class CDevice
{
public:
protected:

private:
	vector<string> m_vec_IPs; //�豸��ip��ַ�б����ܴ��ڶ�����
	size_t m_nId; //�������豸�������ܿط����ж�Ӧ�ı��
	EDevice_Type m_eType; //�豸����
};
//����ԭ�Ͷ���
