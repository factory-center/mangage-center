/******************************************************************
* Copyright (c) 2016-2017,HIIRI Inc.
* All rights reserved.
* @file: Resource_Manager.h
* @brief: ʹ����Դ��������������Դ��
* @author:	minglu2
* @version: 1.0
* @date: 2017/09/29
* 
* @see
* 
*  History: 
*  �汾	 ����       ����       ��ע
*  1.0	 2017/09/29     minglu2    ����
*****************************************************************/
#pragma once

#ifdef __cplusplus  
extern "C" {  
//����C���Խӿڡ������ͷ�ļ�
#include <stdio.h>
#endif  
#ifdef __cplusplus  
}  
#endif  
//����C++ͷ�ļ������Ǳ�׼��ͷ�ļ���������Ŀͷ�ļ�

//�궨��

//���Ͷ���
/************************************************************************/
/*   ����������һ�����飬�����鲻�ɿ纯��ʹ��                                                                  */
/************************************************************************/
template <typename Element_Type>
class CNewBuffMngr
{
public:
	//����һ���ڴ沢�����׵�ַ
	CNewBuffMngr(Element_Type*& pElement, size_t Size_Of_Array)
	{
		pElement = new Element_Type[Size_Of_Array];
		memset(pElement, 0, Size_Of_Array);
		m_pFirst_element = pElement;
	}

	~CNewBuffMngr()
	{
		if (m_pFirst_element)
		{
			delete[] m_pFirst_element;
			m_pFirst_element = NULL;
		}
	}
protected:
private:
	CNewBuffMngr& operator=(const CNewBuffMngr&);
	CNewBuffMngr(const CNewBuffMngr&);
    Element_Type* m_pFirst_element;
};

/************************************************************************/
/* ����������һ���ļ�����������鲻�ɿ纯��ʹ��                                                                  */
/************************************************************************/
class CFileMng
{
public:
	/** 
	* @ func:  CFileMng
	* @ brief:  ����ָ��ģʽ��ָ���ļ��������ض�Ӧ���ļ�ָ�룬��������򷵻�NULL
	* 
	* @author minlgu2
	* Access: public 
	* @return: �����򷵻�NULL�����򷵻��ļ�ָ��
	* @param: FILE *  & pFile [out] ����ΪNULL
	* @param: const char * file_patch �ļ�·��
	* @param: const char * mode ��ģʽ
	* @see
	*/
	CFileMng(FILE* &pFile ,const char* file_patch, const char* mode)
	{
		if (file_patch && mode)
		{
#ifdef _WINDOWS
			fopen_s(&m_pFile, file_patch, mode);
#else
			m_pFile = fopen(file_patch, mode);
#endif
		}
		else
		{//�������Ϸ�
			m_pFile =  NULL;
		}
		pFile = m_pFile;
	}

	~CFileMng()
	{
		if (m_pFile)
		{
			fclose(m_pFile);
			m_pFile = NULL;
		}
	}
protected:
private:
	CFileMng& operator=(const CFileMng&);
	CFileMng(const CFileMng&);
	FILE* m_pFile;
};

/************************************************************************/
/*  ��ͨ����Դ����ģ�飬�ڶ�������ʱ����ע������Ļص�������                                                                    */
/************************************************************************/
typedef void* (*CallBack_Func_Type)(void*);
class CCommonResMng
{
public:

	/** 
	* @ func:  CCommonResMng
	* @ brief: �����ⲿ��Դ�����ͷź���������һ����Դ�������
	* @author minglu2
	* Access: public 
	* @return: 
	* @param: void * pRes ��Դ
	* @param: CallBack_Func_Type pRelease_Source_Callback_Fun  ָ�������ͷ���Դ�ĺ���
	* @see
	*/
	CCommonResMng(void *pRes ,CallBack_Func_Type pRelease_Source_Callback_Fun)
	{
		m_pRes = pRes;
		m_pRelease_Source_Callback_Fun = pRelease_Source_Callback_Fun;
	}
	virtual ~CCommonResMng(void)
	{
		if (m_pRes && m_pRelease_Source_Callback_Fun)
		{
			m_pRelease_Source_Callback_Fun(m_pRes);
			m_pRelease_Source_Callback_Fun = NULL;
		}
	}
protected:
private:
	CCommonResMng& operator=(const CCommonResMng&);
	CCommonResMng(const CCommonResMng&);
	CallBack_Func_Type m_pRelease_Source_Callback_Fun;  //����ָ�룬ָ�������ͷ���Դ�Ļص�����
	void* m_pRes; //ָ���ⲿ����Դʵ�����
};
//����ԭ�Ͷ���
