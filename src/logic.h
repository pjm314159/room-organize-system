/**
 * @file logic.h
 * @brief 业务逻辑模块头文件
 * @author Computer Room Scheduling System
 * @version 1.0
 * @date 2024
 * 
 * @description
 * 本文件声明了所有业务逻辑函数，负责处理用户操作、数据验证、
 * 业务规则检查等功能。作为 UI 层和 DB 层之间的桥梁。
 */

#ifndef LOGIC_H
#define LOGIC_H

#include "types.h"
#include "db.h"

/*============================================================================
 * 机房管理业务逻辑
 *============================================================================*/

/**
 * @brief 添加机房
 * 
 * 交互式获取用户输入，验证数据后调用数据库接口创建机房记录。
 * 
 * @return SUCCESS 成功
 * @return ERROR_INVALID_INPUT 输入无效
 * @return ERROR_DUPLICATE 机房编号已存在
 * @return FAILURE 操作失败
 */
int logic_add_room(void);

/**
 * @brief 列出所有机房
 * @return SUCCESS 成功
 * @return FAILURE 查询失败
 */
int logic_list_rooms(void);

/**
 * @brief 更新机房信息
 * @return SUCCESS 成功
 * @return ERROR_NOT_FOUND 机房不存在
 * @return FAILURE 更新失败
 */
int logic_update_room(void);

/**
 * @brief 删除机房
 * @return SUCCESS 成功
 * @return ERROR_NOT_FOUND 机房不存在
 * @return FAILURE 删除失败
 */
int logic_delete_room(void);

/*============================================================================
 * 教师管理业务逻辑
 *============================================================================*/

/**
 * @brief 添加教师
 * @return SUCCESS 成功
 * @return ERROR_INVALID_INPUT 输入无效
 * @return ERROR_DUPLICATE 工号已存在
 * @return FAILURE 操作失败
 */
int logic_add_teacher(void);

/**
 * @brief 列出所有教师
 * @return SUCCESS 成功
 * @return FAILURE 查询失败
 */
int logic_list_teachers(void);

/**
 * @brief 更新教师信息
 * @return SUCCESS 成功
 * @return ERROR_NOT_FOUND 教师不存在
 * @return FAILURE 更新失败
 */
int logic_update_teacher(void);

/**
 * @brief 删除教师
 * @return SUCCESS 成功
 * @return ERROR_NOT_FOUND 教师不存在
 * @return FAILURE 删除失败
 */
int logic_delete_teacher(void);

/*============================================================================
 * 课程管理业务逻辑
 *============================================================================*/

/**
 * @brief 添加课程
 * @return SUCCESS 成功
 * @return ERROR_INVALID_INPUT 输入无效
 * @return ERROR_NOT_FOUND 关联教师不存在
 * @return ERROR_DUPLICATE 课程编号已存在
 * @return FAILURE 操作失败
 */
int logic_add_course(void);

/**
 * @brief 列出所有课程
 * @return SUCCESS 成功
 * @return FAILURE 查询失败
 */
int logic_list_courses(void);

/**
 * @brief 更新课程信息
 * @return SUCCESS 成功
 * @return ERROR_NOT_FOUND 课程不存在
 * @return FAILURE 更新失败
 */
int logic_update_course(void);

/**
 * @brief 删除课程
 * @return SUCCESS 成功
 * @return ERROR_NOT_FOUND 课程不存在
 * @return FAILURE 删除失败
 */
int logic_delete_course(void);

/*============================================================================
 * 排课管理业务逻辑
 *============================================================================*/

/**
 * @brief 创建排课
 * 
 * 交互式获取机房、课程、时间等信息，进行冲突检测后创建排课记录。
 * 
 * @return SUCCESS 成功
 * @return ERROR_NOT_FOUND 机房或课程不存在
 * @return ERROR_CONFLICT 时间冲突
 * @return FAILURE 操作失败
 */
int logic_add_schedule(void);

/**
 * @brief 列出所有排课（带详情）
 * @return SUCCESS 成功
 * @return FAILURE 查询失败
 */
int logic_list_schedules(void);

/**
 * @brief 查询排课（多条件）
 * 
 * 支持按机房、教师、星期等条件查询排课记录。
 * 
 * @return SUCCESS 成功
 * @return FAILURE 查询失败
 */
int logic_query_schedules(void);

/**
 * @brief 更新排课时间
 * @return SUCCESS 成功
 * @return ERROR_NOT_FOUND 排课不存在
 * @return ERROR_CONFLICT 时间冲突
 * @return FAILURE 更新失败
 */
int logic_update_schedule(void);

/**
 * @brief 删除排课
 * @return SUCCESS 成功
 * @return ERROR_NOT_FOUND 排课不存在
 * @return FAILURE 删除失败
 */
int logic_delete_schedule(void);

/*============================================================================
 * 统计功能
 *============================================================================*/

/**
 * @brief 显示数据统计
 * 
 * 统计并显示机房数量、教师数量、课程数量、排课数量、使用率等信息。
 * 
 * @return SUCCESS 成功
 */
int logic_show_statistics(void);

#endif /* LOGIC_H */
