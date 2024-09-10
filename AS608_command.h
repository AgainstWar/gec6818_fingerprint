#ifndef AS608_COMMAND_H
#define AS608_COMMAND_H
#include <stdint.h>

/*
AS608操作命令包格式
包头（2bytes）+芯片地址（4bytes）+包标识（1byte）+包长度（2byte）+指令（1byte）+参数1……+参数n+校验和（2byte）
包头：0xEF01
芯片地址：默认0xFFFFFFFF（上位机）
包标识：01
包长度 = 指令字节数+参数字节数+校验字节数
校验和：从包标识至校验和之间所有字节之和。 
*/
#define CharBuffer1 0x01
#define CharBuffer2 0x02

typedef struct  
{
	uint16_t pageID;//指纹ID
	uint16_t mathscore;//匹配得分
}SearchResult;

typedef struct
{
	uint16_t PS_max;//指纹最大容量
	uint8_t  PS_level;//安全等级
	uint32_t PS_addr;
	uint8_t  PS_size;//通讯数据包大小
	uint8_t  PS_N;//波特率基数N
}SysPara;

static uint8_t Command_Hand[2] = {0xEF,0X01};//包头
//包标识
static uint8_t package_ID_Command = 0x01;   //命令包
static uint8_t package_ID_Data = 0x02;      //数据包
static uint8_t package_ID_End = 0x08;       //结束包

//指令集
static uint8_t PS_GetImage = 0x01;          //握手，从传感器上读入图像存于图像缓冲区
static uint8_t PS_GenChar = 0x02;           //根据原始图像生成指纹特征存于CharBuffer1或CharBuffer2 
static uint8_t PS_Match = 0x03;             //精确比对CharBuffer1与CharBuffer2中的特征文件 
static uint8_t PS_Search = 0x04;            //以CharBuffer1或CharBuffer2中的特征文件搜索整个或部分指纹库
static uint8_t PS_RegModel = 0x05;          //将CharBuffer1 与 CharBuffer2 中的特征文件合并生成模板存于CharBuffer2 
static uint8_t PS_StoreChar = 0x06;         //将特征缓冲区中的文件储存到flash指纹库中
static uint8_t PS_LoadChar = 0x07;          //从flash指纹库中读取一个模板到特征缓冲区
static uint8_t PS_UpChar = 0x08;            //将特征缓冲区中的文件上传给上位机
static uint8_t PS_DownChar = 0x09;          //从上位机下载一个特征文件到特征缓冲区
static uint8_t PS_UpImage = 0x0A;           //上传原始图像
static uint8_t PS_DownImage = 0x0B;         //下载原始图像 
static uint8_t PS_DeletChar = 0x0C;         //删除flash指纹库中的一个特征文件
static uint8_t PS_Empty = 0x0D;             //清空flash指纹库
static uint8_t PS_WriteReg = 0x0E;          //写SOC系统寄存器 
static uint8_t PS_ReadSysPara = 0x0F;       //读系统基本参数
static uint8_t PS_Enroll = 0x10;            //注册模板
static uint8_t PS_Identify = 0x11;          //验证指纹
static uint8_t PS_SetPwd = 0x12;            //设置设备握手口令
static uint8_t PS_VfyPwd = 0x13;            //验证设备握手口令
static uint8_t PS_GetRandomCode = 0x14;     //采样随机数
static uint8_t PS_SetChipAddr = 0x15;       //设置芯片地址
static uint8_t PS_ReadINFpage = 0x16;       //读取FLASH Information Page 内容
static uint8_t PS_Port_Control = 0x17;      //通讯端口（UART/USB）开关控制
static uint8_t PS_WriteNotepad = 0x18;      //写记事本 
static uint8_t PS_ReadNotepad = 0x19;       //读记事本
static uint8_t PS_BurnCode = 0x1A;          //烧写片内FLASH
static uint8_t PS_HighSpeedSearch = 0x1B;   //高速搜索FLASH 
static uint8_t PS_GenBinImage = 0x1C;       //生成二值化指纹图像 
static uint8_t PS_ValidTempleteNum = 0x1D;  //读有效模板个数
static uint8_t PS_UserGPIOCommand = 0x1E;   //用户GPIO控制命令 （PS1802-3 及以后版本适用）
static uint8_t PS_ReadIndexTable = 0x1F;    //读索引表 

//确认码
static uint8_t CfC_Success = 0x00;                  //成功，指令执行完毕
static uint8_t CfC_packet_reception = 0x01;         //表示数据包接收错误
static uint8_t CfC_no_finger = 0x02;                //表示传感器上没有手指 
static uint8_t CfC_entry_image = 0x03;              //表示录入指纹图像失败
static uint8_t CfC_image_light = 0x04;              //表示指纹图像太干、太淡而生不成特征；
static uint8_t CfC_image_blurred = 0x05;            //表示指纹图像太湿、太糊而生不成特征；
static uint8_t CfC_image_messy = 0x06;              //表示指纹图像太乱而生不成特征；
static uint8_t CfC_feature_points = 0x07;           //表示指纹图像正常，但特征点太少（或面积太小）而生不成特征；
static uint8_t CfC_fingerprint_mismatches = 0x08;   //表示指纹不匹配；
static uint8_t CfC_Search_fingerprints = 0x09;      //表示没搜索到指纹；
static uint8_t CfC_feature_pooling = 0x0A;          //表示特征合并失败；
static uint8_t CfC_serial  = 0x0B;                  //表示访问指纹库时地址序号超出指纹库范围；
static uint8_t CfC_Read_template = 0x0C;            //表示从指纹库读模板出错或无效； 
static uint8_t CfC_upload_characteristics = 0x0D;   //表示上传特征失败； 
static uint8_t CfC_subsequent_packet = 0x0E;        //表示模块不能接受后续数据包
static uint8_t CfC_upload_images = 0x0F;            //表示上传图像失败；
static uint8_t CfC_delete_template = 0x10;          //表示删除模板失败；  
static uint8_t CfC_Empty_library = 0x11;            //表示清空指纹库失败；
static uint8_t CfC_password = 0x13;                 //表示口令不正确；
static uint8_t CfC_original_graph = 0x15;           //表示缓冲区内没有有效原始图而生不成图像；
static uint8_t CfC_FLASH = 0x18;                    //表示读写FLASH 出错
static uint8_t CfC_undefined = 0x19;                //未定义错误；  
static uint8_t CfC_Registe_number = 0x1A;           //无效寄存器号；
static uint8_t CfC_register_contents = 0x1B;        //寄存器设定内容错误
static uint8_t CfC_page_number = 0x1C;              //记事本页码指定错误；
static uint8_t CfC_PORT = 0x1D;                     //端口操作失败；
static uint8_t CfC_automatic_registration = 0x1E;   //自动注册（enroll）失败
static uint8_t CfC_full = 0x1F;                     //指纹库已满
static uint8_t CfC_address = 0x20;                     //地址错误
#endif