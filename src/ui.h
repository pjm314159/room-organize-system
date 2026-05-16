/**
 * @file ui.h
 * @brief 用户界面模块头文件
 * @author Computer Room Scheduling System
 * @version 1.0
 * @date 2026
 * 
 * @description
 * 本文件声明了所有用户界面相关函数，包括菜单显示、用户输入处理、
 * 数据展示等功能。使用 ANSI 转义序列实现终端清屏和刷新。
 */

#ifndef UI_H
#define UI_H

#include "types.h"

/*============================================================================
 * 终端控制函数
 *============================================================================*/

/**
 * @brief 清屏并移动光标到左上角
 * 
 * 使用 ANSI 转义序列 \033[2J\033[H 实现清屏。
 */
void ui_clear_screen(void);

/**
 * @brief 清除当前行
 * 
 * 使用 ANSI 转义序列 \033[2K\r 清除当前行内容。
 */
void ui_clear_line(void);

/*============================================================================
 * 菜单显示函数
 *============================================================================*/

/**
 * @brief 显示主菜单
 * 
 * 显示系统主菜单选项，包括基础数据管理、排课管理、数据统计等。
 */
void ui_show_main_menu(void);

/**
 * @brief 获取主菜单用户选择
 * @return 用户输入的菜单选项（0-3）
 */
int ui_get_main_choice(void);

/**
 * @brief 显示基础数据管理子菜单
 */
void ui_show_data_menu(void);

/**
 * @brief 获取数据管理菜单用户选择
 * @return 用户输入的菜单选项（0-3）
 */
int ui_get_data_choice(void);

/**
 * @brief 显示机房管理子菜单
 */
void ui_show_room_menu(void);

/**
 * @brief 获取机房管理菜单用户选择
 * @return 用户输入的菜单选项（0-4）
 */
int ui_get_room_choice(void);

/**
 * @brief 显示课程管理子菜单
 */
void ui_show_course_menu(void);

/**
 * @brief 获取课程管理菜单用户选择
 * @return 用户输入的菜单选项（0-4）
 */
int ui_get_course_choice(void);

/**
 * @brief 显示教师管理子菜单
 */
void ui_show_teacher_menu(void);

/**
 * @brief 获取教师管理菜单用户选择
 * @return 用户输入的菜单选项（0-4）
 */
int ui_get_teacher_choice(void);

/**
 * @brief 显示排课管理子菜单
 */
void ui_show_schedule_menu(void);

/**
 * @brief 获取排课管理菜单用户选择
 * @return 用户输入的菜单选项（0-4）
 */
int ui_get_schedule_choice(void);

/*============================================================================
 * 用户输入函数
 *============================================================================*/

/**
 * @brief 读取用户输入的字符串
 * @param prompt 提示信息
 * @param buffer 存储输入的缓冲区
 * @param max_len 缓冲区最大长度
 */
void ui_input_string(const char *prompt, char *buffer, int max_len);

/**
 * @brief 读取用户输入的整数
 * @param prompt 提示信息
 * @return 用户输入的整数值
 */
int ui_input_int(const char *prompt);

/**
 * @brief 读取用户输入的整数（带范围验证）
 * @param prompt 提示信息
 * @param min 最小值
 * @param max 最大值
 * @return 用户输入的整数值（在 [min, max] 范围内）
 */
int ui_input_int_range(const char *prompt, int min, int max);

/**
 * @brief 读取用户确认输入
 * @param prompt 提示信息
 * @return 'Y' 表示确认，'N' 表示取消
 */
char ui_input_confirm(const char *prompt);

/*============================================================================
 * 信息提示函数
 *============================================================================*/

/**
 * @brief 显示标题
 * @param title 标题文本
 */
void ui_show_title(const char *title);

/**
 * @brief 显示分隔线
 */
void ui_show_divider(void);

/**
 * @brief 显示成功消息
 * @param message 消息内容
 */
void ui_show_success(const char *message);

/**
 * @brief 显示错误消息
 * @param message 消息内容
 */
void ui_show_error(const char *message);

/**
 * @brief 显示警告消息
 * @param message 消息内容
 */
void ui_show_warning(const char *message);

/**
 * @brief 暂停等待用户按键
 */
void ui_pause(void);

/*============================================================================
 * 数据展示函数 - 机房
 *============================================================================*/

/**
 * @brief 显示单个机房信息
 * @param room 机房数据指针
 */
void ui_display_room(ComputerRoom *room);

/**
 * @brief 显示机房列表
 * @param rooms 机房数组指针
 * @param count 数组元素数量
 */
void ui_display_rooms(ComputerRoom *rooms, int count);

/*============================================================================
 * 数据展示函数 - 教师
 *============================================================================*/

/**
 * @brief 显示单个教师信息
 * @param teacher 教师数据指针
 */
void ui_display_teacher(Teacher *teacher);

/**
 * @brief 显示教师列表
 * @param teachers 教师数组指针
 * @param count 数组元素数量
 */
void ui_display_teachers(Teacher *teachers, int count);

/*============================================================================
 * 数据展示函数 - 课程
 *============================================================================*/

/**
 * @brief 显示单个课程信息
 * @param course 课程数据指针
 */
void ui_display_course(Course *course);

/**
 * @brief 显示课程列表
 * @param courses 课程数组指针
 * @param count 数组元素数量
 */
void ui_display_courses(Course *courses, int count);

/*============================================================================
 * 数据展示函数 - 排课
 *============================================================================*/

/**
 * @brief 显示单个排课信息
 * @param schedule 排课数据指针
 */
void ui_display_schedule(Schedule *schedule);

/**
 * @brief 显示单个排课详情（带关联信息）
 * @param detail 排课详情数据指针
 */
void ui_display_schedule_detail(ScheduleDetail *detail);

/**
 * @brief 显示排课列表
 * @param schedules 排课数组指针
 * @param count 数组元素数量
 */
void ui_display_schedules(Schedule *schedules, int count);

/**
 * @brief 显示排课详情列表
 * @param details 排课详情数组指针
 * @param count 数组元素数量
 */
void ui_display_schedule_details(ScheduleDetail *details, int count);

/*============================================================================
 * 辅助函数
 *============================================================================*/

/**
 * @brief 获取星期名称
 * @param day 星期几（1-7）
 * @return 星期名称字符串（如 "周一"）
 */
const char* ui_get_day_name(int day);

#endif /* UI_H */
