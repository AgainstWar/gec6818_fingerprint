#include "Finger.h"

uint32_t AS608Addr = 0xFFFFFFFF; // 默认
SysPara AS608Para;               // 指纹模块参数
void Send_cmd(int fd, uint8_t flag, uint8_t cmd);
/**
 * @brief 初始化串口
 *
 * @param port 串口号
 * @return fd 文件描述符
 */
int USART_Init(char *port)
{
    int fd = open(port, O_RDWR | O_NOCTTY); // O_NOCTTY：该文件是一个终端设备
    if (fd == -1)
    {
        printf("open %s failed: %s\n", port, strerror(errno));
        exit(0); // 退出进程
    }
    init_tty(fd); // 配置串口
    // 将串口设置为非阻塞状态，避免第一次运行卡住的情况
    // long state = fcntl(fd, F_GETFL);	//获取文件状态
    // state |= O_NONBLOCK;	//非阻塞状态
    // fcntl(fd, F_SETFL, state);//设置为为非阻塞状态

    return fd;
}
/**
 * @brief 配置串口
 *
 * @param fd 文件描述符
 */
void init_tty(int fd)
{
    // 声明设置串口的结构体
    struct termios config;
    bzero(&config, sizeof(config));

    /*
    设置为原始模式
    无奇偶校验
    数据位为8位
    非规范模式：输入数据不进行任何编辑和缓冲处理，直接传递给程序
    停止位：1位
    */
    cfmakeraw(&config);

    // 设置波特率9600
    cfsetispeed(&config, B57600);
    cfsetospeed(&config, B57600);

    // CLOCAL和CREAD分别用于本地连接和接受使能
    // 首先要通过位掩码的方式激活这两个选项。
    config.c_cflag |= CLOCAL | CREAD;
    config.c_cflag &= ~CSTOPB; // 1位停止位
    config.c_cflag &= ~CSIZE;  // 清除数据位设置
    config.c_cflag |= CS8;     // 设置数据位为8位
    config.c_cflag &= ~PARENB; // 无奇偶校验

    // 可设置接收字符和等待时间，无特殊要求可以将其设置为0
    config.c_cc[VTIME] = 0;
    config.c_cc[VMIN] = 1;

    // 用于清空输入/输出缓冲区
    tcflush(fd, TCIFLUSH);
    tcflush(fd, TCOFLUSH);

    // 完成配置后，可以使用以下函数激活串口设置
    if (tcsetattr(fd, TCSANOW, &config) != 0)
    {
        perror("设置串口失败");
        exit(0);
    }
}

/**
 * @brief 与AS608模块握手
 *
 * @param fd 文件描述符
 * @return 0 握手成功
 * @return -1 握手失败
 */
int AS608_HandShake(int fd)
{
    tcflush(fd, TCIFLUSH); // 清空输入缓冲区

    Send_cmd(fd, package_ID_Command, PS_GetImage);
    // usleep(200000);

    // 接收响应包
    char recvinfo[128] = {0};
    read(fd, recvinfo, 128);
    // 判断是不是模块返回的应答包
    if (recvinfo[0] = 0xEF && recvinfo[1] == 0x01 && recvinfo[6] == 0x07)
    {
        AS608Addr = recvinfo[2] << 24 | recvinfo[3] << 16 | recvinfo[4] << 8 | recvinfo[5];
        printf("AS608Addr = %08X\n", AS608Addr);
        return 0;
    }
    printf("NO%s\n", recvinfo);
    return -1;
}

/**
 * @brief 读库指纹个数
 *
 * @param fd 文件描述符
 * @param ValidN 有效模板个数
 * @return true 读取成功
 * @return false 读取失败
 */
bool AS608_Read_Library_Count(int fd, uint16_t *ValidN)
{
    tcflush(fd, TCIFLUSH);
    tcflush(fd, TCOFLUSH);
    int ensure = 0; // 有效模板个数
    Send_cmd(fd, package_ID_Command, PS_ValidTempleteNum);

    char recvinfo[128] = {0};
    read(fd, recvinfo, 128);
    if (AS608_Has_Answer(recvinfo)) // 判断是不是模块返回的应答包
    {
        if (recvinfo[9] == CfC_Success) // 读库指纹个数成功
        {
            *ValidN = (recvinfo[10] << 8) | recvinfo[11];
            printf("读库指纹个数成功，有效模板个数：%d\n", *ValidN);
            return true;
        }
        else
            return false;
    }
    else
        return false;
}

/**
 * @brief 读参数
 *
 * @param fd 文件描述符
 * @return true 读参数成功
 * @return false 读参数失败
 */
bool ReadSysPara(int fd)
{
    tcflush(fd, TCIFLUSH);
    tcflush(fd, TCOFLUSH);
    Send_cmd(fd, package_ID_Command, PS_ReadSysPara);
    char recvinfo[128] = {0};
    read(fd, recvinfo, 128);
    if (AS608_Has_Answer(recvinfo))
    {
        AS608Para.PS_max = (recvinfo[14] << 8) | recvinfo[15];                                                // 最大指纹库容量
        AS608Para.PS_level = recvinfo[17];                                                                    // 指纹库安全等级
        AS608Para.PS_addr = (recvinfo[18] << 24) | (recvinfo[19] << 16) | (recvinfo[20] << 8) | recvinfo[21]; // 指纹库地址
        AS608Para.PS_size = recvinfo[23];
        AS608Para.PS_N = recvinfo[25];

        printf("最大指纹库容量：%d\n", AS608Para.PS_max);
        printf("对比等级：%d\n", AS608Para.PS_level);
        printf("地址：%08X\n", AS608Para.PS_addr);
        printf("波特率：%d\n", AS608Para.PS_N);
        return true;
    }
    else
        return false;
}

/**
 * @brief 刷指纹
 *
 * @param fd 文件描述符
 * @note 刷指纹，搜索指纹库
 */
void press_FR(int fd)
{
    SearchResult Search;
    printf("请按手指\n");
    while (1)
    {
        // 等待手指按下
        while (!GetImage(fd))
        {
            usleep(100000); // 休眠100毫秒，避免过于频繁的检测
        }

        printf("检测到手指按下\n");

        // 获取图像并生成特征
        if (GenChar(fd, CharBuffer1))
        {
            printf("特征生成成功\n");

            // 搜索指纹库
            if (HighSpeedSearch(fd, CharBuffer1, 0, AS608Para.PS_max, &Search))
            {
                printf("搜索到指纹，用户：%d，匹配度：%d\n", Search.pageID, Search.mathscore);
            }            
        }
        
        // 等待手指松开
        printf("请松开手指\n");
        while (GetImage(fd))
        {
            usleep(100000); // 休眠100毫秒，避免过于频繁的检测
        }

        printf("检测到手指松开\n");
        printf("请按手指\n");
    }
}

/**
 * @brief 添加指纹
 *
 * @param fd 文件描述符
 * @note 添加指纹到指纹库
 */
void Add_FR(int fd)
{
    tcflush(fd, TCIFLUSH);
    tcflush(fd, TCOFLUSH);
    uint8_t processnum = 0; // 录入过程号
    uint16_t ValidN = 0;    // 有效模板个数
    uint16_t ID = 0;        // 指纹ID
    while (1)
    {
        switch (processnum)
        {
        case 0: // 第一次录入指纹
        {
            printf("开始录入指纹\n");
            while (1)
            {
                if (GetImage(fd))
                {
                    if (GenChar(fd, CharBuffer1))
                    {
                        printf("特征生成成功\n");
                        processnum = 1;
                        break;
                    }
                }
            }
        }
        break;
        case 1: // 第二次录入指纹
        {
            printf("请再按一次指纹\n");
            while (1)
            {
                if (GetImage(fd))
                {
                    if (GenChar(fd, CharBuffer2))
                    {
                        printf("特征生成成功\n");
                        processnum = 2;
                        break;
                    }
                }
            }
        }
        break;
        case 2: // 对比两次指纹
        {
            printf("对比两次指纹\n");
            if (Match(fd))
            {
                printf("指纹匹配成功\n");
                processnum = 3;
            }
            else
            {
                printf("指纹匹配失败，请重新录入指纹\n");
                processnum = 0;
            }
        }
        break;
        case 3: // 生成指纹模板
        {
            printf("生成指纹模板\n");
            if (RegModel(fd))
            {
                printf("指纹模板生成成功\n");
                processnum = 4;
            }
            else
            {
                printf("指纹模板生成失败，请重新录入指纹\n");
                processnum = 0;
            }
        }
        break;
        case 4: // 存储模板
        {
            // 读出库中指纹个数
            if (AS608_Read_Library_Count(fd, &ValidN))
                ID = ValidN + 1;
            // 存储模板
            if (StoreModel(fd, CharBuffer1, ID))
            {
                printf("模板存储成功，指纹ID：%d\n", ID);
                AS608_Read_Library_Count(fd, &ValidN);
                printf("剩余指纹容量：%d\n", AS608Para.PS_max - ValidN);
                return;
            }
            else
            {
                printf("模板存储失败，请重新录入指纹\n");
                processnum = 0;
            }
        }
        break;
        }
    }
}

/**
 * @brief 发送包头
 *
 * @param fd 文件描述符
 */
void Send_Head(int fd)
{
    write(fd, Command_Hand, 2);
}
/**
 * @brief 发送地址
 *
 * @param fd 文件描述符
 */
void Send_Addr(int fd)
{
    char buf[4] = {AS608Addr >> 24 & 0xFF, AS608Addr >> 16 & 0xFF, AS608Addr >> 8 & 0xFF, AS608Addr & 0xFF};
    write(fd, buf, 4);
}
/**
 * @brief 发送包标识
 *
 * @param fd 文件描述符
 * @param flag 包标识
 */
void SendFlag(int fd, uint8_t flag)
{
    write(fd, &flag, 1);
}
/**
 * @brief 发送包长度
 *
 * @param fd 文件描述符
 * @param length 包长度
 */
void SendLength(int fd, uint16_t length)
{
    char buf[2] = {length >> 8 & 0xFF, length & 0xFF};
    write(fd, buf, 2);
}
/**
 * @brief 发送指令码
 *
 * @param fd 文件描述符
 * @param cmd 指令码
 */
void Sendcmd(int fd, uint8_t cmd)
{
    write(fd, &cmd, 1);
}
/**
 * @brief 发送校验和
 *
 * @param fd 文件描述符
 * @param check 校验和
 */
void SendCheck(int fd, uint16_t check)
{
    char buf[2] = {check >> 8 & 0xFF, check & 0xFF};
    write(fd, buf, 2);
}

/**
 * @brief 发送命令包
 *
 * @param fd 文件描述符
 * @param flag 包标识
 * @param cmd 指令码
 */
void Send_cmd(int fd, uint8_t flag, uint8_t cmd)
{
    // 发送包头
    Send_Head(fd);
    // 发送地址
    Send_Addr(fd);
    // 发送包标识-命令包
    SendFlag(fd, flag);
    uint16_t length = 3; // 包长度
    // 发送包长度
    SendLength(fd, length);
    // 发送指令码-读库指纹个数
    Sendcmd(fd, cmd);
    // 发送校验和
    SendCheck(fd, length + cmd + flag);
}

/**
 * @brief 模块应答包确认码信息解析
 *
 * @param ensure 确认码
 * @return p 确认码信息
 * @note 解析模块返回的确认码信息
 */
const char *EnsureMessage(uint8_t ensure)
{
    const char *p;
    switch (ensure)
    {
    case 0x00:
        p = "OK";
        break;
    case 0x01:
        p = "数据包接收错误";
        break;
    case 0x02:
        p = "传感器上没有手指";
        break;
    case 0x03:
        p = "录入指纹图像失败";
        break;
    case 0x04:
        p = "指纹图像太干、太淡而生不成特征";
        break;
    case 0x05:
        p = "指纹图像太湿、太糊而生不成特征";
        break;
    case 0x06:
        p = "指纹图像太乱而生不成特征";
        break;
    case 0x07:
        p = "指纹图像正常，但特征点太少（或面积太小）而生不成特征";
        break;
    case 0x08:
        p = "指纹不匹配";
        break;
    case 0x09:
        p = "没搜索到指纹";
        break;
    case 0x0a:
        p = "特征合并失败";
        break;
    case 0x0b:
        p = "访问指纹库时地址序号超出指纹库范围";
    case 0x0c:
        p = "从指纹库读模板出错或无效";
    case 0x0d:
        p = "上传特征失败";
    case 0x0e:
        p = "模块不能接受后续数据包";
    case 0x0f:
        p = "上传图像失败";
    case 0x10:
        p = "删除模板失败";
        break;
    case 0x11:
        p = "清空指纹库失败";
        break;
    case 0x13:
        p = "口令不正确";
        break;
    case 0x15:
        p = "缓冲区内没有有效原始图而生不成图像";
        break;
    case 0x18:
        p = "读写FLASH出错";
        break;
    case 0x19:
        p = "未定义错误";
        break;
    case 0x1a:
        p = "无效寄存器号";
        break;
    case 0x1b:
        p = "寄存器设定内容错误";
        break;
    case 0x1c:
        p = "记事本页码指定错误";
        break;
    case 0x1d:
        p = "端口操作失败";
        break;
    case 0x1e:
        p = "自动注册失败";
        break;
    case 0x1f:
        p = "指纹库已满";
        break;
    case 0x20:
        p = "地址错误";
        break;
    default:
        p = "模块返回确认码错误";
        break;
    }
    return p;
}

/**
 * @brief 判断有没有应答包
 *
 * @param recvinfo 接收到的信息
 * @return true 有应答包 false：无应答包
 * @note 判断是否为模块返回的应答包
 */
bool AS608_Has_Answer(char *recvinfo)
{
    char str[] = {0xEF, 0x01, AS608Addr >> 24, AS608Addr >> 16, AS608Addr >> 8, AS608Addr & 0xFF, 0x07, '\0'};
    if (strstr(recvinfo, str))
        return true;
    else
    {
        printf("%s\n", EnsureMessage(recvinfo[9]));
        return false;
    }
}

/**
 * @brief 录入图像
 *
 * @param fd 文件描述符
 * @return true 获取图像成功 false：获取图像失败
 * @note 探测手指，探测到后，录入指纹图像存于imagebuffer中
 */
bool GetImage(int fd)
{
    tcflush(fd, TCIFLUSH);
    tcflush(fd, TCOFLUSH);
    Send_cmd(fd, package_ID_Command, PS_GetImage);
    char recvinfo[128] = {0};
    read(fd, recvinfo, 128);
    if (AS608_Has_Answer(recvinfo))
    {
        if (recvinfo[9] == CfC_Success) // 获取图像成功
            return true;
        else
            return false;
    }
    else
        return false;
}
/**
 * @brief 生成特征
 *
 * @param fd：文件描述符
 * @param BufferID：Charbuffer1-->0x01，Charbuffer2-->0x02
 * @return true：生成成功
 * @return false：生成失败
 * @note 将imagebuffer中的图像转换为特征，存于Charbuffer1或Charbuffer2中
 */
bool GenChar(int fd, uint8_t BufferID)
{
    tcflush(fd, TCIFLUSH);
    tcflush(fd, TCOFLUSH);

    // 发送包头
    Send_Head(fd);
    // 发送地址
    Send_Addr(fd);
    // 发送包标识-命令包
    SendFlag(fd, package_ID_Command);
    uint16_t length = 4; // 包长度
    // 发送包长度
    SendLength(fd, length);
    // 发送指令码-读库指纹个数
    Sendcmd(fd, PS_GenChar);
    write(fd, &BufferID, 1); // 发送CharbufferID
    // 发送校验和
    SendCheck(fd, length + PS_GenChar + package_ID_Command + BufferID);

    char recvinfo[128] = {0};
    read(fd, recvinfo, 128);
    if (AS608_Has_Answer(recvinfo))
    {
        if (recvinfo[9] == CfC_Success) // 特征生成成功
            return true;
        else
            return false;
    }
    else
        return false;
}

/**
 * @brief 精确对比两枚指纹特征
 *
 * @param fd：文件描述符
 * @return true：匹配成功
 * @param false：匹配失败
 * @note 精确对比CharBuffer1和CharBuffer2中的特征文件
 */
bool Match(int fd)
{
    tcflush(fd, TCIFLUSH);
    tcflush(fd, TCOFLUSH);
    Send_cmd(fd, package_ID_Command, PS_Match);
    char recvinfo[128] = {0};
    read(fd, recvinfo, 128);
    if (AS608_Has_Answer(recvinfo))
    {
        if (recvinfo[9] == CfC_Success) // 特征匹配成功
            return true;
        else
            return false;
    }
    else
        return false;
}
/**
 * @brief 搜索指纹库
 *
 * @param fd 文件描述符
 * @param BufferID Charbuffer1-->0x01，Charbuffer2-->0x02
 * @param StartPage 搜索起始页码
 * @param PageNum 搜索页数
 * @return SearchResult 搜索结果
 * @note 以CharBuffer1或CharBuffer2中的特征文件搜索整个或部分指纹库，若搜索到，则返回页码
 */
bool Search(int fd, uint8_t BufferID, uint16_t StartPage, uint16_t PageNum, SearchResult *p)
{
    tcflush(fd, TCIFLUSH);
    tcflush(fd, TCOFLUSH);
    // 发送包头
    Send_Head(fd);
    // 发送地址
    Send_Addr(fd);
    // 发送包标识-命令包
    SendFlag(fd, package_ID_Command);
    uint16_t length = 0x08; // 包长度
    // 发送包长度
    SendLength(fd, length);
    // 发送指令码-读库指纹个数
    Sendcmd(fd, PS_Search);
    write(fd, &BufferID, 1); // 发送CharbufferID
    char buf[4] = {StartPage >> 8 & 0xFF, StartPage & 0xFF, PageNum >> 8 & 0xFF, PageNum & 0xFF};
    write(fd, buf, 4); // 发送起始页码和页数
    // 发送校验和
    SendCheck(fd, package_ID_Command + length + PS_Search + BufferID + buf[0] + buf[1] + buf[2] + buf[3]);
    char recvinfo[128] = {0};
    read(fd, recvinfo, 128);
    if (AS608_Has_Answer(recvinfo))
    {
        if (recvinfo[9] == CfC_Success) // 搜索成功
        {
            p->pageID = (recvinfo[10] << 8) | recvinfo[11];  // 页码
            p->mathscore = recvinfo[12] << 8 | recvinfo[13]; // 匹配度
            return true;
        }
        else
            return false;
    }
    else
        return false;
}
/**
 * @brief 生成指纹模板(合并特征)
 *
 * @param fd 文件描述符
 * @return true 合并成功
 * @return false 合并失败
 * @note 将CharBuffer1或CharBuffer2中的特征文件合并生成指纹模板，存于Charbuffer1或Charbuffer2中
 */
bool RegModel(int fd)
{
    tcflush(fd, TCIFLUSH);
    tcflush(fd, TCOFLUSH);
    Send_cmd(fd, package_ID_Command, PS_RegModel);
    char recvinfo[128] = {0};
    read(fd, recvinfo, 128);
    if (AS608_Has_Answer(recvinfo))
    {
        if (recvinfo[9] == CfC_Success) // 特征合并成功
            return true;
        else
            return false;
    }
    else
        return false;
}

/**
 * @brief 存储模板
 *
 * @param fd 文件描述符
 * @param BufferID Charbuffer1-->0x01，Charbuffer2-->0x02
 * @param PageID 存储页码
 * @return true 存储成功
 * @return false 存储失败
 * @note 将CharBuffer1或CharBuffer2中的特征文件存储到PageID号flash数据库中
 */
bool StoreModel(int fd, uint8_t BufferID, uint16_t PageID)
{
    tcflush(fd, TCIFLUSH);
    tcflush(fd, TCOFLUSH);
    // 发送包头
    Send_Head(fd);
    // 发送地址
    Send_Addr(fd);
    // 发送包标识-命令包
    SendFlag(fd, package_ID_Command);
    uint16_t length = 0x06; // 包长度
    // 发送包长度
    SendLength(fd, length);
    // 发送指令码-存储模板
    Sendcmd(fd, PS_StoreChar);
    write(fd, &BufferID, 1); // 发送CharbufferID
    uint8_t buf[2] = {PageID >> 8, PageID & 0xFF};
    write(fd, buf, 2); // 发送起始页码
    // 发送校验和
    SendCheck(fd, package_ID_Command + length + PS_StoreChar + BufferID + buf[0] + buf[1]);

    char recvinfo[128] = {0};
    read(fd, recvinfo, 128);
    if (AS608_Has_Answer(recvinfo))
    {
        if (recvinfo[9] == CfC_Success) // 模板存储成功
            return true;
        else
        {
            printf("%s\n", EnsureMessage(recvinfo[9]));
            return false;
        }
    }
    else
        return false;
}

/**
 * @brief 高速搜索指纹库
 *
 * @param fd 文件描述符
 * @param BufferID Charbuffer1-->0x01，Charbuffer2-->0x02
 * @param StartPage 搜索起始页码
 * @param PageNum 搜索页数
 * @return SearchResult 搜索结果
 * @note 以CharBuffer1或CharBuffer2中的特征文件搜索整个或部分指纹库，若搜索到，则返回页码参数
 */
bool HighSpeedSearch(int fd, uint8_t BufferID, uint16_t StartPage, uint16_t PageNum, SearchResult *p)
{
    tcflush(fd, TCIFLUSH);
    tcflush(fd, TCOFLUSH);
    // 发送包头
    Send_Head(fd);
    // 发送地址
    Send_Addr(fd);
    // 发送包标识-命令包
    SendFlag(fd, package_ID_Command);
    uint16_t length = 0x08; // 包长度
    // 发送包长度
    SendLength(fd, length);
    // 发送指令码-存储模板
    Sendcmd(fd, PS_HighSpeedSearch);
    write(fd, &BufferID, 1); // 发送CharbufferID
    uint8_t buf[4] = {StartPage >> 8, StartPage & 0xFF, PageNum >> 8, PageNum & 0xFF};
    write(fd, buf, 4); // 发送起始页码和页数
    // 发送校验和
    SendCheck(fd, package_ID_Command + length + PS_HighSpeedSearch + BufferID + buf[0] + buf[1] + buf[2] + buf[3]);

    char recvinfo[128] = {0};
    read(fd, recvinfo, 128);
    if (AS608_Has_Answer(recvinfo))
    {
        if (recvinfo[9] == CfC_Success) // 搜索成功
        {
            p->pageID = (recvinfo[10] << 8) | recvinfo[11];  // 页码
            p->mathscore = recvinfo[12] << 8 | recvinfo[13]; // 匹配度
            return true;
        }
        else
        {
            printf("%s\n", EnsureMessage(recvinfo[9]));
            return false;
        }
    }
    else
        return false;
}

/**
 * @brief 删除模板
 *
 * @param fd 文件描述符
 * @param PageID 模板页码
 * @param N 偏移量
 * @return true 删除成功
 * @return false 删除失败
 * @note 删除PageID号flash数据库中的指纹模板
 */
bool DeleteChar(int fd, uint16_t PageID, uint16_t N)
{
    tcflush(fd, TCIFLUSH);
    tcflush(fd, TCOFLUSH);
    // 发送包头
    Send_Head(fd);
    // 发送地址
    Send_Addr(fd);
    // 发送包标识-命令包
    SendFlag(fd, package_ID_Command);
    uint16_t length = 0x08; // 包长度
    // 发送包长度
    SendLength(fd, length);
    // 发送指令码-存储模板
    Sendcmd(fd, PS_DeletChar);
    uint8_t buf[4] = {PageID >> 8, PageID & 0xFF, N >> 8, N & 0xFF};
    write(fd, buf, 4); // 发送起始页码和页数
    // 发送校验和
    SendCheck(fd, package_ID_Command + length + PS_DeletChar + buf[0] + buf[1] + buf[2] + buf[3]);

    char recvinfo[128] = {0};
    read(fd, recvinfo, 128);
    if (AS608_Has_Answer(recvinfo))
    {
        if (recvinfo[9] == CfC_Success) // 删除成功
            return true;
        else
        {
            printf("%s\n", EnsureMessage(recvinfo[9]));
            return false;
        }
    }
    else
        return false;
}
/*
清空指纹库
删除指纹库中所有指纹
*/
bool Empty_FR(int fd)
{
    tcflush(fd, TCIFLUSH);
    tcflush(fd, TCOFLUSH);
    Send_cmd(fd, package_ID_Command, PS_Empty);
    char recvinfo[128] = {0};
    read(fd, recvinfo, 128);
    if (AS608_Has_Answer(recvinfo))
    {
        if (recvinfo[9] == CfC_Success) // 清空指纹库成功
            return true;
        else
            return false;
    }
    else
        return false;
}
enum {
    State_Forbidden,//禁止指纹输入
    State_ReadyForInput,//当前在主界面，准备打卡指纹输入
    State_ADDing,//在添加的界面，准备添加指纹输入
};
int StateFinger=State_Forbidden;
