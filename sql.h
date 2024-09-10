/* sql.h 这个函数用于数据库查询 */
#ifndef SQL_H
#define SQL_H

#include <sqlite3.h>
#include <stdio.h>
#include <sha256.h>
#include <string.h>
#include <stdlib.h>
#define DBFileName "a.db"
sqlite3 *db; /* 数据库指针 全局*/

int db_connect(char *dbName);
void CreateTablesOnce();
void *QueryUser(int page, int *count, int *UserID, char *Text);
void *QueryRecords(int page, int *UserID, char *text);
char *DeleteUser(int uid);
char *QueryBasicData();
char *GetWorkTime(int schedule_id);
void SetWorkTime(int time1, int time2, int schedule_id);
int CreateUser(char *name, char *PIN, int *User_ID, int Fingerprint_ID);
int RootPasswordCheck(char *pwd, char *UserName, int *UID);
int PasswordHashCheck(char *pwd, char *UserName, int *UID);
int FingerprintsCheck(int FingerprintsID, int *UID, char *UserName);
int addFingerprints(int UID, int FingerprintsID);
int attendance_Checkin(int *UserID, int *WorkTimeID);
int attendance_Checkout(int *UserID, int *WorkTimeID);

#endif // SQL_H
