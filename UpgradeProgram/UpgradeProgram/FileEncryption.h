#pragma once



//加密
void Encrypt(char *oldPath, char *newPath, char *password);
//解密
void decode(char *oldPath, char *newPath, char *password);
//获取文件大小
int getFileSize(char *path);


//字符串加密  
char *stringEncrypt(char *password, char *string);
//字符串解密算法  
char *stringDecode(char *password, char *string);
