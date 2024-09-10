# gec6818 指纹考勤系统

基于 粤嵌® gec6818 开发板的指纹考勤系统的硬件部分和数据库部分代码

FileTree:
```
 ┣ sqlite
 ┃ ┣ CMakeLists.txt
 ┃ ┣ shell.c
 ┃ ┣ sqlite3.c
 ┃ ┣ sqlite3.h
 ┃ ┣ sqlite3ext.h
 ┃ ┗ test.sql
 ┣ .gitignore
 ┣ AS608_command.h // 指纹模块的命令集
 ┣ Finger.c // 指纹模块的驱动
 ┣ Finger.h 
 ┣ LICENSE.md
 ┣ README.md
 ┣ sha256.c // sha256加密算法（非标准结果）
 ┣ sha256.h
 ┣ sql.c // 数据库操作
 ┣ sql.h
 ┣ TimerEvent.c //计时器（多线程）
 ┗ TimerEvent.h
```
 
