# 机房排课系统 - 开发任务分解

## 项目概述

- **项目名称**：机房排课系统（Computer Room Scheduling System）
- **项目类型**：C语言 Demo 项目
- **核心功能**：通过终端菜单操作 SQLite 数据库，实现机房课程的增删改查和排课管理
- **目标用户**：院校机房管理人员、培训机构教务人员
- **预计代码量**：约 1500-2000 行 C 代码

---

## 第一阶段：项目初始化

### 1.1 创建目录结构

```
schedule_system/
├── src/
│   ├── main.c           # 主程序入口
│   ├── db.c             # 数据库操作
│   ├── db.h             # 数据库头文件
│   ├── ui.c             # 用户界面
│   ├── ui.h             # 界面头文件
│   ├── logic.c          # 业务逻辑
│   ├── logic.h          # 逻辑头文件
│   ├── types.h          # 公共类型定义
│   └── Makefile         # 编译配置文件
├── data/                # 数据库文件存放目录
│   └── schedule.db      # SQLite 数据库文件
├── docs/                # 文档目录
│   ├── overview.md      # 项目设计文档
│   └── data-structure-optimization.md  # 数据结构优化方案
└── README.md            # 项目说明文档
```

**验收标准**：
- [ ] 目录结构符合上述规范
- [ ] 所有 .c 和 .h 文件已创建（内容可为空）
- [ ] Makefile 已配置

---

### 1.2 编写公共类型定义 (types.h)

**文件路径**：`src/types.h`

#### 1.2.1 包含的头文件

```c
#ifndef TYPES_H
#define TYPES_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#endif
```

#### 1.2.2 常量定义

```c
// 数据库相关
#define DB_PATH "./data/schedule.db"
#define MAX_SQL_LENGTH 1024

// 时间相关常量
#define DAYS_PER_WEEK 7
#define PERIODS_PER_DAY 8

// 状态码定义
#define SUCCESS 1
#define FAILURE 0
#define ERROR_DUPLICATE -1
#define ERROR_NOT_FOUND -2
#define ERROR_CONFLICT -3
#define ERROR_INVALID_INPUT -4
```

#### 1.2.3 机房状态枚举

```c
typedef enum {
    ROOM_STATUS_NORMAL = 0,   // 正常可用
    ROOM_STATUS_MAINTENANCE = 1  // 维护中
} RoomStatus;
```

#### 1.2.4 机房结构体

```c
typedef struct {
    int id;                  // 主键ID
    char room_no[20];        // 机房编号（如 "R101"）
    char name[50];           // 机房名称（如 "1号机房"）
    int capacity;            // 容纳人数
    RoomStatus status;       // 状态
} ComputerRoom;
```

#### 1.2.5 教师结构体

```c
typedef struct {
    int id;                  // 主键ID
    char teacher_no[20];     // 工号（如 "T001"）
    char name[50];           // 姓名
    char phone[20];          // 联系电话
} Teacher;
```

#### 1.2.6 课程结构体

```c
typedef struct {
    int id;                  // 主键ID
    char course_no[20];      // 课程编号（如 "CS101"）
    char name[50];           // 课程名称
    int teacher_id;          // 授课教师ID（外键）
    int hours;               // 所需课时数
} Course;
```

#### 1.2.7 排课结构体

```c
typedef struct {
    int id;                  // 主键ID
    int room_id;             // 机房ID（外键）
    int course_id;           // 课程ID（外键）
    int teacher_id;          // 教师ID（外键）
    int day_of_week;         // 星期几（1-7）
    int period;              // 第几节课（1-8）
} Schedule;
```

#### 1.2.8 排课查询结果结构体（带关联信息）

```c
typedef struct {
    int schedule_id;
    int room_id;
    char room_name[50];
    int course_id;
    char course_name[50];
    int teacher_id;
    char teacher_name[50];
    int day_of_week;
    int period;
} ScheduleDetail;
```

#### 1.2.9 时间槽结构体（用于二维数组索引）

```c
typedef struct {
    int schedule_id;         // -1 表示空闲
    int course_id;
    int teacher_id;
} TimeSlot;
```

**验收标准**：
- [ ] types.h 文件已创建
- [ ] 包含所有必要的头文件
- [ ] 所有结构体定义完整
- [ ] 编译无错误（空项目编译测试）

---

### 1.3 创建 Makefile

**文件路径**：`src/Makefile`

```makefile
CC = gcc
CFLAGS = -Wall -Wextra -std=c99
LDFLAGS = -lsqlite3
TARGET = schedule_system
SRC_DIR = .
BUILD_DIR = .

SOURCES = $(wildcard $(SRC_DIR)/*.c)
OBJECTS = $(SOURCES:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(BUILD_DIR)/*.o $(TARGET)

.PHONY: all clean
```

**验收标准**：
- [ ] Makefile 语法正确
- [ ] 可以执行 `make` 编译
- [ ] 可以执行 `make clean` 清理

---

## 第二阶段：数据库模块

### 2.1 数据库初始化 (db.c)

**文件路径**：`src/db.h`

```c
#ifndef DB_H
#define DB_H

#include "types.h"
#include <sqlite3.h>

// 数据库初始化和关闭
int db_init(const char *db_path);
void db_close(void);

// 机房表操作
int db_create_room(ComputerRoom *room);
int db_get_all_rooms(ComputerRoom **rooms, int *count);
int db_get_room_by_id(int id, ComputerRoom *room);
int db_update_room(ComputerRoom *room);
int db_delete_room(int id);

// 教师表操作
int db_create_teacher(Teacher *teacher);
int db_get_all_teachers(Teacher **teachers, int *count);
int db_get_teacher_by_id(int id, Teacher *teacher);
int db_update_teacher(Teacher *teacher);
int db_delete_teacher(int id);

// 课程表操作
int db_create_course(Course *course);
int db_get_all_courses(Course **courses, int *count);
int db_get_course_by_id(int id, Course *course);
int db_update_course(Course *course);
int db_delete_course(int id);

// 排课表操作
int db_create_schedule(Schedule *schedule);
int db_get_all_schedules(Schedule **schedules, int *count);
int db_get_schedule_by_id(int id, Schedule *schedule);
int db_update_schedule(Schedule *schedule);
int db_delete_schedule(int id);

// 排课详情查询（带关联信息）
int db_get_schedule_details(ScheduleDetail **details, int *count);

// 按条件查询排课
int db_get_schedules_by_room(int room_id, Schedule **schedules, int *count);
int db_get_schedules_by_teacher(int teacher_id, Schedule **schedules, int *count);
int db_get_schedules_by_day(int day_of_week, Schedule **schedules, int *count);

// 冲突检测
int db_check_room_conflict(int room_id, int day_of_week, int period);
int db_check_teacher_conflict(int teacher_id, int day_of_week, int period);

// 获取统计信息
int db_get_room_usage_count(int room_id);

#endif
```

**验收标准**：
- [ ] db.h 头文件已创建
- [ ] 函数声明完整，覆盖所有数据库操作

---

### 2.2 数据库实现 (db.c)

#### 2.2.1 全局变量和初始化

```c
#include "db.h"

static sqlite3 *db = NULL;

int db_init(const char *db_path) {
    if (sqlite3_open(db_path, &db) != SQLITE_OK) {
        fprintf(stderr, "无法打开数据库: %s\n", sqlite3_errmsg(db));
        return FAILURE;
    }

    // 创建表
    const char *sql =
        "CREATE TABLE IF NOT EXISTS computer_room ("
        "    id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "    room_no TEXT UNIQUE NOT NULL,"
        "    name TEXT NOT NULL,"
        "    capacity INTEGER NOT NULL,"
        "    status INTEGER DEFAULT 0"
        ");"

        "CREATE TABLE IF NOT EXISTS teacher ("
        "    id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "    teacher_no TEXT UNIQUE NOT NULL,"
        "    name TEXT NOT NULL,"
        "    phone TEXT"
        ");"

        "CREATE TABLE IF NOT EXISTS course ("
        "    id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "    course_no TEXT UNIQUE NOT NULL,"
        "    name TEXT NOT NULL,"
        "    teacher_id INTEGER,"
        "    hours INTEGER,"
        "    FOREIGN KEY (teacher_id) REFERENCES teacher(id)"
        ");"

        "CREATE TABLE IF NOT EXISTS schedule ("
        "    id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "    room_id INTEGER NOT NULL,"
        "    course_id INTEGER NOT NULL,"
        "    teacher_id INTEGER NOT NULL,"
        "    day_of_week INTEGER NOT NULL,"
        "    period INTEGER NOT NULL,"
        "    FOREIGN KEY (room_id) REFERENCES computer_room(id),"
        "    FOREIGN KEY (course_id) REFERENCES course(id),"
        "    FOREIGN KEY (teacher_id) REFERENCES teacher(id)"
        ");";

    char *err_msg = NULL;
    if (sqlite3_exec(db, sql, NULL, NULL, &err_msg) != SQLITE_OK) {
        fprintf(stderr, "SQL错误: %s\n", err_msg);
        sqlite3_free(err_msg);
        return FAILURE;
    }

    return SUCCESS;
}

void db_close(void) {
    if (db) {
        sqlite3_close(db);
        db = NULL;
    }
}
```

#### 2.2.2 机房 CRUD 实现

```c
// 创建机房
int db_create_room(ComputerRoom *room) {
    char sql[MAX_SQL_LENGTH];
    snprintf(sql, sizeof(sql),
        "INSERT INTO computer_room (room_no, name, capacity, status) VALUES ('%s', '%s', %d, %d);",
        room->room_no, room->name, room->capacity, room->status);

    if (sqlite3_exec(db, sql, NULL, NULL, NULL) != SQLITE_OK) {
        return ERROR_DUPLICATE;
    }

    room->id = (int)sqlite3_last_insert_rowid(db);
    return SUCCESS;
}

// 获取所有机房
int db_get_all_rooms(ComputerRoom **rooms, int *count) {
    const char *sql = "SELECT id, room_no, name, capacity, status FROM computer_room ORDER BY room_no;";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return FAILURE;
    }

    *count = 0;
    *rooms = NULL;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        *rooms = realloc(*rooms, (*count + 1) * sizeof(ComputerRoom));
        ComputerRoom *r = &(*rooms)[*count];
        r->id = sqlite3_column_int(stmt, 0);
        strcpy(r->room_no, (const char*)sqlite3_column_text(stmt, 1));
        strcpy(r->name, (const char*)sqlite3_column_text(stmt, 2));
        r->capacity = sqlite3_column_int(stmt, 3);
        r->status = sqlite3_column_int(stmt, 4);
        (*count)++;
    }

    sqlite3_finalize(stmt);
    return SUCCESS;
}

// 根据ID获取机房
int db_get_room_by_id(int id, ComputerRoom *room) {
    char sql[MAX_SQL_LENGTH];
    snprintf(sql, sizeof(sql),
        "SELECT id, room_no, name, capacity, status FROM computer_room WHERE id = %d;", id);

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return FAILURE;
    }

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        room->id = sqlite3_column_int(stmt, 0);
        strcpy(room->room_no, (const char*)sqlite3_column_text(stmt, 1));
        strcpy(room->name, (const char*)sqlite3_column_text(stmt, 2));
        room->capacity = sqlite3_column_int(stmt, 3);
        room->status = sqlite3_column_int(stmt, 4);
        sqlite3_finalize(stmt);
        return SUCCESS;
    }

    sqlite3_finalize(stmt);
    return ERROR_NOT_FOUND;
}

// 更新机房
int db_update_room(ComputerRoom *room) {
    char sql[MAX_SQL_LENGTH];
    snprintf(sql, sizeof(sql),
        "UPDATE computer_room SET room_no='%s', name='%s', capacity=%d, status=%d WHERE id=%d;",
        room->room_no, room->name, room->capacity, room->status, room->id);

    if (sqlite3_exec(db, sql, NULL, NULL, NULL) != SQLITE_OK) {
        return FAILURE;
    }

    return sqlite3_changes(db) > 0 ? SUCCESS : ERROR_NOT_FOUND;
}

// 删除机房
int db_delete_room(int id) {
    char sql[MAX_SQL_LENGTH];
    snprintf(sql, sizeof(sql), "DELETE FROM computer_room WHERE id = %d;", id);

    if (sqlite3_exec(db, sql, NULL, NULL, NULL) != SQLITE_OK) {
        return FAILURE;
    }

    return sqlite3_changes(db) > 0 ? SUCCESS : ERROR_NOT_FOUND;
}
```

#### 2.2.3 教师 CRUD 实现

```c
// 创建教师
int db_create_teacher(Teacher *teacher) {
    char sql[MAX_SQL_LENGTH];
    snprintf(sql, sizeof(sql),
        "INSERT INTO teacher (teacher_no, name, phone) VALUES ('%s', '%s', '%s');",
        teacher->teacher_no, teacher->name, teacher->phone);

    if (sqlite3_exec(db, sql, NULL, NULL, NULL) != SQLITE_OK) {
        return ERROR_DUPLICATE;
    }

    teacher->id = (int)sqlite3_last_insert_rowid(db);
    return SUCCESS;
}

// 获取所有教师
int db_get_all_teachers(Teacher **teachers, int *count) {
    const char *sql = "SELECT id, teacher_no, name, phone FROM teacher ORDER BY teacher_no;";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return FAILURE;
    }

    *count = 0;
    *teachers = NULL;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        *teachers = realloc(*teachers, (*count + 1) * sizeof(Teacher));
        Teacher *t = &(*teachers)[*count];
        t->id = sqlite3_column_int(stmt, 0);
        strcpy(t->teacher_no, (const char*)sqlite3_column_text(stmt, 1));
        strcpy(t->name, (const char*)sqlite3_column_text(stmt, 2));
        strcpy(t->phone, (const char*)sqlite3_column_text(stmt, 3));
        (*count)++;
    }

    sqlite3_finalize(stmt);
    return SUCCESS;
}

// 根据ID获取教师
int db_get_teacher_by_id(int id, Teacher *teacher) {
    char sql[MAX_SQL_LENGTH];
    snprintf(sql, sizeof(sql),
        "SELECT id, teacher_no, name, phone FROM teacher WHERE id = %d;", id);

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return FAILURE;
    }

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        teacher->id = sqlite3_column_int(stmt, 0);
        strcpy(teacher->teacher_no, (const char*)sqlite3_column_text(stmt, 1));
        strcpy(teacher->name, (const char*)sqlite3_column_text(stmt, 2));
        strcpy(teacher->phone, (const char*)sqlite3_column_text(stmt, 3));
        sqlite3_finalize(stmt);
        return SUCCESS;
    }

    sqlite3_finalize(stmt);
    return ERROR_NOT_FOUND;
}

// 更新教师
int db_update_teacher(Teacher *teacher) {
    char sql[MAX_SQL_LENGTH];
    snprintf(sql, sizeof(sql),
        "UPDATE teacher SET teacher_no='%s', name='%s', phone='%s' WHERE id=%d;",
        teacher->teacher_no, teacher->name, teacher->phone, teacher->id);

    if (sqlite3_exec(db, sql, NULL, NULL, NULL) != SQLITE_OK) {
        return FAILURE;
    }

    return sqlite3_changes(db) > 0 ? SUCCESS : ERROR_NOT_FOUND;
}

// 删除教师
int db_delete_teacher(int id) {
    char sql[MAX_SQL_LENGTH];
    snprintf(sql, sizeof(sql), "DELETE FROM teacher WHERE id = %d;", id);

    if (sqlite3_exec(db, sql, NULL, NULL, NULL) != SQLITE_OK) {
        return FAILURE;
    }

    return sqlite3_changes(db) > 0 ? SUCCESS : ERROR_NOT_FOUND;
}
```

#### 2.2.4 课程 CRUD 实现

```c
// 创建课程
int db_create_course(Course *course) {
    char sql[MAX_SQL_LENGTH];
    snprintf(sql, sizeof(sql),
        "INSERT INTO course (course_no, name, teacher_id, hours) VALUES ('%s', '%s', %d, %d);",
        course->course_no, course->name, course->teacher_id, course->hours);

    if (sqlite3_exec(db, sql, NULL, NULL, NULL) != SQLITE_OK) {
        return ERROR_DUPLICATE;
    }

    course->id = (int)sqlite3_last_insert_rowid(db);
    return SUCCESS;
}

// 获取所有课程
int db_get_all_courses(Course **courses, int *count) {
    const char *sql = "SELECT id, course_no, name, teacher_id, hours FROM course ORDER BY course_no;";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return FAILURE;
    }

    *count = 0;
    *courses = NULL;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        *courses = realloc(*courses, (*count + 1) * sizeof(Course));
        Course *c = &(*courses)[*count];
        c->id = sqlite3_column_int(stmt, 0);
        strcpy(c->course_no, (const char*)sqlite3_column_text(stmt, 1));
        strcpy(c->name, (const char*)sqlite3_column_text(stmt, 2));
        c->teacher_id = sqlite3_column_int(stmt, 3);
        c->hours = sqlite3_column_int(stmt, 4);
        (*count)++;
    }

    sqlite3_finalize(stmt);
    return SUCCESS;
}

// 根据ID获取课程
int db_get_course_by_id(int id, Course *course) {
    char sql[MAX_SQL_LENGTH];
    snprintf(sql, sizeof(sql),
        "SELECT id, course_no, name, teacher_id, hours FROM course WHERE id = %d;", id);

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return FAILURE;
    }

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        course->id = sqlite3_column_int(stmt, 0);
        strcpy(course->course_no, (const char*)sqlite3_column_text(stmt, 1));
        strcpy(course->name, (const char*)sqlite3_column_text(stmt, 2));
        course->teacher_id = sqlite3_column_int(stmt, 3);
        course->hours = sqlite3_column_int(stmt, 4);
        sqlite3_finalize(stmt);
        return SUCCESS;
    }

    sqlite3_finalize(stmt);
    return ERROR_NOT_FOUND;
}

// 更新课程
int db_update_course(Course *course) {
    char sql[MAX_SQL_LENGTH];
    snprintf(sql, sizeof(sql),
        "UPDATE course SET course_no='%s', name='%s', teacher_id=%d, hours=%d WHERE id=%d;",
        course->course_no, course->name, course->teacher_id, course->hours, course->id);

    if (sqlite3_exec(db, sql, NULL, NULL, NULL) != SQLITE_OK) {
        return FAILURE;
    }

    return sqlite3_changes(db) > 0 ? SUCCESS : ERROR_NOT_FOUND;
}

// 删除课程
int db_delete_course(int id) {
    char sql[MAX_SQL_LENGTH];
    snprintf(sql, sizeof(sql), "DELETE FROM course WHERE id = %d;", id);

    if (sqlite3_exec(db, sql, NULL, NULL, NULL) != SQLITE_OK) {
        return FAILURE;
    }

    return sqlite3_changes(db) > 0 ? SUCCESS : ERROR_NOT_FOUND;
}
```

#### 2.2.5 排课 CRUD 实现

```c
// 创建排课
int db_create_schedule(Schedule *schedule) {
    char sql[MAX_SQL_LENGTH];
    snprintf(sql, sizeof(sql),
        "INSERT INTO schedule (room_id, course_id, teacher_id, day_of_week, period) VALUES (%d, %d, %d, %d, %d);",
        schedule->room_id, schedule->course_id, schedule->teacher_id,
        schedule->day_of_week, schedule->period);

    if (sqlite3_exec(db, sql, NULL, NULL, NULL) != SQLITE_OK) {
        return FAILURE;
    }

    schedule->id = (int)sqlite3_last_insert_rowid(db);
    return SUCCESS;
}

// 获取所有排课
int db_get_all_schedules(Schedule **schedules, int *count) {
    const char *sql = "SELECT id, room_id, course_id, teacher_id, day_of_week, period FROM schedule ORDER BY day_of_week, period;";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return FAILURE;
    }

    *count = 0;
    *schedules = NULL;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        *schedules = realloc(*schedules, (*count + 1) * sizeof(Schedule));
        Schedule *s = &(*schedules)[*count];
        s->id = sqlite3_column_int(stmt, 0);
        s->room_id = sqlite3_column_int(stmt, 1);
        s->course_id = sqlite3_column_int(stmt, 2);
        s->teacher_id = sqlite3_column_int(stmt, 3);
        s->day_of_week = sqlite3_column_int(stmt, 4);
        s->period = sqlite3_column_int(stmt, 5);
        (*count)++;
    }

    sqlite3_finalize(stmt);
    return SUCCESS;
}

// 根据ID获取排课
int db_get_schedule_by_id(int id, Schedule *schedule) {
    char sql[MAX_SQL_LENGTH];
    snprintf(sql, sizeof(sql),
        "SELECT id, room_id, course_id, teacher_id, day_of_week, period FROM schedule WHERE id = %d;", id);

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return FAILURE;
    }

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        schedule->id = sqlite3_column_int(stmt, 0);
        schedule->room_id = sqlite3_column_int(stmt, 1);
        schedule->course_id = sqlite3_column_int(stmt, 2);
        schedule->teacher_id = sqlite3_column_int(stmt, 3);
        schedule->day_of_week = sqlite3_column_int(stmt, 4);
        schedule->period = sqlite3_column_int(stmt, 5);
        sqlite3_finalize(stmt);
        return SUCCESS;
    }

    sqlite3_finalize(stmt);
    return ERROR_NOT_FOUND;
}

// 更新排课
int db_update_schedule(Schedule *schedule) {
    char sql[MAX_SQL_LENGTH];
    snprintf(sql, sizeof(sql),
        "UPDATE schedule SET room_id=%d, course_id=%d, teacher_id=%d, day_of_week=%d, period=%d WHERE id=%d;",
        schedule->room_id, schedule->course_id, schedule->teacher_id,
        schedule->day_of_week, schedule->period, schedule->id);

    if (sqlite3_exec(db, sql, NULL, NULL, NULL) != SQLITE_OK) {
        return FAILURE;
    }

    return sqlite3_changes(db) > 0 ? SUCCESS : ERROR_NOT_FOUND;
}

// 删除排课
int db_delete_schedule(int id) {
    char sql[MAX_SQL_LENGTH];
    snprintf(sql, sizeof(sql), "DELETE FROM schedule WHERE id = %d;", id);

    if (sqlite3_exec(db, sql, NULL, NULL, NULL) != SQLITE_OK) {
        return FAILURE;
    }

    return sqlite3_changes(db) > 0 ? SUCCESS : ERROR_NOT_FOUND;
}
```

#### 2.2.6 排课详情查询

```c
// 获取排课详情（带关联信息）
int db_get_schedule_details(ScheduleDetail **details, int *count) {
    const char *sql =
        "SELECT s.id, s.room_id, r.name, s.course_id, c.name, "
        "       s.teacher_id, t.name, s.day_of_week, s.period "
        "FROM schedule s "
        "JOIN computer_room r ON s.room_id = r.id "
        "JOIN course c ON s.course_id = c.id "
        "JOIN teacher t ON s.teacher_id = t.id "
        "ORDER BY s.day_of_week, s.period;";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return FAILURE;
    }

    *count = 0;
    *details = NULL;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        *details = realloc(*details, (*count + 1) * sizeof(ScheduleDetail));
        ScheduleDetail *d = &(*details)[*count];
        d->schedule_id = sqlite3_column_int(stmt, 0);
        d->room_id = sqlite3_column_int(stmt, 1);
        strcpy(d->room_name, (const char*)sqlite3_column_text(stmt, 2));
        d->course_id = sqlite3_column_int(stmt, 3);
        strcpy(d->course_name, (const char*)sqlite3_column_text(stmt, 4));
        d->teacher_id = sqlite3_column_int(stmt, 5);
        strcpy(d->teacher_name, (const char*)sqlite3_column_text(stmt, 6));
        d->day_of_week = sqlite3_column_int(stmt, 7);
        d->period = sqlite3_column_int(stmt, 8);
        (*count)++;
    }

    sqlite3_finalize(stmt);
    return SUCCESS;
}
```

#### 2.2.7 按条件查询排课

```c
// 按机房查询排课
int db_get_schedules_by_room(int room_id, Schedule **schedules, int *count) {
    char sql[MAX_SQL_LENGTH];
    snprintf(sql, sizeof(sql),
        "SELECT id, room_id, course_id, teacher_id, day_of_week, period "
        "FROM schedule WHERE room_id = %d ORDER BY day_of_week, period;", room_id);

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return FAILURE;
    }

    *count = 0;
    *schedules = NULL;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        *schedules = realloc(*schedules, (*count + 1) * sizeof(Schedule));
        Schedule *s = &(*schedules)[*count];
        s->id = sqlite3_column_int(stmt, 0);
        s->room_id = sqlite3_column_int(stmt, 1);
        s->course_id = sqlite3_column_int(stmt, 2);
        s->teacher_id = sqlite3_column_int(stmt, 3);
        s->day_of_week = sqlite3_column_int(stmt, 4);
        s->period = sqlite3_column_int(stmt, 5);
        (*count)++;
    }

    sqlite3_finalize(stmt);
    return SUCCESS;
}

// 按教师查询排课
int db_get_schedules_by_teacher(int teacher_id, Schedule **schedules, int *count) {
    char sql[MAX_SQL_LENGTH];
    snprintf(sql, sizeof(sql),
        "SELECT id, room_id, course_id, teacher_id, day_of_week, period "
        "FROM schedule WHERE teacher_id = %d ORDER BY day_of_week, period;", teacher_id);

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return FAILURE;
    }

    *count = 0;
    *schedules = NULL;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        *schedules = realloc(*schedules, (*count + 1) * sizeof(Schedule));
        Schedule *s = &(*schedules)[*count];
        s->id = sqlite3_column_int(stmt, 0);
        s->room_id = sqlite3_column_int(stmt, 1);
        s->course_id = sqlite3_column_int(stmt, 2);
        s->teacher_id = sqlite3_column_int(stmt, 3);
        s->day_of_week = sqlite3_column_int(stmt, 4);
        s->period = sqlite3_column_int(stmt, 5);
        (*count)++;
    }

    sqlite3_finalize(stmt);
    return SUCCESS;
}

// 按星期查询排课
int db_get_schedules_by_day(int day_of_week, Schedule **schedules, int *count) {
    char sql[MAX_SQL_LENGTH];
    snprintf(sql, sizeof(sql),
        "SELECT id, room_id, course_id, teacher_id, day_of_week, period "
        "FROM schedule WHERE day_of_week = %d ORDER BY period;", day_of_week);

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return FAILURE;
    }

    *count = 0;
    *schedules = NULL;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        *schedules = realloc(*schedules, (*count + 1) * sizeof(Schedule));
        Schedule *s = &(*schedules)[*count];
        s->id = sqlite3_column_int(stmt, 0);
        s->room_id = sqlite3_column_int(stmt, 1);
        s->course_id = sqlite3_column_int(stmt, 2);
        s->teacher_id = sqlite3_column_int(stmt, 3);
        s->day_of_week = sqlite3_column_int(stmt, 4);
        s->period = sqlite3_column_int(stmt, 5);
        (*count)++;
    }

    sqlite3_finalize(stmt);
    return SUCCESS;
}
```

#### 2.2.8 冲突检测

```c
// 检测机房时间冲突
int db_check_room_conflict(int room_id, int day_of_week, int period) {
    char sql[MAX_SQL_LENGTH];
    snprintf(sql, sizeof(sql),
        "SELECT COUNT(*) FROM schedule WHERE room_id = %d AND day_of_week = %d AND period = %d;",
        room_id, day_of_week, period);

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return FAILURE;
    }

    int count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    return count > 0 ? ERROR_CONFLICT : SUCCESS;
}

// 检测教师时间冲突
int db_check_teacher_conflict(int teacher_id, int day_of_week, int period) {
    char sql[MAX_SQL_LENGTH];
    snprintf(sql, sizeof(sql),
        "SELECT COUNT(*) FROM schedule WHERE teacher_id = %d AND day_of_week = %d AND period = %d;",
        teacher_id, day_of_week, period);

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return FAILURE;
    }

    int count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    return count > 0 ? ERROR_CONFLICT : SUCCESS;
}
```

#### 2.2.9 统计信息

```c
// 获取机房使用次数
int db_get_room_usage_count(int room_id) {
    char sql[MAX_SQL_LENGTH];
    snprintf(sql, sizeof(sql),
        "SELECT COUNT(*) FROM schedule WHERE room_id = %d;", room_id);

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return 0;
    }

    int count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    return count;
}
```

**验收标准**：
- [ ] db.c 和 db.h 完整实现
- [ ] 数据库初始化时自动创建表
- [ ] 所有 CRUD 操作正确实现
- [ ] 冲突检测返回正确的错误码
- [ ] 编译通过，无内存泄漏

---

## 第三阶段：UI 模块

### 3.1 UI 头文件 (ui.h)

**文件路径**：`src/ui.h`

```c
#ifndef UI_H
#define UI_H

#include "types.h"

// 主菜单
void ui_show_main_menu(void);
int ui_get_main_choice(void);

// 基础数据管理菜单
void ui_show_data_menu(void);
int ui_get_data_choice(void);

// 机房管理菜单
void ui_show_room_menu(void);
int ui_get_room_choice(void);

// 课程管理菜单
void ui_show_course_menu(void);
int ui_get_course_choice(void);

// 教师管理菜单
void ui_show_teacher_menu(void);
int ui_get_teacher_choice(void);

// 排课管理菜单
void ui_show_schedule_menu(void);
int ui_get_schedule_choice(void);

// 输入函数
void ui_input_string(const char *prompt, char *buffer, int max_len);
int ui_input_int(const char *prompt);
int ui_input_int_range(const char *prompt, int min, int max);
char ui_input_confirm(const char *prompt);

// 显示函数
void ui_show_title(const char *title);
void ui_show_divider(void);
void ui_show_success(const char *message);
void ui_show_error(const char *message);
void ui_show_warning(const char *message);
void ui_pause(void);

// 机房显示
void ui_display_room(ComputerRoom *room);
void ui_display_rooms(ComputerRoom *rooms, int count);

// 教师显示
void ui_display_teacher(Teacher *teacher);
void ui_display_teachers(Teacher *teachers, int count);

// 课程显示
void ui_display_course(Course *course);
void ui_display_courses(Course *courses, int count);

// 排课显示
void ui_display_schedule(Schedule *schedule);
void ui_display_schedule_detail(ScheduleDetail *detail);
void ui_display_schedules(Schedule *schedules, int count);
void ui_display_schedule_details(ScheduleDetail *details, int count);

// 星期转换
const char* ui_get_day_name(int day);

#endif
```

**验收标准**：
- [ ] ui.h 函数声明完整
- [ ] 包含所有必要的输入输出函数

---

### 3.2 UI 实现 (ui.c)

#### 3.2.1 主菜单

```c
#include "ui.h"

void ui_show_main_menu(void) {
    printf("\n");
    printf("========================================\n");
    printf("          机房排课系统\n");
    printf("========================================\n");
    printf("1. 基础数据管理\n");
    printf("2. 排课管理\n");
    printf("3. 数据统计\n");
    printf("0. 退出系统\n");
    printf("========================================\n");
}

int ui_get_main_choice(void) {
    return ui_input_int_range("请选择操作: ", 0, 3);
}
```

#### 3.2.2 子菜单

```c
void ui_show_data_menu(void) {
    printf("\n");
    printf("========== 基础数据管理 ==========\n");
    printf("1. 机房管理\n");
    printf("2. 课程管理\n");
    printf("3. 教师管理\n");
    printf("0. 返回上级菜单\n");
    printf("================================\n");
}

int ui_get_data_choice(void) {
    return ui_input_int_range("请选择: ", 0, 3);
}

void ui_show_room_menu(void) {
    printf("\n");
    printf("========== 机房管理 ==========\n");
    printf("1. 添加机房\n");
    printf("2. 查看机房列表\n");
    printf("3. 修改机房信息\n");
    printf("4. 删除机房\n");
    printf("0. 返回上级菜单\n");
    printf("==============================\n");
}

int ui_get_room_choice(void) {
    return ui_input_int_range("请选择: ", 0, 4);
}

void ui_show_course_menu(void) {
    printf("\n");
    printf("========== 课程管理 ==========\n");
    printf("1. 添加课程\n");
    printf("2. 查看课程列表\n");
    printf("3. 修改课程信息\n");
    printf("4. 删除课程\n");
    printf("0. 返回上级菜单\n");
    printf("==============================\n");
}

int ui_get_course_choice(void) {
    return ui_input_int_range("请选择: ", 0, 4);
}

void ui_show_teacher_menu(void) {
    printf("\n");
    printf("========== 教师管理 ==========\n");
    printf("1. 添加教师\n");
    printf("2. 查看教师列表\n");
    printf("3. 修改教师信息\n");
    printf("4. 删除教师\n");
    printf("0. 返回上级菜单\n");
    printf("==============================\n");
}

int ui_get_teacher_choice(void) {
    return ui_input_int_range("请选择: ", 0, 4);
}

void ui_show_schedule_menu(void) {
    printf("\n");
    printf("========== 排课管理 ==========\n");
    printf("1. 创建排课\n");
    printf("2. 查询排课\n");
    printf("3. 修改排课\n");
    printf("4. 删除排课\n");
    printf("0. 返回上级菜单\n");
    printf("==============================\n");
}

int ui_get_schedule_choice(void) {
    return ui_input_int_range("请选择: ", 0, 4);
}
```

#### 3.2.3 输入函数

```c
void ui_input_string(const char *prompt, char *buffer, int max_len) {
    printf("%s", prompt);
    if (fgets(buffer, max_len, stdin) != NULL) {
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
        }
    }
}

int ui_input_int(const char *prompt) {
    char buffer[100];
    printf("%s", prompt);
    if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
        return atoi(buffer);
    }
    return 0;
}

int ui_input_int_range(const char *prompt, int min, int max) {
    int value;
    while (1) {
        value = ui_input_int(prompt);
        if (value >= min && value <= max) {
            break;
        }
        printf("输入无效，请输入 %d-%d 之间的数字\n", min, max);
    }
    return value;
}

char ui_input_confirm(const char *prompt) {
    char buffer[10];
    printf("%s (Y/N): ", prompt);
    if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
        char c = buffer[0];
        return (c == 'Y' || c == 'y') ? 'Y' : 'N';
    }
    return 'N';
}
```

#### 3.2.4 显示辅助函数

```c
void ui_show_title(const char *title) {
    printf("\n");
    printf("========== %s ==========\n", title);
}

void ui_show_divider(void) {
    printf("----------------------------------------\n");
}

void ui_show_success(const char *message) {
    printf("[成功] %s\n", message);
}

void ui_show_error(const char *message) {
    printf("[错误] %s\n", message);
}

void ui_show_warning(const char *message) {
    printf("[警告] %s\n", message);
}

void ui_pause(void) {
    printf("\n按回车键继续...");
    getchar();
}
```

#### 3.2.5 数据显示函数

```c
void ui_display_room(ComputerRoom *room) {
    const char *status_str = room->status == ROOM_STATUS_NORMAL ? "正常" : "维护";
    printf("|%4d | %-10s | %-15s | %4d | %-6s|\n",
           room->id, room->room_no, room->name, room->capacity, status_str);
}

void ui_display_rooms(ComputerRoom *rooms, int count) {
    if (count == 0) {
        printf("暂无机房信息\n");
        return;
    }

    ui_show_title("机房列表");
    printf("+-----+------------+-----------------+------+--------+\n");
    printf("| ID  | 机房编号    | 机房名称        | 容量 | 状态   |\n");
    printf("+-----+------------+-----------------+------+--------+\n");

    for (int i = 0; i < count; i++) {
        ui_display_room(&rooms[i]);
    }

    printf("+-----+------------+-----------------+------+--------+\n");
    printf("共 %d 条记录\n", count);
}

void ui_display_teacher(Teacher *teacher) {
    printf("|%4d | %-10s | %-15s | %-15s|\n",
           teacher->id, teacher->teacher_no, teacher->name, teacher->phone);
}

void ui_display_teachers(Teacher *teachers, int count) {
    if (count == 0) {
        printf("暂无教师信息\n");
        return;
    }

    ui_show_title("教师列表");
    printf("+-----+------------+-----------------+-----------------+\n");
    printf("| ID  | 工号        | 姓名            | 联系电话        |\n");
    printf("+-----+------------+-----------------+-----------------+\n");

    for (int i = 0; i < count; i++) {
        ui_display_teacher(&teachers[i]);
    }

    printf("+-----+------------+-----------------+-----------------+\n");
    printf("共 %d 条记录\n", count);
}

void ui_display_course(Course *course) {
    printf("|%4d | %-10s | %-15s | %4d |\n",
           course->id, course->course_no, course->name, course->hours);
}

void ui_display_courses(Course *courses, int count) {
    if (count == 0) {
        printf("暂无课程信息\n");
        return;
    }

    ui_show_title("课程列表");
    printf("+-----+------------+-----------------+------+\n");
    printf("| ID  | 课程编号    | 课程名称        | 课时 |\n");
    printf("+-----+------------+-----------------+------+\n");

    for (int i = 0; i < count; i++) {
        ui_display_course(&courses[i]);
    }

    printf("+-----+------------+-----------------+------+\n");
    printf("共 %d 条记录\n", count);
}

void ui_display_schedule(Schedule *schedule) {
    printf("|%4d | %4d       | %4d          | %4d       | %4d     |\n",
           schedule->id, schedule->room_id, schedule->course_id,
           schedule->teacher_id, schedule->day_of_week, schedule->period);
}

void ui_display_schedule_detail(ScheduleDetail *detail) {
    printf("|%4d | %-10s | %-15s | %-10s | %-4s  | 第%d节  |\n",
           detail->schedule_id,
           detail->room_name,
           detail->course_name,
           detail->teacher_name,
           ui_get_day_name(detail->day_of_week),
           detail->period);
}

void ui_display_schedule_details(ScheduleDetail *details, int count) {
    if (count == 0) {
        printf("暂无排课信息\n");
        return;
    }

    ui_show_title("排课详情");
    printf("+-----+------------+-----------------+-------------+--------+--------+\n");
    printf("| ID  | 机房        | 课程            | 教师        | 星期   | 课时   |\n");
    printf("+-----+------------+-----------------+-------------+--------+--------+\n");

    for (int i = 0; i < count; i++) {
        ui_display_schedule_detail(&details[i]);
    }

    printf("+-----+------------+-----------------+-------------+--------+--------+\n");
    printf("共 %d 条记录\n", count);
}

const char* ui_get_day_name(int day) {
    static const char *days[] = {"", "周一", "周二", "周三", "周四", "周五", "周六", "周日"};
    if (day >= 1 && day <= 7) {
        return days[day];
    }
    return "未知";
}
```

**验收标准**：
- [ ] ui.c 和 ui.h 完整实现
- [ ] 所有菜单显示正确
- [ ] 输入验证完整
- [ ] 显示格式整齐美观
- [ ] 编译通过

---

## 第四阶段：业务逻辑模块

### 4.1 逻辑头文件 (logic.h)

**文件路径**：`src/logic.h`

```c
#ifndef LOGIC_H
#define LOGIC_H

#include "types.h"
#include "db.h"

// 机房管理
int logic_add_room(void);
int logic_list_rooms(void);
int logic_update_room(void);
int logic_delete_room(void);

// 教师管理
int logic_add_teacher(void);
int logic_list_teachers(void);
int logic_update_teacher(void);
int logic_delete_teacher(void);

// 课程管理
int logic_add_course(void);
int logic_list_courses(void);
int logic_update_course(void);
int logic_delete_course(void);

// 排课管理
int logic_add_schedule(void);
int logic_list_schedules(void);
int logic_query_schedules(void);
int logic_update_schedule(void);
int logic_delete_schedule(void);

// 数据统计
int logic_show_statistics(void);

#endif
```

**验收标准**：
- [ ] logic.h 函数声明完整
- [ ] 包含所有业务逻辑函数

---

### 4.2 逻辑实现 (logic.c)

#### 4.2.1 机房管理逻辑

```c
#include "logic.h"
#include "ui.h"

int logic_add_room(void) {
    ComputerRoom room;

    ui_input_string("请输入机房编号: ", room.room_no, sizeof(room.room_no));
    if (strlen(room.room_no) == 0) {
        ui_show_error("机房编号不能为空");
        return ERROR_INVALID_INPUT;
    }

    ui_input_string("请输入机房名称: ", room.name, sizeof(room.name));
    if (strlen(room.name) == 0) {
        ui_show_error("机房名称不能为空");
        return ERROR_INVALID_INPUT;
    }

    room.capacity = ui_input_int_range("请输入容纳人数: ", 1, 500);
    room.status = ui_input_int_range("请输入状态 (0-正常, 1-维护): ", 0, 1);

    int result = db_create_room(&room);
    if (result == SUCCESS) {
        ui_show_success("机房添加成功");
        return SUCCESS;
    } else if (result == ERROR_DUPLICATE) {
        ui_show_error("机房编号已存在");
        return ERROR_DUPLICATE;
    } else {
        ui_show_error("添加失败");
        return FAILURE;
    }
}

int logic_list_rooms(void) {
    ComputerRoom *rooms = NULL;
    int count = 0;

    int result = db_get_all_rooms(&rooms, &count);
    if (result == SUCCESS) {
        ui_display_rooms(rooms, count);
        free(rooms);
        return SUCCESS;
    }

    ui_show_error("查询失败");
    return FAILURE;
}

int logic_update_room(void) {
    int id = ui_input_int("请输入要修改的机房ID (0返回): ");
    if (id == 0) return SUCCESS;

    ComputerRoom room;
    if (db_get_room_by_id(id, &room) != SUCCESS) {
        ui_show_error("机房不存在");
        return ERROR_NOT_FOUND;
    }

    ui_display_room(&room);

    printf("\n请输入新信息 (直接回车保持原值):\n");

    char buffer[100];
    ui_input_string("机房编号: ", buffer, sizeof(buffer));
    if (strlen(buffer) > 0) strcpy(room.room_no, buffer);

    ui_input_string("机房名称: ", buffer, sizeof(buffer));
    if (strlen(buffer) > 0) strcpy(room.name, buffer);

    int capacity = ui_input_int("容纳人数: ");
    if (capacity > 0) room.capacity = capacity;

    room.status = ui_input_int_range("状态 (0-正常, 1-维护): ", 0, 1);

    int result = db_update_room(&room);
    if (result == SUCCESS) {
        ui_show_success("修改成功");
        return SUCCESS;
    }

    ui_show_error("修改失败");
    return FAILURE;
}

int logic_delete_room(void) {
    int id = ui_input_int("请输入要删除的机房ID (0返回): ");
    if (id == 0) return SUCCESS;

    ComputerRoom room;
    if (db_get_room_by_id(id, &room) != SUCCESS) {
        ui_show_error("机房不存在");
        return ERROR_NOT_FOUND;
    }

    ui_display_room(&room);

    if (ui_input_confirm("确认删除") == 'Y') {
        int result = db_delete_room(id);
        if (result == SUCCESS) {
            ui_show_success("删除成功");
            return SUCCESS;
        }
        ui_show_error("删除失败");
        return FAILURE;
    }

    ui_show_warning("取消删除");
    return SUCCESS;
}
```

#### 4.2.2 教师管理逻辑

```c
int logic_add_teacher(void) {
    Teacher teacher;

    ui_input_string("请输入工号: ", teacher.teacher_no, sizeof(teacher.teacher_no));
    if (strlen(teacher.teacher_no) == 0) {
        ui_show_error("工号不能为空");
        return ERROR_INVALID_INPUT;
    }

    ui_input_string("请输入姓名: ", teacher.name, sizeof(teacher.name));
    if (strlen(teacher.name) == 0) {
        ui_show_error("姓名不能为空");
        return ERROR_INVALID_INPUT;
    }

    ui_input_string("请输入联系电话: ", teacher.phone, sizeof(teacher.phone));

    int result = db_create_teacher(&teacher);
    if (result == SUCCESS) {
        ui_show_success("教师添加成功");
        return SUCCESS;
    } else if (result == ERROR_DUPLICATE) {
        ui_show_error("工号已存在");
        return ERROR_DUPLICATE;
    } else {
        ui_show_error("添加失败");
        return FAILURE;
    }
}

int logic_list_teachers(void) {
    Teacher *teachers = NULL;
    int count = 0;

    int result = db_get_all_teachers(&teachers, &count);
    if (result == SUCCESS) {
        ui_display_teachers(teachers, count);
        free(teachers);
        return SUCCESS;
    }

    ui_show_error("查询失败");
    return FAILURE;
}

int logic_update_teacher(void) {
    int id = ui_input_int("请输入要修改的教师ID (0返回): ");
    if (id == 0) return SUCCESS;

    Teacher teacher;
    if (db_get_teacher_by_id(id, &teacher) != SUCCESS) {
        ui_show_error("教师不存在");
        return ERROR_NOT_FOUND;
    }

    ui_display_teacher(&teacher);

    printf("\n请输入新信息 (直接回车保持原值):\n");

    char buffer[100];
    ui_input_string("工号: ", buffer, sizeof(buffer));
    if (strlen(buffer) > 0) strcpy(teacher.teacher_no, buffer);

    ui_input_string("姓名: ", buffer, sizeof(buffer));
    if (strlen(buffer) > 0) strcpy(teacher.name, buffer);

    ui_input_string("联系电话: ", buffer, sizeof(buffer));
    if (strlen(buffer) > 0) strcpy(teacher.phone, buffer);

    int result = db_update_teacher(&teacher);
    if (result == SUCCESS) {
        ui_show_success("修改成功");
        return SUCCESS;
    }

    ui_show_error("修改失败");
    return FAILURE;
}

int logic_delete_teacher(void) {
    int id = ui_input_int("请输入要删除的教师ID (0返回): ");
    if (id == 0) return SUCCESS;

    Teacher teacher;
    if (db_get_teacher_by_id(id, &teacher) != SUCCESS) {
        ui_show_error("教师不存在");
        return ERROR_NOT_FOUND;
    }

    ui_display_teacher(&teacher);

    if (ui_input_confirm("确认删除") == 'Y') {
        int result = db_delete_teacher(id);
        if (result == SUCCESS) {
            ui_show_success("删除成功");
            return SUCCESS;
        }
        ui_show_error("删除失败");
        return FAILURE;
    }

    ui_show_warning("取消删除");
    return SUCCESS;
}
```

#### 4.2.3 课程管理逻辑

```c
int logic_add_course(void) {
    Course course;

    ui_input_string("请输入课程编号: ", course.course_no, sizeof(course.course_no));
    if (strlen(course.course_no) == 0) {
        ui_show_error("课程编号不能为空");
        return ERROR_INVALID_INPUT;
    }

    ui_input_string("请输入课程名称: ", course.name, sizeof(course.name));
    if (strlen(course.name) == 0) {
        ui_show_error("课程名称不能为空");
        return ERROR_INVALID_INPUT;
    }

    Teacher *teachers = NULL;
    int teacher_count = 0;
    db_get_all_teachers(&teachers, &teacher_count);

    if (teacher_count == 0) {
        ui_show_error("请先添加教师");
        free(teachers);
        return ERROR_NOT_FOUND;
    }

    ui_display_teachers(teachers, teacher_count);
    course.teacher_id = ui_input_int_range("请选择教师ID: ", 1, 1000);

    Teacher tmp;
    if (db_get_teacher_by_id(course.teacher_id, &tmp) != SUCCESS) {
        ui_show_error("教师不存在");
        free(teachers);
        return ERROR_NOT_FOUND;
    }

    course.hours = ui_input_int_range("请输入所需课时数: ", 1, 100);

    int result = db_create_course(&course);
    free(teachers);

    if (result == SUCCESS) {
        ui_show_success("课程添加成功");
        return SUCCESS;
    } else if (result == ERROR_DUPLICATE) {
        ui_show_error("课程编号已存在");
        return ERROR_DUPLICATE;
    } else {
        ui_show_error("添加失败");
        return FAILURE;
    }
}

int logic_list_courses(void) {
    Course *courses = NULL;
    int count = 0;

    int result = db_get_all_courses(&courses, &count);
    if (result == SUCCESS) {
        ui_display_courses(courses, count);
        free(courses);
        return SUCCESS;
    }

    ui_show_error("查询失败");
    return FAILURE;
}

int logic_update_course(void) {
    int id = ui_input_int("请输入要修改的课程ID (0返回): ");
    if (id == 0) return SUCCESS;

    Course course;
    if (db_get_course_by_id(id, &course) != SUCCESS) {
        ui_show_error("课程不存在");
        return ERROR_NOT_FOUND;
    }

    ui_display_course(&course);

    printf("\n请输入新信息 (直接回车保持原值):\n");

    char buffer[100];
    ui_input_string("课程编号: ", buffer, sizeof(buffer));
    if (strlen(buffer) > 0) strcpy(course.course_no, buffer);

    ui_input_string("课程名称: ", buffer, sizeof(buffer));
    if (strlen(buffer) > 0) strcpy(course.name, buffer);

    course.hours = ui_input_int("所需课时数: ");
    if (course.hours <= 0) course.hours = 1;

    int result = db_update_course(&course);
    if (result == SUCCESS) {
        ui_show_success("修改成功");
        return SUCCESS;
    }

    ui_show_error("修改失败");
    return FAILURE;
}

int logic_delete_course(void) {
    int id = ui_input_int("请输入要删除的课程ID (0返回): ");
    if (id == 0) return SUCCESS;

    Course course;
    if (db_get_course_by_id(id, &course) != SUCCESS) {
        ui_show_error("课程不存在");
        return ERROR_NOT_FOUND;
    }

    ui_display_course(&course);

    if (ui_input_confirm("确认删除") == 'Y') {
        int result = db_delete_course(id);
        if (result == SUCCESS) {
            ui_show_success("删除成功");
            return SUCCESS;
        }
        ui_show_error("删除失败");
        return FAILURE;
    }

    ui_show_warning("取消删除");
    return SUCCESS;
}
```

#### 4.2.4 排课管理逻辑

```c
int logic_add_schedule(void) {
    Schedule schedule;

    ComputerRoom *rooms = NULL;
    int room_count = 0;
    db_get_all_rooms(&rooms, &room_count);

    if (room_count == 0) {
        ui_show_error("请先添加机房");
        return ERROR_NOT_FOUND;
    }

    ui_display_rooms(rooms, room_count);
    schedule.room_id = ui_input_int_range("请选择机房ID: ", 1, 1000);

    ComputerRoom room;
    if (db_get_room_by_id(schedule.room_id, &room) != SUCCESS) {
        ui_show_error("机房不存在");
        free(rooms);
        return ERROR_NOT_FOUND;
    }
    free(rooms);

    Course *courses = NULL;
    int course_count = 0;
    db_get_all_courses(&courses, &course_count);

    if (course_count == 0) {
        ui_show_error("请先添加课程");
        return ERROR_NOT_FOUND;
    }

    ui_display_courses(courses, course_count);
    schedule.course_id = ui_input_int_range("请选择课程ID: ", 1, 1000);

    Course course;
    if (db_get_course_by_id(schedule.course_id, &course) != SUCCESS) {
        ui_show_error("课程不存在");
        free(courses);
        return ERROR_NOT_FOUND;
    }
    schedule.teacher_id = course.teacher_id;
    free(courses);

    printf("上课时间:\n");
    printf("星期 (1-7, 1=周一, 7=周日): ");
    schedule.day_of_week = ui_input_int_range("", 1, 7);
    schedule.period = ui_input_int_range("第几节课 (1-8): ", 1, 8);

    if (db_check_room_conflict(schedule.room_id, schedule.day_of_week, schedule.period) == ERROR_CONFLICT) {
        ui_show_error("该机房此时段已有排课");
        return ERROR_CONFLICT;
    }

    if (db_check_teacher_conflict(schedule.teacher_id, schedule.day_of_week, schedule.period) == ERROR_CONFLICT) {
        ui_show_error("该教师此时段已有课程");
        return ERROR_CONFLICT;
    }

    int result = db_create_schedule(&schedule);
    if (result == SUCCESS) {
        ui_show_success("排课创建成功");
        return SUCCESS;
    }

    ui_show_error("创建失败");
    return FAILURE;
}

int logic_list_schedules(void) {
    ScheduleDetail *details = NULL;
    int count = 0;

    int result = db_get_schedule_details(&details, &count);
    if (result == SUCCESS) {
        ui_display_schedule_details(details, count);
        free(details);
        return SUCCESS;
    }

    ui_show_error("查询失败");
    return FAILURE;
}

int logic_query_schedules(void) {
    printf("\n========== 排课查询 ==========\n");
    printf("1. 按机房查询\n");
    printf("2. 按教师查询\n");
    printf("3. 按星期查询\n");
    printf("0. 返回\n");

    int choice = ui_input_int_range("请选择: ", 0, 3);

    Schedule *schedules = NULL;
    int count = 0;
    int result = SUCCESS;

    switch (choice) {
        case 1: {
            ComputerRoom *rooms = NULL;
            int room_count = 0;
            db_get_all_rooms(&rooms, &room_count);

            if (room_count == 0) {
                ui_show_error("暂无机房");
                break;
            }

            ui_display_rooms(rooms, room_count);
            int room_id = ui_input_int_range("请选择机房ID: ", 1, 1000);
            free(rooms);

            result = db_get_schedules_by_room(room_id, &schedules, &count);
            break;
        }
        case 2: {
            Teacher *teachers = NULL;
            int teacher_count = 0;
            db_get_all_teachers(&teachers, &teacher_count);

            if (teacher_count == 0) {
                ui_show_error("暂无教师");
                break;
            }

            ui_display_teachers(teachers, teacher_count);
            int teacher_id = ui_input_int_range("请选择教师ID: ", 1, 1000);
            free(teachers);

            result = db_get_schedules_by_teacher(teacher_id, &schedules, &count);
            break;
        }
        case 3: {
            int day = ui_input_int_range("请输入星期 (1-7): ", 1, 7);
            result = db_get_schedules_by_day(day, &schedules, &count);
            break;
        }
        default:
            return SUCCESS;
    }

    if (result == SUCCESS) {
        if (count == 0) {
            printf("没有找到排课记录\n");
        } else {
            printf("\n找到 %d 条记录:\n", count);
            for (int i = 0; i < count; i++) {
                printf("  排课ID: %d, 机房ID: %d, 课程ID: %d, 教师ID: %d, 星期: %d, 课时: %d\n",
                       schedules[i].id, schedules[i].room_id, schedules[i].course_id,
                       schedules[i].teacher_id, schedules[i].day_of_week, schedules[i].period);
            }
        }
        free(schedules);
    } else {
        ui_show_error("查询失败");
    }

    return result;
}

int logic_update_schedule(void) {
    int id = ui_input_int("请输入要修改的排课ID (0返回): ");
    if (id == 0) return SUCCESS;

    Schedule schedule;
    if (db_get_schedule_by_id(id, &schedule) != SUCCESS) {
        ui_show_error("排课不存在");
        return ERROR_NOT_FOUND;
    }

    printf("\n当前信息:\n");
    printf("  机房ID: %d, 课程ID: %d, 教师ID: %d\n", schedule.room_id, schedule.course_id, schedule.teacher_id);
    printf("  时间: 星期%d, 第%d节课\n", schedule.day_of_week, schedule.period);

    int new_day = ui_input_int_range("新星期 (1-7): ", 1, 7);
    int new_period = ui_input_int_range("新课次 (1-8): ", 1, 8);

    if (db_check_room_conflict(schedule.room_id, new_day, new_period) == ERROR_CONFLICT) {
        ui_show_error("该机房此时段已有排课");
        return ERROR_CONFLICT;
    }

    if (db_check_teacher_conflict(schedule.teacher_id, new_day, new_period) == ERROR_CONFLICT) {
        ui_show_error("该教师此时段已有课程");
        return ERROR_CONFLICT;
    }

    schedule.day_of_week = new_day;
    schedule.period = new_period;

    int result = db_update_schedule(&schedule);
    if (result == SUCCESS) {
        ui_show_success("修改成功");
        return SUCCESS;
    }

    ui_show_error("修改失败");
    return FAILURE;
}

int logic_delete_schedule(void) {
    int id = ui_input_int("请输入要删除的排课ID (0返回): ");
    if (id == 0) return SUCCESS;

    Schedule schedule;
    if (db_get_schedule_by_id(id, &schedule) != SUCCESS) {
        ui_show_error("排课不存在");
        return ERROR_NOT_FOUND;
    }

    printf("当前信息:\n");
    printf("  机房ID: %d, 课程ID: %d, 教师ID: %d, 星期: %d, 课时: %d\n",
           schedule.room_id, schedule.course_id, schedule.teacher_id,
           schedule.day_of_week, schedule.period);

    if (ui_input_confirm("确认删除") == 'Y') {
        int result = db_delete_schedule(id);
        if (result == SUCCESS) {
            ui_show_success("删除成功");
            return SUCCESS;
        }
        ui_show_error("删除失败");
        return FAILURE;
    }

    ui_show_warning("取消删除");
    return SUCCESS;
}
```

#### 4.2.5 数据统计逻辑

```c
int logic_show_statistics(void) {
    ui_show_title("数据统计");

    ComputerRoom *rooms = NULL;
    int room_count = 0;
    db_get_all_rooms(&rooms, &room_count);

    printf("\n机房数量: %d\n", room_count);

    if (room_count > 0) {
        printf("\n机房使用情况:\n");
        printf("+------------+-----------------+------+\n");
        printf("| 机房名称    | 机房编号        | 使用次数 |\n");
        printf("+------------+-----------------+------+\n");

        for (int i = 0; i < room_count; i++) {
            int usage = db_get_room_usage_count(rooms[i].id);
            printf("| %-10s | %-15s | %d     |\n",
                   rooms[i].name, rooms[i].room_no, usage);
        }
        printf("+------------+-----------------+------+\n");

        free(rooms);
    }

    Teacher *teachers = NULL;
    int teacher_count = 0;
    db_get_all_teachers(&teachers, &teacher_count);
    printf("\n教师数量: %d\n", teacher_count);
    free(teachers);

    Course *courses = NULL;
    int course_count = 0;
    db_get_all_courses(&courses, &course_count);
    printf("课程数量: %d\n", course_count);
    free(courses);

    Schedule *schedules = NULL;
    int schedule_count = 0;
    db_get_all_schedules(&schedules, &schedule_count);
    printf("排课数量: %d\n", schedule_count);
    free(schedules);

    if (schedule_count > 0 && room_count > 0) {
        float usage_rate = (float)schedule_count / (room_count * DAYS_PER_WEEK * PERIODS_PER_DAY) * 100;
        printf("\n总体使用率: %.1f%%\n", usage_rate);
    }

    return SUCCESS;
}
```

**验收标准**：
- [ ] logic.c 和 logic.h 完整实现
- [ ] 所有业务逻辑正确
- [ ] 输入验证完整
- [ ] 错误处理得当
- [ ] 编译通过

---

## 第五阶段：主程序集成

### 5.1 主程序 (main.c)

**文件路径**：`src/main.c`

```c
#include <stdio.h>
#include <stdlib.h>
#include "types.h"
#include "db.h"
#include "ui.h"
#include "logic.h"

int main(void) {
    printf("\n");
    printf("========================================\n");
    printf("      欢迎使用机房排课系统\n");
    printf("========================================\n");

    if (db_init(DB_PATH) != SUCCESS) {
        printf("[错误] 数据库初始化失败\n");
        return 1;
    }

    printf("[提示] 数据库初始化成功\n");

    int running = 1;
    while (running) {
        ui_show_main_menu();
        int main_choice = ui_get_main_choice();

        switch (main_choice) {
            case 1: {
                int back = 0;
                while (!back) {
                    ui_show_data_menu();
                    int data_choice = ui_get_data_choice();

                    switch (data_choice) {
                        case 1: {
                            int back_room = 0;
                            while (!back_room) {
                                ui_show_room_menu();
                                int room_choice = ui_get_room_choice();

                                switch (room_choice) {
                                    case 1: logic_add_room(); break;
                                    case 2: logic_list_rooms(); break;
                                    case 3: logic_update_room(); break;
                                    case 4: logic_delete_room(); break;
                                    case 0: back_room = 1; break;
                                }
                                if (room_choice != 0) ui_pause();
                            }
                            break;
                        }
                        case 2: {
                            int back_course = 0;
                            while (!back_course) {
                                ui_show_course_menu();
                                int course_choice = ui_get_course_choice();

                                switch (course_choice) {
                                    case 1: logic_add_course(); break;
                                    case 2: logic_list_courses(); break;
                                    case 3: logic_update_course(); break;
                                    case 4: logic_delete_course(); break;
                                    case 0: back_course = 1; break;
                                }
                                if (course_choice != 0) ui_pause();
                            }
                            break;
                        }
                        case 3: {
                            int back_teacher = 0;
                            while (!back_teacher) {
                                ui_show_teacher_menu();
                                int teacher_choice = ui_get_teacher_choice();

                                switch (teacher_choice) {
                                    case 1: logic_add_teacher(); break;
                                    case 2: logic_list_teachers(); break;
                                    case 3: logic_update_teacher(); break;
                                    case 4: logic_delete_teacher(); break;
                                    case 0: back_teacher = 1; break;
                                }
                                if (teacher_choice != 0) ui_pause();
                            }
                            break;
                        }
                        case 0:
                            back = 1;
                            break;
                    }
                }
                break;
            }

            case 2: {
                int back = 0;
                while (!back) {
                    ui_show_schedule_menu();
                    int schedule_choice = ui_get_schedule_choice();

                    switch (schedule_choice) {
                        case 1: logic_add_schedule(); break;
                        case 2: logic_list_schedules(); break;
                        case 3: logic_query_schedules(); break;
                        case 4: logic_update_schedule(); break;
                        case 5: logic_delete_schedule(); break;
                        case 0: back = 1; break;
                    }
                    if (schedule_choice != 0) ui_pause();
                }
                break;
            }

            case 3:
                logic_show_statistics();
                ui_pause();
                break;

            case 0:
                running = 0;
                ui_show_success("感谢使用，再见！");
                break;
        }
    }

    db_close();
    return 0;
}
```

**验收标准**：
- [ ] main.c 完整实现
- [ ] 菜单流程正确
- [ ] 可以正常运行
- [ ] 数据库正确关闭
- [ ] 无内存泄漏

---

## 第六阶段：编译测试

### 6.1 编译步骤

```bash
# 进入 src 目录
cd src

# 编译
gcc main.c db.c ui.c logic.c -o schedule_system -lsqlite3

# 或者使用 Makefile
make
```

### 6.2 运行测试

```bash
./schedule_system
```

### 6.3 测试用例

#### 6.3.1 添加机房
```
输入：
1. 机房编号: R101
2. 机房名称: 1号机房
3. 容量: 50
4. 状态: 0

预期：显示"机房添加成功"
```

#### 6.3.2 添加教师
```
输入：
1. 工号: T001
2. 姓名: 张老师
3. 电话: 13800138000

预期：显示"教师添加成功"
```

#### 6.3.3 添加课程
```
输入：
1. 课程编号: CS101
2. 课程名称: C语言程序设计
3. 选择教师: 1
4. 课时: 4

预期：显示"课程添加成功"
```

#### 6.3.4 创建排课
```
输入：
1. 选择机房: 1
2. 选择课程: 1
3. 星期: 1
4. 课时: 1

预期：显示"排课创建成功"
```

#### 6.3.5 冲突检测
```
操作：再次创建相同机房、相同时间的排课

预期：显示"该机房此时段已有排课"
```

---

## 第七阶段：性能优化（可选）

如果需要提升性能，可以按照 data-structure-optimization.md 中的方案实现：

### 7.1 哈希表索引

```c
// 在 db.c 中添加全局索引
static HashTable *room_time_index = NULL;
static HashTable *teacher_time_index = NULL;

// 在 db_init 中初始化索引
room_time_index = hash_table_create(1024);
teacher_time_index = hash_table_create(1024);

// 在 db_create_schedule 中更新索引
hash_table_set(room_time_index, make_key(schedule->room_id, ...), schedule);
hash_table_set(teacher_time_index, make_key(schedule->teacher_id, ...), schedule);

// 在 db_check_room_conflict 中使用索引
return hash_table_get(room_time_index, make_key(room_id, day, period)) != NULL;
```

### 7.2 二维数组索引

```c
// 机房时间表
static TimeSlot room_schedule[7][8];

// 初始化
memset(room_schedule, -1, sizeof(room_schedule));

// 安排课程
room_schedule[day - 1][period - 1].schedule_id = schedule->id;

// 查询空闲
int is_free = room_schedule[day - 1][period - 1].schedule_id == -1;
```

---

## 验收检查清单

### 必须完成
- [ ] types.h 公共类型定义完整
- [ ] db.c/db.h 数据库操作完整
- [ ] ui.c/ui.h 用户界面完整
- [ ] logic.c/logic.h 业务逻辑完整
- [ ] main.c 主程序正常执行
- [ ] Makefile 编译配置正确
- [ ] 可以成功添加、查询、删除数据
- [ ] 排课冲突检测正常工作

### 代码质量
- [ ] 无编译警告
- [ ] 无内存泄漏（使用 valgrind 检查）
- [ ] 代码格式整齐
- [ ] 函数命名清晰

### 文档
- [ ] README.md 编译运行说明
- [ ] 注释完整（关键逻辑）
