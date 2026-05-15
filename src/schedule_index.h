/**
 * @file schedule_index.h
 * @brief 排课内存索引模块头文件
 * @author Computer Room Scheduling System
 * @version 1.0
 * @date 2024
 * 
 * @description
 * 本文件声明了排课内存索引相关函数，包括哈希表索引和二维数组索引。
 * 通过内存索引将冲突检测从 O(n) 优化到 O(1)。
 */

#ifndef SCHEDULE_INDEX_H
#define SCHEDULE_INDEX_H

#include "types.h"

/*============================================================================
 * 哈希表操作
 *============================================================================*/

/**
 * @brief 创建哈希表
 * @param capacity 桶数量
 * @return 哈希表指针，失败返回 NULL
 */
HashTable* hash_table_create(int capacity);

/**
 * @brief 销毁哈希表，释放所有内存
 * @param table 哈希表指针
 */
void hash_table_destroy(HashTable *table);

/**
 * @brief 向哈希表插入键值对
 * @param table 哈希表指针
 * @param key 键字符串
 * @param value 值（排课ID）
 * @return SUCCESS 成功，FAILURE 失败
 */
int hash_table_set(HashTable *table, const char *key, int value);

/**
 * @brief 从哈希表获取值
 * @param table 哈希表指针
 * @param key 键字符串
 * @return 值（排课ID），未找到返回 -1
 */
int hash_table_get(HashTable *table, const char *key);

/**
 * @brief 从哈希表删除键值对
 * @param table 哈希表指针
 * @param key 键字符串
 * @return SUCCESS 成功，ERROR_NOT_FOUND 未找到
 */
int hash_table_remove(HashTable *table, const char *key);

/*============================================================================
 * 排课索引管理
 *============================================================================*/

/**
 * @brief 创建排课索引管理器
 * @return 索引管理器指针，失败返回 NULL
 */
ScheduleIndex* schedule_index_create(void);

/**
 * @brief 销毁排课索引管理器
 * @param index 索引管理器指针
 */
void schedule_index_destroy(ScheduleIndex *index);

/**
 * @brief 从数据库加载所有排课记录到内存索引
 * 
 * 系统启动时调用，将数据库中的排课记录加载到哈希表和二维数组中。
 * 
 * @param index 索引管理器指针
 * @return SUCCESS 成功，FAILURE 失败
 */
int schedule_index_load(ScheduleIndex *index);

/**
 * @brief 添加排课记录到内存索引
 * @param index 索引管理器指针
 * @param schedule 排课记录
 * @return SUCCESS 成功，ERROR_CONFLICT 冲突
 */
int schedule_index_add(ScheduleIndex *index, Schedule *schedule);

/**
 * @brief 从内存索引删除排课记录
 * @param index 索引管理器指针
 * @param schedule 排课记录
 * @return SUCCESS 成功
 */
int schedule_index_remove(ScheduleIndex *index, Schedule *schedule);

/**
 * @brief 更新内存索引中的排课记录
 * @param index 索引管理器指针
 * @param old_schedule 旧排课记录
 * @param new_schedule 新排课记录
 * @return SUCCESS 成功，ERROR_CONFLICT 冲突
 */
int schedule_index_update(ScheduleIndex *index, Schedule *old_schedule, Schedule *new_schedule);

/*============================================================================
 * 快速冲突检测（O(1)）
 *============================================================================*/

/**
 * @brief 检测机房时间冲突（哈希表索引，O(1)）
 * @param index 索引管理器指针
 * @param room_id 机房ID
 * @param day_of_week 星期几（1-7）
 * @param period 课时（1-8）
 * @return SUCCESS 无冲突，ERROR_CONFLICT 存在冲突
 */
int schedule_index_check_room_conflict(ScheduleIndex *index, int room_id, int day_of_week, int period);

/**
 * @brief 检测教师时间冲突（哈希表索引，O(1)）
 * @param index 索引管理器指针
 * @param teacher_id 教师ID
 * @param day_of_week 星期几（1-7）
 * @param period 课时（1-8）
 * @return SUCCESS 无冲突，ERROR_CONFLICT 存在冲突
 */
int schedule_index_check_teacher_conflict(ScheduleIndex *index, int teacher_id, int day_of_week, int period);

/*============================================================================
 * 二维数组快速查询
 *============================================================================*/

/**
 * @brief 获取指定机房的周时间表
 * @param index 索引管理器指针
 * @param room_id 机房ID
 * @return 机房周时间表指针，未找到返回 NULL
 */
RoomWeekSchedule* schedule_index_get_room_schedule(ScheduleIndex *index, int room_id);

/**
 * @brief 计算指定机房的使用率
 * @param index 索引管理器指针
 * @param room_id 机房ID
 * @return 使用率（0.0 - 100.0）
 */
float schedule_index_get_room_usage_rate(ScheduleIndex *index, int room_id);

/**
 * @brief 获取指定机房指定天的排课数量
 * @param index 索引管理器指针
 * @param room_id 机房ID
 * @param day_of_week 星期几（1-7）
 * @return 排课数量
 */
int schedule_index_get_day_count(ScheduleIndex *index, int room_id, int day_of_week);

#endif /* SCHEDULE_INDEX_H */
