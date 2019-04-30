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
		fseek(pf, 0, SEEK_END); //�ļ�ָ���ƶ����ļ�ĩβ
		int length = ftell(pf);
		fclose(pf);
		pf = NULL;
		return length;
	}
}

//�ַ�������  
char *stringEncrypt(char *password, char *string)
{
	int passLength = strlen(password); //��ȡ���ܳ���  
	int stringLength = strlen(string); //��ȡ�ַ�������  

	if (stringLength%passLength == 0)//�ַ������������볤�ȵ�������  
	{
		int times = stringLength / passLength;
		for (int i = 0; i < times; i++)
		{
			for (int j = 0; j < passLength; j++)
			{
				string[i*passLength + j] ^= password[j];
				//������±���㻭ͼ���������  
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
		//���������ģ�Ҳ������������  
		for (int i = 0; i < lastLength; i++)
		{
			string[passLength*(stringLength / passLength) + i] ^= password[i];
			//����Ľ���Ҫ��ǰ���Ѿ�������ɵĺ�һ��λ�ÿ�ʼ��  
			//����˵stringLength = 10,passLength = 4;��ô�������ļ��ܾʹ��±�8��ʼ��  
			//(10 / 4)*4 = 8.  
		}
	}
	return string;
}
//�ַ��������㷨  
char *stringDecode(char *password, char *string)
{
	int passLength = strlen(password); //��ȡ���ܳ���  
	int stringLength = strlen(string); //��ȡ�ַ�������  

	if (stringLength % passLength == 0)//�ַ������������볤�ȵ�������  
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
		//���������ģ�Ҳ������������  
		for (int i = 0; i < lastLength; i++)
		{
			string[passLength*(stringLength / passLength) + i] ^= password[i];
			//����Ľ���Ҫ��ǰ���Ѿ�������ɵĺ�һ��λ�ÿ�ʼ��  
			//����˵stringLength = 10,passLength = 4;��ô�������ļ��ܾʹ��±�8��ʼ��  
			//(10 / 4)*4 = 8.  
		}
	}
	return string;
}

//����
void Encrypt(char *oldPath, char *newPath, char *password)
{
	FILE *pfr, *pfw;

	pfr = fopen(oldPath, "rb");//�Զ����ƶ�ȡ
	pfw = fopen(newPath, "wb");
	if (pfr == NULL || pfw == NULL)
	{
		fclose(pfr);
		fclose(pfw);
		return;
	}
	else
	{
		int length = getFileSize(oldPath);//��ȡԭ�ļ���С
		char *p = (char*)malloc(length * sizeof(char));//Ϊ���ļ�����ռ�

		fread(p, sizeof(char), length, pfr);//��ȡ�����Ƶ��ļ�
		stringEncrypt(password, p);
		fwrite(p, sizeof(char), length, pfw);//д������Ƶ��ļ�

		//�ļ��������֮��ر��ļ�
		fclose(pfr);
		fclose(pfw);
		pfr = NULL;
		pfw = NULL;
		VOS_FREE(p);
	}
}
//����
void decode(char *oldPath, char *newPath, char *password)
{
	//! ���ܺ�ɾ��ԭ�ļ�
	FILE *pfr, *pfw;

	pfr = fopen(oldPath, "rb");//�Զ����ƶ�ȡ
	pfw = fopen(newPath, "wb");
	if (pfr == NULL || pfw == NULL)
	{
		fclose(pfr);
		fclose(pfw);
		return;
	}
	else
	{
		int length = getFileSize(oldPath);//��ȡԭ�ļ���С
		char *p = (char*)malloc(length * sizeof(char));//Ϊ���ļ�����ռ�
		fread(p, sizeof(char), length, pfr);//��ȡ�����Ƶ��ļ�
		stringDecode(password, p);
		fwrite(p, sizeof(char), length, pfw);//д������Ƶ��ļ�

		//�ļ��������֮��ر��ļ�
		int aaa = fclose(pfr);
		aaa = fclose(pfw);
		pfr = NULL;
		pfw = NULL;
		VOS_FREE(p);
		remove(oldPath);
	}
}