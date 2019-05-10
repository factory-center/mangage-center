#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <stdlib.h>
#include <iostream>
using namespace std;

// 变换法则
int genFun(int size, int key, int i)
{
	return size + key + i - size / key;
}
// 获得文件大小
int getFileSize(char str[])
{
	FILE *fp = fopen(str, "rb");
	if (fp == NULL)
	{
		return -1;
	}
	fseek(fp, 0, SEEK_END);
	int size = ftell(fp);
	fclose(fp);
	return size;
}
// 文件加密
bool encrypt(char toBeEncFileName[], int key)
{
	string outFileBuf = toBeEncFileName;
	outFileBuf = outFileBuf + "E";

	FILE *fp1 = fopen(toBeEncFileName, "rb");
	FILE *fp2 = fopen(outFileBuf.c_str(), "wb");
	if (fp1 == NULL || fp2 == NULL) 
	{
		return false;
	}
	int i = 0;
	int s = 0;
	int t = 0;
	int size = getFileSize(toBeEncFileName);
	for (i = 0; i < size; i++)
	{
		s = getc(fp1);

		t = genFun(size, key, i) ^ s;
		// 加密  
		putc(t, fp2);
	}
	fclose(fp1);
	fclose(fp2);
	return true;
}
// 文件解密
bool decrypt(char toBeDecFileName[], int key)
{
	string OutFileBuf;
	OutFileBuf = toBeDecFileName;
	OutFileBuf = OutFileBuf.substr(0, OutFileBuf.length() - 1);

	FILE *fp1 = fopen(toBeDecFileName, "rb");
	FILE *fp2 = fopen(OutFileBuf.c_str(), "wb");
	if (fp1 == NULL || fp2 == NULL)
	{
		return false;
	}
	int i = 0;
	int	s = 0;
	int	t = 0;
	int	size = getFileSize(toBeDecFileName);
	for (int i = 0; i < size; i++)
	{
		s = getc(fp1);
		t = genFun(size, key, i) ^ s;
		// 解密  
		putc(t, fp2);
	}
	fclose(fp1);
	fclose(fp2);
	return true;
}

