#ifndef FINGER_H
#define FINGER_H

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include "AS608_command.h"
// 初始化串口
int USART_Init(char *port);
// 配置串口
void init_tty(int fd);

// 与AS608握手
int AS608_HandShake(int fd);
// 读库指纹个数
bool AS608_Read_Library_Count(int fd, uint16_t *ValidN);
// 读参数
bool ReadSysPara(int fd);
// 刷指纹
void press_FR(int fd);
// 录指纹
void Add_FR(int fd);
// 判断有没有应答包
bool AS608_Has_Answer(char *recvinfo);
// 获取图像
bool GetImage(int fd);
// 生成特征
bool GenChar(int fd, uint8_t bufferID);
// 对比
bool Match(int fd);
// 搜索
bool Search(int fd, uint8_t BufferID, uint16_t StartPage, uint16_t PageNum, SearchResult *p);
// 生成模板
bool RegModel(int fd);
// 存储模板
bool StoreModel(int fd, uint8_t BufferID, uint16_t PageID);
// 高速搜索指纹库
bool HighSpeedSearch(int fd, uint8_t BufferID, uint16_t StartPage, uint16_t PageNum, SearchResult *p);
// 清空指纹库
bool Empty_FR(int fd);
// 删除指纹
bool DeleteChar(int fd, uint16_t PageID, uint16_t N);

#endif