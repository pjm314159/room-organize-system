/**
 * @file db.h
 * @brief 数据库操作模块头文件
 * @author Computer Room Scheduling System
 * @version 1.0
 * @date 2024
 * 
 * @description
 * 本文件声明了所有数据库操作函数，包括数据库的初始化、关闭，
 * 以及机房、教师、课程、排课的 CRUD 操作。
 * 
 * @note 所有函数均使用 SQLite3 数据库引擎
 * @note 返回值遵循 types.h 中定义的状态码规范
 */

#ifndef DB_H
#define DB_H

#include "types.h"
#include <sqlite3.h>

/*============================================================================
 * 数据库生命周期管理
 *============================================================================*/

/**
 * @brief 初始化数据库连接并创建表结构
 * 
 * 打开指定的 SQLite 数据库文件，如果文件不存在则自动创建。
 * 同时创建所需的数据表（computer_room, teacher, course, schedule）。
 * 
 * @param db_path 数据库文件路径
 * @return SUCCESS 成功
 * @return FAILURE 失败（无法打开数据库或创建表失败）
 * 
 * @note 会自动创建 data 目录（如果不存在）
 */
int db_init(const char *db_path);

/**
 * @brief 关闭数据库连接
 * 
 * 释放数据库资源，应在程序退出前调用。
 */
void db_close(void);

/*============================================================================
 * 机房表操作 (computer_room)
 *============================================================================*/

/**
 * @brief 创建新机房记录
 * @param room 机房信息（输入），成功后 id 字段被更新
 * @return SUCCESS 成功
 * @return ERROR_DUPLICATE 机房编号已存在
 */
int db_create_room(ComputerRoom *room);

/**
 * @brief 获取所有机房记录
 * @param rooms 机房数组指针（输出），需调用方释放
 * @param count 记录数量（输出）
 * @return SUCCESS 成功
 * @return FAILURE 失败
 */
int db_get_all_rooms(ComputerRoom **rooms, int *count);

/**
 * @brief 根据ID获取机房记录
 * @param id 机房ID
 * @param room 机房信息（输出）
 * @return SUCCESS 成功
 * @return ERROR_NOT_FOUND 未找到
 * @return FAILURE 查询失败
 */
int db_get_room_by_id(int id, ComputerRoom *room);

/**
 * @brief 更新机房记录
 * @param room 机房信息（输入，必须包含有效 id）
 * @return SUCCESS 成功
 * @return ERROR_NOT_FOUND 未找到
 * @return FAILURE 更新失败
 */
int db_update_room(ComputerRoom *room);

/**
 * @brief 删除机房记录
 * @param id 机房ID
 * @return SUCCESS 成功
 * @return ERROR_NOT_FOUND 未找到
 * @return FAILURE 删除失败
 */
int db_delete_room(int id);

/*============================================================================
 * 教师表操作 (teacher)
 *============================================================================*/

/**
 * @brief 创建新教师记录
 * @param teacher 教师信息（输入），成功后 id 字段被更新
 * @return SUCCESS 成功
 * @return ERROR_DUPLICATE 工号已存在
 */
int db_create_teacher(Teacher *teacher);

/**
 * @brief 获取所有教师记录
 * @param teachers 教师数组指针（输出），需调用方释放
 * @param count 记录数量（输出）
 * @return SUCCESS 成功
 * @return FAILURE 失败
 */
int db_get_all_teachers(Teacher **teachers, int *count);

/**
 * @brief 根据ID获取教师记录
 * @param id 教师ID
 * @param teacher 教师信息（输出）
 * @return SUCCESS 成功
 * @return ERROR_NOT_FOUND 未找到
 * @return FAILURE 查询失败
 */
int db_get_teacher_by_id(int id, Teacher *teacher);

/**
 * @brief 更新教师记录
 * @param teacher 教师信息（输入，必须包含有效 id）
 * @return SUCCESS 成功
 * @return ERROR_NOT_FOUND 未找到
 * @return FAILURE 更新失败
 */
int db_update_teacher(Teacher *teacher);

/**
 * @brief 删除教师记录
 * @param id 教师ID
 * @return SUCCESS 成功
 * @return ERROR_NOT_FOUND 未找到
 * @return FAILURE 删除失败
 */
int db_delete_teacher(int id);

/*============================================================================
 * 课程表操作 (course)
 *============================================================================*/

/**
 * @brief 创建新课程记录
 * @param course 课程信息（输入），成功后 id 字段被更新
 * @return SUCCESS 成功
 * @return ERROR_DUPLICATE 课程编号已存在
 */
int db_create_course(Course *course);

/**
 * @brief 获取所有课程记录
 * @param courses 课程数组指针（输出），需调用方释放
 * @param count 记录数量（输出）
 * @return SUCCESS 成功
 * @return FAILURE 失败
 */
int db_get_all_courses(Course **courses, int *count);

/**
 * @brief 根据ID获取课程记录
 * @param id 课程ID
 * @param course 课程信息（输出）
 * @return SUCCESS 成功
 * @return ERROR_NOT_FOUND 未找到
 * @return FAILURE 查询失败
 */
int db_get_course_by_id(int id, Course *course);

/**
 * @brief 更新课程记录
 * @param course 课程信息（输入，必须包含有效 id）
 * @return SUCCESS 成功
 * @return ERROR_NOT_FOUND 未找到
 * @return FAILURE 更新失败
 */
int db_update_course(Course *course);

/**
 * @brief 删除课程记录
 * @param id 课程ID
 * @return SUCCESS 成功
 * @return ERROR_NOT_FOUND 未找到
 * @return FAILURE 删除失败
 */
int db_delete_course(int id);

/*============================================================================
 * 排课表操作 (schedule)
 *============================================================================*/

/**
 * @brief 创建新排课记录
 * @param schedule 排课信息（输入），成功后 id 字段被更新
 * @return SUCCESS 成功
 * @return FAILURE 失败
 */
int db_create_schedule(Schedule *schedule);

/**
 * @brief 获取所有排课记录
 * @param schedules 排课数组指针（输出），需调用方释放
 * @param count 记录数量（输出）
 * @return SUCCESS 成功
 * @return FAILURE 失败
 */
int db_get_all_schedules(Schedule **schedules, int *count);

/**
 * @brief 根据ID获取排课记录
 * @param id 排课ID
 * @param schedule 排课信息（输出）
 * @return SUCCESS 成功
 * @return ERROR_NOT_FOUND 未找到
 * @return FAILURE 查询失败
 */
int db_get_schedule_by_id(int id, Schedule *schedule);

/**
 * @brief 更新排课记录
 * @param schedule 排课信息（输入，必须包含有效 id）
 * @return SUCCESS 成功
 * @return ERROR_NOT_FOUND 未找到
 * @return FAILURE 更新失败
 */
int db_update_schedule(Schedule *schedule);

/**
 * @brief 删除排课记录
 * @param id 排课ID
 * @return SUCCESS 成功
 * @return ERROR_NOT_FOUND 未找到
 * @return FAILURE 删除失败
 */
int db_delete_schedule(int id);

/*============================================================================
 * 排课详情查询
 *============================================================================*/

/**
 * @brief 获取排课详情列表（带关联信息）
 * 
 * 查询所有排课记录，并关联查询机房、课程、教师的名称。
 * 
 * @param details 详情数组指针（输出），需调用方释放
 * @param count 记录数量（输出）
 * @return SUCCESS 成功
 * @return FAILURE 失败
 */
int db_get_schedule_details(ScheduleDetail **details, int *count);

/*============================================================================
 * 条件查询
 *============================================================================*/

/**
 * @brief 按机房ID查询排课记录
 * @param room_id 机房ID
 * @param schedules 排课数组指针（输出），需调用方释放
 * @param count 记录数量（输出）
 * @return SUCCESS 成功
 * @return FAILURE 失败
 */
int db_get_schedules_by_room(int room_id, Schedule **schedules, int *count);

/**
 * @brief 按教师ID查询排课记录
 * @param teacher_id 教师ID
 * @param schedules 排课数组指针（输出），需调用方释放
 * @param count 记录数量（输出）
 * @return SUCCESS 成功
 * @return FAILURE 失败
 */
int db_get_schedules_by_teacher(int teacher_id, Schedule **schedules, int *count);

/**
 * @brief 按星期查询排课记录
 * @param day_of_week 星期几（1-7）
 * @param schedules 排课数组指针（输出），需调用方释放
 * @param count 记录数量（输出）
 * @return SUCCESS 成功
 * @return FAILURE 失败
 */
int db_get_schedules_by_day(int day_of_week, Schedule **schedules, int *count);

/*============================================================================
 * 冲突检测
 *============================================================================*/

/**
 * @brief 检测机房时间冲突
 * 
 * 检查指定机房在指定时间是否已有排课。
 * 
 * @param room_id 机房ID
 * @param day_of_week 星期几（1-7）
 * @param period 课时（1-8）
 * @return SUCCESS 无冲突
 * @return ERROR_CONFLICT 存在冲突
 * @return FAILURE 查询失败
 */
int db_check_room_conflict(int room_id, int day_of_week, int period);

/**
 * @brief 检测教师时间冲突
 * 
 * 检查指定教师在指定时间是否已有课程。
 * 
 * @param teacher_id 教师ID
 * @param day_of_week 星期几（1-7）
 * @param period 课时（1-8）
 * @return SUCCESS 无冲突
 * @return ERROR_CONFLICT 存在冲突
 * @return FAILURE 查询失败
 */
int db_check_teacher_conflict(int teacher_id, int day_of_week, int period);

/*============================================================================
 * 统计查询
 *============================================================================*/

/**
 * @brief 获取机房使用次数
 * @param room_id 机房ID
 * @return 使用次数（>= 0）
 */
int db_get_room_usage_count(int room_id);

#endif /* DB_H */
