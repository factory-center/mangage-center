#ifndef __Downloader_LibCurl_H__
#define __Downloader_LibCurl_H__

#pragma once
#include "curl\\curl.h"
#include <atlstr.h>

#define MAXWORK 200
typedef struct DownloadInfo
{
	char url[512];
	char filePath[256];
}DLIO;

typedef struct CurDownloadInfor
{
	char url[512];     //url
	char fileName[256];   //�ļ�����
	long preLocalLen;     //���������صĳ��ȣ���С��
	double totalFileLen;   //�ļ��ܳ��ȣ���С��
	double CurDownloadLen;   //ÿ�����ص��ļ�����(��С)
}CURDI;

class CDownloader
{
public:
	CDownloader(void);
	~CDownloader(void);
	int StartDownloadThread();
	double GetTotalFileLenth(const char* url); //��ȡ��Ҫ���ص��ļ�����
	long GetLocalFileLenth(const char* fileName); //��ȡ�����ļ�����
	void GetFileNameFormUrl(char* fileName, const char* url); //��URL�л�ȡ�ļ���
	void AddDownloadWork(DLIO downloadWork);
	int SetConnectTimeOut(DWORD nConnectTimeOut); //�������ӵĳ�ʱʱ��
	int GetCurrentDownloadInfo(CURDI* lpCurDownloadInfor);
	BOOL CreateMultiDir(const char* pathName); //�Ƿ��ڱ��ش���Ŀ¼��û�оʹ���
	BOOL IsDownloadBegin();
	BOOL IsDownloadEnd();
protected:
	static DWORD WINAPI SingleDownloadProc(LPVOID lpParameter);       //�̺߳���
	static size_t WriteFunc(char *str, size_t size, size_t nmemb, void *stream);     //д�����ݣ��ص�������
	static size_t ProgressFunc(double* fileLen, double t, double d, double ultotal, double ulnow);   //���ؽ���
private:
	char m_filePath[512];
	char m_downloadUrl[256];
	int m_downloadCourse;   //-1 ��δ���� 0�������� 1�������
	long m_curLocalFileLenth; //��Ϊ���ص�ʱ���Ѿ������˱����ļ��Ĵ�С�������öϵ㣬���Զ���ÿ���ļ���������ֻ�ᱻ����һ�Σ���������ǰ�ı��ش�С��
	long m_nConnectTimeOut;      //���ӵĳ�ʱʱ��
	DLIO m_dowloadWork[MAXWORK];
	CURDI m_curDownloadInfo;
	int m_curIndex;
	CURL* m_pCurl;
};


#endif
