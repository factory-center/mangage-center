#pragma once



//����
void Encrypt(char *oldPath, char *newPath, char *password);
//����
void decode(char *oldPath, char *newPath, char *password);
//��ȡ�ļ���С
int getFileSize(char *path);


//�ַ�������  
char *stringEncrypt(char *password, char *string);
//�ַ��������㷨  
char *stringDecode(char *password, char *string);
