#include <iostream>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>


void VOS_FREE(void *pData)
{
	if (NULL != pData)
	{
		free(pData);
		pData = NULL;
	}
	return;
}
int getFileSize(char *path)
{
	FILE *pf = fopen(path, "r");
	if (pf == NULL)
	{
		return-1;
	}
	else
	{
		fseek(pf, 0, SEEK_END); //文件指针移动到文件末尾
		int length = ftell(pf);
		fclose(pf);
		pf = NULL;
		return length;
	}
}

//字符串加密  
char *stringEncrypt(char *password, char *string)
{
	int passLength = strlen(password); //获取加密长度  
	int stringLength = strlen(string); //获取字符串长度  

	if (stringLength%passLength == 0)//字符串长度是密码长度的整数倍  
	{
		int times = stringLength / passLength;
		for (int i = 0; i < times; i++)
		{
			for (int j = 0; j < passLength; j++)
			{
				string[i*passLength + j] ^= password[j];
				//这里的下标计算画图很容易理解  
			}
		}
	}
	else
	{
		int times = stringLength / passLength;
		for (int i = 0; i < times; i++)
		{
			for (int j = 0; j < passLength; j++)
			{
				string[i*passLength + j] ^= password[j];
			}
		}
		int lastLength = stringLength % passLength;
		//不能整除的，也就是余数长度  
		for (int i = 0; i < lastLength; i++)
		{
			string[passLength*(stringLength / passLength) + i] ^= password[i];
			//这里的解密要从前边已经整除完成的后一个位置开始，  
			//假如说stringLength = 10,passLength = 4;那么除不尽的加密就从下标8开始，  
			//(10 / 4)*4 = 8.  
		}
	}
	return string;
}
//字符串解密算法  
char *stringDecode(char *password, char *string)
{
	int passLength = strlen(password); //获取加密长度  
	int stringLength = strlen(string); //获取字符串长度  

	if (stringLength % passLength == 0)//字符串长度是密码长度的整数倍  
	{
		int times = stringLength / passLength;
		for (int i = 0; i < times; i++)
		{
			for (int j = 0; j < passLength; j++)
			{
				string[i*passLength + j] ^= password[j];
			}
		}
	}
	else
	{
		int times = stringLength / passLength;
		for (int i = 0; i < times; i++)
		{
			for (int j = 0; j < passLength; j++)
			{
				string[i*passLength + j] ^= password[j];
			}
		}
		int lastLength = stringLength % passLength;
		//不能整除的，也就是余数长度  
		for (int i = 0; i < lastLength; i++)
		{
			string[passLength*(stringLength / passLength) + i] ^= password[i];
			//这里的解密要从前边已经整除完成的后一个位置开始，  
			//假如说stringLength = 10,passLength = 4;那么除不尽的加密就从下标8开始，  
			//(10 / 4)*4 = 8.  
		}
	}
	return string;
}

//加密
void Encrypt(char *oldPath, char *newPath, char *password)
{
	FILE *pfr, *pfw;

	pfr = fopen(oldPath, "rb");//以二进制读取
	pfw = fopen(newPath, "wb");
	if (pfr == NULL || pfw == NULL)
	{
		fclose(pfr);
		fclose(pfw);
		return;
	}
	else
	{
		int length = getFileSize(oldPath);//获取原文件大小
		char *p = (char*)malloc(length * sizeof(char));//为新文件申请空间

		fread(p, sizeof(char), length, pfr);//读取二进制到文件
		stringEncrypt(password, p);
		fwrite(p, sizeof(char), length, pfw);//写入二进制到文件

		//文件操作完成之后关闭文件
		fclose(pfr);
		fclose(pfw);
		pfr = NULL;
		pfw = NULL;
		VOS_FREE(p);
	}
}
//解密
void decode(char *oldPath, char *newPath, char *password)
{
	//! 解密后删除原文件
	FILE *pfr, *pfw;

	pfr = fopen(oldPath, "rb");//以二进制读取
	pfw = fopen(newPath, "wb");
	if (pfr == NULL || pfw == NULL)
	{
		fclose(pfr);
		fclose(pfw);
		return;
	}
	else
	{
		int length = getFileSize(oldPath);//获取原文件大小
		char *p = (char*)malloc(length * sizeof(char));//为新文件申请空间
		fread(p, sizeof(char), length, pfr);//读取二进制到文件
		stringDecode(password, p);
		fwrite(p, sizeof(char), length, pfw);//写入二进制到文件

		//文件操作完成之后关闭文件
		int aaa = fclose(pfr);
		aaa = fclose(pfw);
		pfr = NULL;
		pfw = NULL;
		VOS_FREE(p);
		remove(oldPath);
	}
}