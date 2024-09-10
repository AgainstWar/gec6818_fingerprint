//
// Created by happy on 24-9-3.
//
#define DEBUG 1
#include "sql.h"

/**
 * @brief 将时间转换为字符串
 *
 * @param time
 * @param buffer
 */
void convertTime(int time, char *buffer)
{

    int minutes = time % 100;
    int hours = time / 100;
    // 四舍五入到最近的整点
    if (minutes >= 30)
    {
        hours += 1;
    }
    sprintf(buffer, "%02d:00:00", hours);
}
/**
 * @brief 将字符串转换为时间
 *
 * @param buffer
 * @param time
 */
void reconvertTime(char *buffer, int *time)
{
    int hours, minutes;
    sscanf(buffer, "%02d:%02d:00", &hours, &minutes);
    *time = hours * 100 + minutes;
}

/**
 * @brief 连接数据库，保存到全局指针db
 * @return 状态码 1成功 0失败
 * @param dbName 数据库名
 */
int db_connect(char *dbName)
{
    // rc 用于存储返回值
    int rc = sqlite3_open(dbName, &db);

    if (rc)
    {
        // printf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return 0; // 失败
    }
    else
    {
        // printf(stderr, "Opened database successfully\n");
        return 1; // 成功
    }
}

/**
 * @brief 初始化基本表（如果不存在）
 * 1. USERS 用户表
 * 2. Fingerprints 指纹表
 * 3. Attendance 考勤数据表
 * 4. WorkTime 工作时间表
 * @return void
 */
void CreateTablesOnce()
{

    // 创建USERS
    const char *sql_users = "CREATE TABLE IF NOT EXISTS USERS("
                            "UID INTEGER PRIMARY KEY,"
                            "NAME           VARCHAR(100)    NOT NULL,"
                            "PWD_HASH       VARCHAR(255),"
                            "CREATED_AT     DATETIME  DEFAULT CURRENT_TIMESTAMP"
                            ");";

    // 创建Fingerprints
    const char *sql_fingerprints = "CREATE TABLE IF NOT EXISTS Fingerprints("
                                   "FINGERPRINT_ID INTEGER PRIMARY KEY,"
                                   "USER_ID INTEGER NOT NULL,"
                                   "CREATED_AT     DATETIME  DEFAULT CURRENT_TIMESTAMP,"
                                   "FOREIGN KEY (USER_ID) REFERENCES USERS(UID) ON DELETE CASCADE"
                                   ");";

    // 创建Attendance
    const char *sql_attendance = "CREATE TABLE IF NOT EXISTS Attendance("
                                 "ATTENDANCE_ID INTEGER PRIMARY KEY,"
                                 "USER_ID INTEGER NOT NULL,"
                                 "CHECK_IN_TIME DATETIME NOT NULL,"
                                 "CHECK_OUT_TIME DATETIME,"
                                 "IS_LATE BOOLEAN DEFAULT FALSE,"
                                 "IS_EARLY BOOLEAN DEFAULT FALSE,"
                                 "IS_ABSENT BOOLEAN DEFAULT FALSE,"
                                 "WORKTIME_ID INTEGER NOT NULL,"
                                 "FOREIGN KEY (USER_ID) REFERENCES USERS(UID) ON DELETE CASCADE "
                                 "FOREIGN KEY (WORKTIME_ID) REFERENCES WorkTime(WORKTIME_ID) ON DELETE CASCADE"
                                 ");";

    // 创建 WorkTime
    const char *sql_worktime = "CREATE TABLE IF NOT EXISTS WorkTime("
                               "WORKTIME_ID INTEGER PRIMARY KEY,"
                               "CHECK_IN_TIME TIME NOT NULL,"
                               "CHECK_OUT_TIME TIME NOT NULL"
                               ");";

    char *zErrMsg = 0;
    int rc;
    // 执行SQL语句

    // USRES
    rc = sqlite3_exec(db, sql_users, 0, 0, &zErrMsg);
    if (rc != SQLITE_OK)
    {
        printf("SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    else
        printf("Table USERS created successfully\n");

    // Fingerprints
    rc = sqlite3_exec(db, sql_fingerprints, 0, 0, &zErrMsg);
    if (rc != SQLITE_OK)
    {
        printf("SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    else
        printf("Table Fingerprints created successfully\n");

    // WorkTime
    rc = sqlite3_exec(db, sql_worktime, 0, 0, &zErrMsg);
    if (rc != SQLITE_OK)
    {
        printf("SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    else
        printf("Table WorkTime created successfully\n");

    // Attendance
    rc = sqlite3_exec(db, sql_attendance, 0, 0, &zErrMsg);
    if (rc != SQLITE_OK)
    {
        printf("SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    else
        printf("Table Attendance created successfully\n");
    /*--------------------------------------*/
    // 添加管理员账户
    const char *sql_admin = "INSERT INTO USERS (UID, NAME, PWD_HASH) " // 默认密码为 123456
                            "VALUES (1, 'Administrator', '62674f9a0672ba6650be9b24a6d10a2b9a66e75217767b1c69c06f118744b12c');";
    rc = sqlite3_exec(db, sql_admin, 0, 0, &zErrMsg);
    if (rc != SQLITE_OK)
    {
        printf("SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    else
        printf("Admin account created successfully\n");

    /*---------------TRIGGER-----------------*/
    // 创建触发器
    const char *sql_trigger = "CREATE TRIGGER IF NOT EXISTS check_attendance "
                              "AFTER INSERT ON Attendance "
                              "FOR EACH ROW "
                              "BEGIN "
                              "    UPDATE Attendance "
                              "    SET IS_LATE = ( "
                              "        SELECT CASE "
                              "            WHEN NEW.CHECK_IN_TIME > WorkTime.CHECK_IN_TIME THEN 1 "
                              "            ELSE 0 "
                              "        END "
                              "        FROM WorkTime "
                              "        WHERE WorkTime.WORKTIME_ID = NEW.WORKTIME_ID "
                              "    ) "
                              "    WHERE ATTENDANCE_ID = NEW.ATTENDANCE_ID; "
                              "    UPDATE Attendance "
                              "    SET IS_EARLY = ( "
                              "        SELECT CASE "
                              "            WHEN NEW.CHECK_OUT_TIME < WorkTime.CHECK_OUT_TIME THEN 1 "
                              "            ELSE 0 "
                              "        END "
                              "        FROM WorkTime "
                              "        WHERE WorkTime.WORKTIME_ID = NEW.WORKTIME_ID "
                              "    ) "
                              "    WHERE ATTENDANCE_ID = NEW.ATTENDANCE_ID; "
                              "    UPDATE Attendance "
                              "    SET IS_ABSENT = ( "
                              "        CASE "
                              "            WHEN NEW.CHECK_IN_TIME IS NULL AND NEW.CHECK_OUT_TIME IS NULL THEN 1 "
                              "            ELSE 0 "
                              "        END "
                              "    ) "
                              "    WHERE ATTENDANCE_ID = NEW.ATTENDANCE_ID; "
                              "END;";
    rc = sqlite3_exec(db, sql_trigger, 0, 0, &zErrMsg);

    if (rc != SQLITE_OK)
    {
        printf("SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    else
        printf("Trigger check_attendance created successfully\n");

    /*--------------初始化工作时间-----------------*/
    // 上班时间：9:00 - 12:00
    const char *sql_worktime1 = "INSERT INTO WorkTime (WORKTIME_ID, CHECK_IN_TIME, CHECK_OUT_TIME) "
                                "VALUES (1, 9, 12);";
    rc = sqlite3_exec(db, sql_worktime1, 0, 0, &zErrMsg);

    if (rc != SQLITE_OK)
    {
        printf("SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    else
        printf("WorkTime1 created successfully\n");

    // 下班时间：13:00 - 17:00

    const char *sql_worktime2 = "INSERT INTO WorkTime (WORKTIME_ID, CHECK_IN_TIME, CHECK_OUT_TIME) "
                                "VALUES (2, 13, 17);";
    rc = sqlite3_exec(db, sql_worktime2, 0, 0, &zErrMsg);

    if (rc != SQLITE_OK)
    {
        printf("SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    else
        printf("WorkTime2 created successfully\n");
}
/**
 * @brief 查询用户数据，用于显示和删除,最多5行
 * @param page 第几页
 * @param *UserID 用户ID
 * @param *Text 查询到的用户参数
 * @param *count 查询到的用户数
 */
void *QueryUser(int page, int *count, int *UserID, char *Text)
{
    sqlite3_stmt *stmt;
    const char *sql = "SELECT UID, NAME, CREATED_AT FROM USERS "
                      "ORDER BY UID ASC "
                      "LIMIT 5 OFFSET ?;";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK)
    {
        printf("QueryUser: Failed to prepare statement\n");
        return;
    }

    sqlite3_bind_int(stmt, 1, page * 5);
    int row_count = 0;
    size_t text_size = 1024;
    size_t text_len = 0;
    Text[0] = '\0';

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        if (row_count < 5)
        {
            UserID[row_count] = sqlite3_column_int(stmt, 0);
        }

        char *uid = (char *)sqlite3_column_text(stmt, 0);
        char *name = (char *)sqlite3_column_text(stmt, 1);
        char *created_at = (char *)sqlite3_column_text(stmt, 2);

        // 拼接结果字符串
        strcat(Text, uid);
        strcat(Text, "\t");
        strcat(Text, name);
        strcat(Text, "\t");
        strcat(Text, created_at);
        strcat(Text, "\n");

        text_len = strlen(Text);
        row_count++;
    }

    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE)
    {
        printf("QueryUser: Failed to query info\n");
        return; // failed
    }

    // 查询总数
    const char *count_sql = "WITH UserPage AS (SELECT UID FROM USERS ORDER BY UID DESC LIMIT 5 OFFSET ?) "
                            "SELECT COUNT(*) AS total FROM UserPage;";
    rc = sqlite3_prepare_v2(db, count_sql, -1, &stmt, 0);
    if (rc != SQLITE_OK)
    {
        printf("QueryUser: Failed to prepare count statement\n");
        return;
    }

    sqlite3_bind_int(stmt, 1, page * 5);
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW)
    {
        *count = sqlite3_column_int(stmt, 0);
        printf("QueryUser: successfully query count\n");
    }
    else
    {
        printf("QueryUser: Failed to query count\n");
        sqlite3_finalize(stmt);
        return; // failed
    }
    sqlite3_finalize(stmt);
}
/**
 * @brief 查询打卡记录，用于显示，最多5行
 * @param *UserID 用户ID
 * @param *text 打卡记录
 */
void *QueryRecords(int page, int *UserID, char *text)
{
    sqlite3_stmt *stmt;
    const char *sql = "SELECT USER_ID, CHECK_IN_TIME, ATTENDANCE_ID FROM Attendance "
                      "ORDER BY USER_ID DESC "
                      "LIMIT 5 OFFSET ?;";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK)
    {
        printf("QueryRecords: Failed to prepare statement\n");
        return;
    }

    sqlite3_bind_int(stmt, 1, page * 5);

    int row_count = 0;
    size_t text_size = 1024;
    size_t text_len = 0;
    text[0] = '\0';

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        if (row_count <= 5)
        {
            UserID[row_count] = sqlite3_column_int(stmt, 0);
        }

        char *uid = (char *)sqlite3_column_blob(stmt, 0);
        printf("%d@uid:%s\n", row_count, uid);
        char *check_in_time = (char *)sqlite3_column_blob(stmt, 1);
        printf("%d@check_in_time:%s\n", row_count, check_in_time);
        char *Attendance_ID = (char *)sqlite3_column_blob(stmt, 2);
        printf("%d@Attendance_ID:%s\n", row_count, Attendance_ID);
        // 拼接结果字符串
        strcat(text, uid);
        strcat(text, "\t");
        strcat(text, check_in_time);
        strcat(text, "\t");
        strcat(text, Attendance_ID);
        strcat(text, "\n");
        printf("catok");
        text_len = strlen(text);
        row_count++;
    }

    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE)
    {
        printf("QueryRecords: Failed to query info\n");
        return; // failed
    }
}
/**
 *TODO 修改参数 char *Delete(int UserID);
 * @brief 删除用户数据
 * @param uid 用户ID
 * @return UserPage
 */
char *DeleteUser(int uid)
{
    char static data[200];

    sqlite3_stmt *stmt;
    const char *sql = "DELETE FROM USERS "
                      "WHERE UID = ?;";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK)
    {
        printf("DeleteUser: Failed to prepare statement\n");
        return 0;
    }

    sqlite3_bind_int(stmt, 1, uid);

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW)
    {
        char *result = (char *)sqlite3_column_text(stmt, 0);
        strcpy(data, result);
        strcat(data, "\t");
        printf("DeleteUser: Successfully delete\n");
        return data; // success
    }
    else
    {
        printf("DeleteUser: Failed to delete\n");
        return 0; // failed
    }
}
/**
 * @brief 统计信息
 * @param void
 * @return char*
 */
char *QueryBasicData()
{
    char static data[4]; // 大小：char[10]

    sqlite3_stmt *
        stmt;
    char *sql = "SELECT COUNT(UID) FROM USERS;"                               // 员工数
                "SELECT COUNT(USER_ID) FROM Attendance WHERE IS_LATE = TRUE;" // 今日迟到数
                "SELECT COUNT(USER_ID) FROM Attendance;"                      // 今日打卡数
                "SELECT (COUNT(CASE WHEN A.IS_ABSENT = FALSE THEN 1 END) / COUNT(UID)) * 100 AS RATE "
                "FROM USERS U "
                "LEFT JOIN Attendance A ON U.UID = A.USER_ID;"; // 今日出勤率;

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK)
    {
        printf("QueryBasicData: Failed to prepare statement\n");
        return 0;
    }

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW)
    {
        // char[0] : 员工数，char[1] 今日迟到数，可以写0不管，char[2] 今日打卡数, char[3] 今日出勤率（百分比）
        char *result = (char *)sqlite3_column_text(stmt, 0);
        strcpy(data, result);
        strcat(data, "\t");
        printf("QueryBasicData: Successfully query\n");
        return data; // success
    }
    else
    {
        printf("QueryBasicData: Failed to query\n");
        return 0; // failed
    }
}
/**
 * @brief 获取上班时间：上午上下班，下午上下班
 * @param schedule_id 上午:1 下午:2
 * @return data     上下班时间
 */
char *GetWorkTime(int schedule_id)
{
    int data[2];

    sqlite3_stmt *stmt;
    const char *sql = "SELECT CHECK_IN_TIME,CHECK_OUT_TIME FROM WorkTime "
                      "WHERE WORKTIME_ID = ?;";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);

    if (rc != SQLITE_OK)
    {
        printf("GetWorkTime: Failed to prepare statement\n");
        return 0;
    }
    // 绑定参数
    sqlite3_bind_int(stmt, 1, schedule_id);
    // 执行SQL语句
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW)
    {
        char *result = sqlite3_column_text(stmt, 0);
        reconvertTime(result, &data[0]);
        result = sqlite3_column_text(stmt, 1);
        reconvertTime(result, &data[1]);

        return data; // success
    }
    else
    {
        printf("GetWorkTime: Failed to query\n");
        return 0; // failed
    }
}
/**
 * 设置上班时间：上午上下班，下午上下班
 * @param time1 上班时间 只能是整点
 * @param time2 下班时间
 * @param schedule_id 上午/下午
 * @return void
 */
void SetWorkTime(int time1, int time2, int schedule_id)
{
    char checkInBuffer[9]; // HH:MM:SS 格式需要 8 个字符 + 1 个空字符
    char checkOutBuffer[9];

    convertTime(time1, checkInBuffer);
    convertTime(time2, checkOutBuffer);

    sqlite3_stmt *stmt;

    const char *sql = "UPDATE WorkTime "
                      "SET CHECK_IN_TIME = ? "
                      "SET CHECK_OUT_TIME = ? "
                      "WHERE WORKTIME_ID = ?;";
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK)
    {
        printf("SetWorkTime: Failed to prepare statement\n");
        return;
    }

    // 绑定参数
    sqlite3_bind_text(stmt, 1, checkInBuffer, 8, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, checkOutBuffer, 8, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 3, schedule_id);

    // 执行SQL语句
    rc = sqlite3_step(stmt);

    if (rc == SQLITE_ROW)
    {
        printf("SetWorkTime: Successfully update worktime\n");
        return; // success
    }
    else
    {
        printf("SetWorkTime: Failed to update worktime\n");
        return; // failed
    }
}

/**
 * @brief 创建用户
 * @param name 用户名
 * @param PIN 密码
 * @param User_ID 用户ID
 * @param Fingerprint_ID 指纹ID
 * @return 0 失败
 * @return 1 成功
 */
int CreateUser(char *name, char *PIN, int *User_ID, int Fingerprint_ID)
{
    sqlite3_stmt *stmt;
    const char *sql = "INSERT INTO USERS (NAME, PWD_HASH) "
                      "VALUES (?, ?);";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK)
    {
        printf("CreateUser: Failed to prepare statement\n");
        return;
    }
    // 计算密码哈希值
    char SHA256RS[65];
    sha256((unsigned char *)PIN, strlen(PIN), SHA256RS);

    // 绑定参数
    sqlite3_bind_text(stmt, 1, name, strlen(name), SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, SHA256RS, 64, SQLITE_STATIC);
    // 执行SQL语句
    rc = sqlite3_step(stmt);

    int UID;
    // 获取用户ID
    const char *sql2 = "SELECT UID FROM USERS "
                       "WHERE NAME = ? AND PWD_HASH = ?;";
    rc = sqlite3_prepare_v2(db, sql2, -1, &stmt, 0);
    if (rc != SQLITE_OK)
    {
        printf("CreateUser: Failed to prepare statement\n");
        return;
    }
    // 绑定参数
    sqlite3_bind_text(stmt, 1, name, strlen(name), SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, SHA256RS, 64, SQLITE_STATIC);

    // 执行SQL语句
    rc = sqlite3_step(stmt);
    printf("CreateRC:%d", rc);
    if (rc == SQLITE_ROW)
    {
        char *result = (char *)sqlite3_column_blob(stmt, 0);
        UID = atoi(result);
    }
    else
    {
        printf("CreateUser: Failed to create user\n");
        printf("CreateUser: Failed to get UID\n");
        return 0; // failed
    }

    *User_ID = UID;
    // 调用添加指纹函数
    addFingerprints(UID, Fingerprint_ID);

    printf("CreateUser: Successfully insert user\n");
    return 1; // success
}

/**
 * @brief 密码检查
 * @param *pwd 密码
 * @param *UserName 用户名
 * @param *UID 用户ID
 * @return 状态码 1成功 0失败 -1数据库错误
 */
int RootPasswordCheck(char *pwd, char *UserName, int *UID)
{
    char SHA256RS[65];
    sha256((unsigned char *)pwd, strlen(pwd), SHA256RS);

    // 数据库查询语句
    sqlite3_stmt *stmt;
    const char *sql = "SELECT PWD_HASH,UID,NAME FROM USERS "
                      "WHERE PWD_HASH = ? AND UID = ?;";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK)
    {
        printf("RootPasswordCheck: Failed to prepare statement\n");
        return 0;
    }
    // 绑定参数
    sqlite3_bind_text(stmt, 1, SHA256RS, 64, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, *UID);

    // 执行SQL语句
    rc = sqlite3_step(stmt);
#ifdef DEBUG
    printf("PWD User Input:%sEND\n", pwd);
    printf("PWD User Hash:%sEND\n", SHA256RS);
    printf("RC:%d\n", rc);
#ifdef DEBUG
    printf("UID:%d,Ready", *UID);
#endif
#endif
    if (rc == SQLITE_ROW)
    {

        // 对比密码哈希
        char *result = (char *)sqlite3_column_blob(stmt, 0);
#ifdef DEBUG
        printf("HashRs1:%s;\nHashRs2:%s;\nStrcmp:%d\n", result, SHA256RS, strcmp(result, SHA256RS));
#endif
        if (strcmp(result, SHA256RS) == 0 && *UID == sqlite3_column_int(stmt, 1))
        {
            // 获取用户名
#ifdef DEBUG
            printf("GET1");
#endif
            result = (char *)sqlite3_column_blob(stmt, 2);
#ifdef DEBUG
            printf("GET2");
#endif
            strcpy(UserName, result);
#ifdef DEBUG
            printf("GET3");
#endif
            return 1; // 成功
        }
        else
            return 0; // 失败
    }
    else
    {
        printf("RootPasswordCheck: Database error\n");
        return -1; // 数据库错误
    }
}

/**
 * @brief 检查用户密码
 *
 * @param pwd 密码明文
 * @param UserName 用户名
 * @param UID 用户ID
 * @return int
 */
int PasswordHashCheck(char *pwd, char *UserName, int *UID)
{
    char SHA256RS[65];
    sha256((unsigned char *)pwd, strlen(pwd), SHA256RS);

    // 数据库查询语句
    sqlite3_stmt *stmt;
    const char *sql = "SELECT NAME FROM USERS "
                      "WHERE PWD_HASH = ? AND UID = ?;";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK)
    {
        printf("PasswordHashCheck: Failed to prepare statement\n");
        return 0;
    }
    // 绑定参数
    sqlite3_bind_text(stmt, 1, SHA256RS, 64, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, *UID);

    // 执行SQL语句
    rc = sqlite3_step(stmt);

#ifdef DEBUG
    printf("PWD User Input:%sEND\n", pwd);
    printf("PWD User Hash:%sEND\n", SHA256RS);
    printf("RC:%d\n", rc);
#ifdef DEBUG
    printf("UID:%d,Ready", *UID);
#endif
#endif
    if (rc == SQLITE_ROW)
    {
        char *result = (char *)sqlite3_column_blob(stmt, 0);
        // 获取用户名
        result = (char *)sqlite3_column_blob(stmt, 0);
        sprintf(UserName, "%s", result);
        return 1;
    }
    else
    {
        printf("PasswordHashCheck: Database error\n");
        return -1; // 数据库错误
    }
}
/**
 * @brief 指纹检查
 *
 * @param *FingerprintsID 指纹ID
 * @param *UID 用户ID
 * @param *UserName 用户名
 * @return 1 成功
 * @return 0 失败
 */
int FingerprintsCheck(int FingerprintsID, int *UID, char *UserName)
{
    // 数据库查询语句
    sqlite3_stmt *stmt;
    const char *sql = "SELECT USER_ID FROM Fingerprints "
                      "WHERE FINGERPRINT_ID = ?;";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK)
    {
        printf("FingerprintsCheck: Failed to prepare statement\n");
        return 0;
    }
    // 绑定参数
    sqlite3_bind_int(stmt, 1, FingerprintsID);

    // 执行SQL语句
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW)
    {
        // 获取用户ID
        char *result = (char *)sqlite3_column_blob(stmt, 0);
        *UID = atoi(result);
        printf("FingerprintsCheck: Successfully query\n");
    }
    else
    {
        printf("FingerprintsCheck: No such fingerprints\n");
        return 0; // 数据库错误
    }

    // 查询用户ID
    const char *sql2 = "SELECT NAME FROM USERS "
                       "WHERE UID = ?;";
    rc = sqlite3_prepare_v2(db, sql2, -1, &stmt, 0);
    if (rc != SQLITE_OK)
    {
        printf("FingerprintsCheck: Failed to prepare statement\n");
        return 0;
    }
    // 绑定参数
    sqlite3_bind_int(stmt, 1, *UID);

    // 执行SQL语句
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW)
    {
        // 获取用户名
        char *result = (char *)sqlite3_column_blob(stmt, 0);
        strcpy(UserName, result);
        printf("FingerprintsCheck: Successfully query\n");
        return 1; // 成功
    }
    else
    {
        printf("FingerprintsCheck: No such user\n");
        return 0; // 数据库错误
    }
}

int addFingerprints(int UID, int FingerprintsID)
{
    sqlite3_stmt *stmt;
    const char *sql = "INSERT INTO Fingerprints (USER_ID, FINGERPRINT_ID) "
                      "VALUES (?, ?);";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK)
    {
        printf("addFingerprints: Failed to prepare statement\n");
        return 0;
    }
    // 绑定参数
    sqlite3_bind_int(stmt, 1, UID);
    sqlite3_bind_int(stmt, 2, FingerprintsID);

    // 执行SQL语句
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_DONE)
    {
        printf("addFingerprints: Successfully insert fingerprints\n");
        return 1; // 成功
    }
    else
    {
        printf("addFingerprints: Failed to insert fingerprints\n");
        return 0; // 失败
    }
}

int attendance_Checkin(int *UserID, int *WorkTimeID)
{
    sqlite3_stmt *stmt;
    const char *sql = "INSERT INTO Attendance (USER_ID, CHECK_IN_TIME, WORKTIME_ID) "
                      "VALUES (?, CURRENT_TIMESTAMP, ?);";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK)
    {
        printf("attendance_Checkin: Failed to prepare statement\n");
        return 0;
    }
    // 绑定参数
    sqlite3_bind_int(stmt, 1, *UserID);
    sqlite3_bind_int(stmt, 2, *WorkTimeID);

    // 执行SQL语句
    rc = sqlite3_step(stmt);

    if (rc == SQLITE_DONE)
    {
        printf("attendance_Checkin: Successfully insert checkin\n");
        return 1; // 成功
    }
    else
    {
        printf("attendance_Checkin: Failed to insert checkin\n");
        return 0; // 失败
    }
}

int attendance_Checkout(int *UserID, int *WorkTimeID)
{
    sqlite3_stmt *stmt;
    const char *sql = "UPDATE Attendance "
                      "SET CHECK_OUT_TIME = CURRENT_TIMESTAMP "
                      "WHERE USER_ID = ? AND WORKTIME_ID = ?;";

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK)
    {
        printf("attendance_Checkout: Failed to prepare statement\n");
        return 0;
    }
    // 绑定参数
    sqlite3_bind_int(stmt, 1, *UserID);
    sqlite3_bind_int(stmt, 2, *WorkTimeID);

    // 执行SQL语句
    rc = sqlite3_step(stmt);

    if (rc == SQLITE_DONE)
    {
        printf("attendance_Checkout: Successfully update checkout\n");
        return 1; // 成功
    }
    else
    {
        printf("attendance_Checkout: Failed to update checkout\n");
        return 0; // 失败
    }
}