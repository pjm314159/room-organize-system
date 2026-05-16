/**
 * @file types.h
 * @brief 公共类型定义头文件
 * @author Computer Room Scheduling System
 * @version 1.0
 * @date 2026
 * 
 * @description
 * 本文件定义了机房排课系统中使用的所有公共数据类型、常量和结构体。
 * 包括数据库路径、时间常量、状态码、枚举类型以及核心业务实体结构体。
 */

#ifndef TYPES_H
#define TYPES_H

/* 标准库头文件 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/*============================================================================
 * 配置常量定义
 *============================================================================*/

/** @brief SQLite 数据库文件路径 */
#define DB_PATH "./data/schedule.db"

/** @brief SQL 语句最大长度 */
#define MAX_SQL_LENGTH 1024

/** @brief 每周天数 */
#define DAYS_PER_WEEK 7

/** @brief 每天课时数 */
#define PERIODS_PER_DAY 8

/*============================================================================
 * 操作状态码定义
 *============================================================================*/

/** @brief 操作成功 */
#define SUCCESS 1

/** @brief 操作失败 */
#define FAILURE 0

/** @brief 错误：重复记录（唯一键冲突） */
#define ERROR_DUPLICATE -1

/** @brief 错误：记录未找到 */
#define ERROR_NOT_FOUND -2

/** @brief 错误：时间冲突 */
#define ERROR_CONFLICT -3

/** @brief 错误：无效输入 */
#define ERROR_INVALID_INPUT -4

/*============================================================================
 * 枚举类型定义
 *============================================================================*/

/**
 * @brief 机房状态枚举
 * 
 * 用于表示机房的当前使用状态，影响排课时的可选性。
 */
typedef enum {
    ROOM_STATUS_NORMAL = 0,      /**< 正常状态，可进行排课 */
    ROOM_STATUS_MAINTENANCE = 1  /**< 维护状态，暂停排课 */
} RoomStatus;

/*============================================================================
 * 数据结构体定义
 *============================================================================*/

/**
 * @brief 机房信息结构体
 * 
 * 存储机房的基本信息，包括编号、名称、容量和状态。
 * 对应数据库表：computer_room
 */
typedef struct {
    int id;                  /**< 主键ID（数据库自增） */
    char room_no[20];        /**< 机房编号（如 "R101"），唯一标识 */
    char name[50];           /**< 机房名称（如 "1号机房"） */
    int capacity;            /**< 容纳人数 */
    RoomStatus status;       /**< 机房状态 */
} ComputerRoom;

/**
 * @brief 教师信息结构体
 * 
 * 存储教师的基本信息，包括工号、姓名和联系方式。
 * 对应数据库表：teacher
 */
typedef struct {
    int id;                  /**< 主键ID（数据库自增） */
    char teacher_no[20];     /**< 工号（如 "T001"），唯一标识 */
    char name[50];           /**< 教师姓名 */
    char phone[20];          /**< 联系电话 */
} Teacher;

/**
 * @brief 课程信息结构体
 * 
 * 存储课程的基本信息，包括课程编号、名称、授课教师和课时数。
 * 对应数据库表：course
 */
typedef struct {
    int id;                  /**< 主键ID（数据库自增） */
    char course_no[20];      /**< 课程编号（如 "CS101"），唯一标识 */
    char name[50];           /**< 课程名称 */
    int teacher_id;          /**< 授课教师ID（外键，关联 teacher.id） */
    int hours;               /**< 所需课时数 */
} Course;

/**
 * @brief 排课记录结构体
 * 
 * 存储排课的核心信息，关联机房、课程、教师和时间。
 * 对应数据库表：schedule
 */
typedef struct {
    int id;                  /**< 主键ID（数据库自增） */
    int room_id;             /**< 机房ID（外键，关联 computer_room.id） */
    int course_id;           /**< 课程ID（外键，关联 course.id） */
    int teacher_id;          /**< 教师ID（外键，关联 teacher.id） */
    int day_of_week;         /**< 星期几（1-7，1=周一，7=周日） */
    int period;              /**< 第几节课（1-8） */
} Schedule;

/**
 * @brief 排课详情结构体（带关联信息）
 * 
 * 用于查询排课记录时，包含关联的机房、课程、教师名称，
 * 避免多次查询数据库。
 */
typedef struct {
    int schedule_id;         /**< 排课记录ID */
    int room_id;             /**< 机房ID */
    char room_name[50];      /**< 机房名称 */
    int course_id;           /**< 课程ID */
    char course_name[50];    /**< 课程名称 */
    int teacher_id;          /**< 教师ID */
    char teacher_name[50];   /**< 教师姓名 */
    int day_of_week;         /**< 星期几（1-7） */
    int period;              /**< 第几节课（1-8） */
} ScheduleDetail;

/**
 * @brief 时间槽结构体
 * 
 * 用于表示某个时间段的状态，主要用于二维数组索引和快速查找。
 * 当 schedule_id 为 -1 时表示该时间槽空闲。
 */
typedef struct {
    int schedule_id;         /**< 排课记录ID，-1 表示空闲 */
    int course_id;           /**< 课程ID */
    int teacher_id;          /**< 教师ID */
} TimeSlot;

/*============================================================================
 * 性能优化数据结构定义
 *============================================================================*/

/** @brief 哈希表默认容量 */
#define HASH_TABLE_DEFAULT_CAPACITY 1024

/**
 * @brief 哈希表节点
 * 
 * 链地址法解决冲突，每个桶存储一个链表。
 * 用于机房时间索引和教师时间索引。
 */
typedef struct HashNode {
    char key[32];                 /**< 哈希键（如 "1-3-2" 表示 ID1-周三-第2节） */
    int value;                    /**< 哈希值（存储排课记录ID） */
    struct HashNode *next;        /**< 链表下一个节点 */
} HashNode;

/**
 * @brief 哈希表结构体
 * 
 * 基于链地址法的哈希表实现，用于 O(1) 时间复杂度的冲突检测。
 * 以 "实体ID-星期-课时" 为键，排课ID 为值。
 */
typedef struct {
    HashNode **buckets;           /**< 桶数组 */
    int capacity;                 /**< 桶数量 */
    int count;                    /**< 已存储元素数量 */
} HashTable;

/**
 * @brief 机房周时间表结构体
 * 
 * 使用固定大小二维数组表示机房一周的时间安排。
 * schedule_table[day][period] 直接寻址，O(1) 时间复杂度。
 */
typedef struct {
    int room_id;                  /**< 机房ID */
    char room_name[50];           /**< 机房名称 */
    TimeSlot schedule_table[DAYS_PER_WEEK][PERIODS_PER_DAY]; /**< 7×8 时间表 */
} RoomWeekSchedule;

/**
 * @brief 排课索引管理器
 * 
 * 统一管理所有内存索引，包括哈希表索引和二维数组索引。
 * 在数据库操作时同步更新，提供 O(1) 的冲突检测能力。
 */
typedef struct {
    HashTable *room_time_index;       /**< 机房时间哈希索引 */
    HashTable *teacher_time_index;    /**< 教师时间哈希索引 */
    RoomWeekSchedule *room_schedules; /**< 机房周时间表数组 */
    int room_schedule_count;          /**< 机房时间表数量 */
    int room_schedule_capacity;       /**< 机房时间表数组容量 */
} ScheduleIndex;

#endif /* TYPES_H */
