#include "TimerEvent.h"
pthread_t pid_set_time_bysecond;
pthread_t pid_finger_get_bysecond;
pthread_t pid_Finger_bysecond;
int fd_usart = 0;
extern int TryingToDeleteUserID;
extern int TryingToDKUserID;
extern char TryingToDKUserUserName[10];
extern char TryingToAddUserName[10];
extern char TryingToAddUserPassword[10];
extern int PageInt;
extern int count, QueryUserID[5];
extern int FingerState;
void ThreadsInit();
char isFirstRun = 0;
void system_init() {
    char isConnected = 0;//数据库是否连接

    // 判断是否执行导入表的函数
    if (access(DBFileName, F_OK) == -1) // DB文件不存在
        isFirstRun = 1;
    isConnected = db_connect(DBFileName);
    if (isFirstRun) {
        CreateTablesOnce();
    }
  ThreadsInit();
}

struct tm *GetTime() {
    time_t now = time(NULL);
    struct tm *timeinfo;
    timeinfo = localtime(&now);
    return timeinfo;
}

/**
 *获取当前时间，提供给显示
 */
char *GetTimeStrNow() {
    struct tm *tm = GetTime();
    char static array[25]; // 字符串缓冲区
    sprintf(array, "%4.4d-%2.2d-%2.2d %2.2d:%2.2d:%2.2d", tm->tm_year+1900, tm->tm_mon+1,
            tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
    return array;
}

void *SetTimeBySecnods(void *a) {
  while (1) {
    struct tm *tm = GetTime();
    char static array[25]; // 字符串缓冲区
    sprintf(array, "%2.2d:%2.2d:%2.2d", tm->tm_hour, tm->tm_min, tm->tm_sec);
    lv_label_set_text(ui_LabelTime, array); // 时间 HH：MM：SS
    sprintf(array, "%2.2d", tm->tm_mon + 1);
    lv_label_set_text(ui_LabelMonth, array); // mm
    sprintf(array, "%2.2d", tm->tm_mday);
    lv_label_set_text(ui_LabelDay, array); // dd
    sprintf(array, "%2d", tm->tm_year + 1900);
    lv_label_set_text(ui_LabelYear, array); // yyyy
    switch (tm->tm_wday) {
      case 0:
        sprintf(array, "SUN");
        break;
      case 1:
        sprintf(array, "MON");
        break;
      case 2:
        sprintf(array, "TUE");
        break;
      case 3:
        sprintf(array, "WED");
        break;
      case 4:
        sprintf(array, "THU");
        break;
      case 5:
        sprintf(array, "FRI");
        break;
      case 6:
        sprintf(array, "SAT");
        break;
    }
    lv_label_set_text(ui_LabelWeek, array); // www
    sleep(1);
  }
}

/**
 *创建上述所有线程
 */
void *FingerThread(void *a) {
  SearchResult sr;
  uint16_t count_finger;
  int FingerMatch = -1;
  int UID=0;
  int rs;
  FingerState = FingerReadyRead;
  printf("FingerThread\n");
  fd_usart = USART_Init("/dev/ttySAC1");
  AS608_HandShake(fd_usart);
  if(isFirstRun) {
    Empty_FR(fd_usart);
    printf("Finger All Delete\n");
  }
  char Text[20];
  sleep(1);
  while (1) {
    while (!GetImage(fd_usart)) {
      usleep(100000);
    }

    printf("Fingerdown,State:%d\n", FingerState);
    switch (FingerState) {
      case FingerReadyRead:
        printf("FingerReadyReadOK\n");
        GenChar(fd_usart, 0x01);
        printf("Genchar\n");
        if (HighSpeedSearch(fd_usart, 0x01, 0, 299, &sr)) {
          printf("SearchTrue\n");
          FingerMatch=sr.pageID;
          printf("FingerModuleReadID:%d\n", FingerMatch);
          if(FingerprintsCheck(FingerMatch, &TryingToDKUserID,TryingToDKUserUserName)==1) {
              _ui_screen_change(&ui_Screen6, LV_SCR_LOAD_ANIM_FADE_ON, 500, 0,&ui_Screen6_screen_init);
              printf("OKOKOKOKOKOK\n");
              printf("NAME:%sEND;PWD:%sEND",TryingToAddUserName,TryingToAddUserPassword,TryingToAddUserPassword);
              sprintf(Text,"(UID = %d)%s",TryingToDKUserID,TryingToDKUserUserName);
              lv_label_set_text(ui_PassportNameLabel, Text);
              lv_label_set_text(ui_PassportTimeLabel, GetTimeStrNow());
              FingerState = FingerForbidden;
          }else{
            printf("NONONONONONONOFINGER\n");
          }

        }
        break;
      case FingerReadyAddA:
          printf("FingerReadyAddAOK\n");
          GenChar(fd_usart, 0x01);
          lv_img_set_src(ui_AddUserInformation, &ui_img_764847633); // 1
          FingerState = FingerReadyAddB;
          break;
      case FingerReadyAddB:
          printf("FingerReadyAddBOK\n");
          GenChar(fd_usart, 0x02);

          rs=Match(fd_usart);
          if(!rs) {
            _ui_screen_change(&ui_Screen7, LV_SCR_LOAD_ANIM_FADE_ON, 500, 0,&ui_Screen7_screen_init);
            break;
          }

          rs=HighSpeedSearch(fd_usart,0x01,0,300,&sr);
          printf("HS:%d\n",rs);
          if(rs) {
            _ui_screen_change(&ui_Screen7, LV_SCR_LOAD_ANIM_FADE_ON, 500, 0,
                  &ui_Screen7_screen_init);
            break;
          }

          rs=RegModel(fd_usart);
          printf("RegModel:%d\n",rs);
          if(!rs) {
            _ui_screen_change(&ui_Screen7, LV_SCR_LOAD_ANIM_FADE_ON, 500, 0,&ui_Screen7_screen_init);
          }

          AS608_Read_Library_Count(fd_usart, &count_finger);

          rs=StoreModel(fd_usart, CharBuffer1, count_finger);
          printf("SM:%d\n",rs);
          if (!rs) {
            _ui_screen_change(&ui_Screen7, LV_SCR_LOAD_ANIM_FADE_ON, 500, 0,&ui_Screen7_screen_init);
            break;
          }
          printf("CName:%s",TryingToAddUserName);
          printf("CPwd:%s",TryingToAddUserPassword);
          printf("FingerID:%d\n",sr.pageID);
          rs=CreateUser(TryingToAddUserName,TryingToAddUserPassword,&UID,count_finger);
          printf("Create:%d\n",rs);
          if(!rs) {
            _ui_screen_change(&ui_Screen7, LV_SCR_LOAD_ANIM_FADE_ON, 500, 0,&ui_Screen7_screen_init);
            break;
          }
      lv_img_set_src(ui_AddUserInformation, &ui_img_264905541);
      _ui_screen_change(&ui_Screen1, LV_SCR_LOAD_ANIM_FADE_ON, 500, 2000,&ui_Screen1_screen_init);
      FingerState = FingerReadyRead;
        break;
      default:
        break;
    }
  }
}

void ThreadsInit() {
  pthread_create(&pid_set_time_bysecond, NULL, SetTimeBySecnods, NULL);
  pthread_create(&pid_Finger_bysecond,NULL, FingerThread,NULL);
}
