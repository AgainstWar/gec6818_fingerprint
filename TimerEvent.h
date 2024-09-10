/* TimerEent.h 这个文件用于时间相关性函数：pthread线程函数，每秒执行函数，延时函数，延时动画函数*/
#ifndef TIMEREVENT_H
#define TIMEREVENT_H

#include "pthread.h"
#include "time.h"
#include "stdio.h"
#include "ui.h"
#include "unistd.h"
#include "ui_helpers.h"
#include "stdio.h"
#include "sql.h"
#include "Finger.h"


#define DEBUG 1
#define DBFileName "a.db"
void system_init();
char *GetTimeStrNow();
#endif //TIMEREVENT_H
