/******************************************************************
* Copyright (c) 2016-2017,HIIRI Inc.
* All rights reserved.
* @file: Resource_Manager.h
* @brief: 使用资源管理类来管理资源。
* @author:	minglu2
* @version: 1.0
* @date: 2017/09/29
* 
* @see
* 
*  History: 
*  版本	 日期       作者       备注
*  1.0	 2017/09/29     minglu2    创建
*****************************************************************/
#pragma once

#ifdef __cplusplus  
extern "C" {  
//包含C语言接口、定义或头文件
#include <stdio.h>
#endif  
#ifdef __cplusplus  
}  
#endif  
//引用C++头文件：先是标准库头文件，后是项目头文件

//宏定义

//类型定义
/************************************************************************/
/*   创建并管理一个数组，此数组不可跨函数使用                                                                  */
/************************************************************************/
template <typename Element_Type>
class CNewBuffMngr
{
public:
	//创建一个内存并返回首地址
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
/* 创建并管理一个文件句柄，此数组不可跨函数使用                                                                  */
/************************************************************************/
class CFileMng
{
public:
	/** 
	* @ func:  CFileMng
	* @ brief:  按照指定模式打开指定文件，并返回对应的文件指针，如果出错则返回NULL
	* 
	* @author minlgu2
	* Access: public 
	* @return: 出错则返回NULL；否则返回文件指针
	* @param: FILE *  & pFile [out] 可能为NULL
	* @param: const char * file_patch 文件路径
	* @param: const char * mode 打开模式
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
		{//参数不合法
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
/*  普通的资源管理模块，在对象析构时调用注册进来的回调函数。                                                                    */
/************************************************************************/
typedef void* (*CallBack_Func_Type)(void*);
class CCommonResMng
{
public:

	/** 
	* @ func:  CCommonResMng
	* @ brief: 根据外部资源极其释放函数来生成一个资源管理对象
	* @author minglu2
	* Access: public 
	* @return: 
	* @param: void * pRes 资源
	* @param: CallBack_Func_Type pRelease_Source_Callback_Fun  指向用于释放资源的函数
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
	CallBack_Func_Type m_pRelease_Source_Callback_Fun;  //函数指针，指向用于释放资源的回调函数
	void* m_pRes; //指向外部的资源实体对象
};
//函数原型定义
