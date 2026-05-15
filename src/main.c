/**
 * @file main.c
 * @brief 主程序入口
 * @author Computer Room Scheduling System
 * @version 1.0
 * @date 2024
 * 
 * @description
 * 本文件是机房排课系统的主入口，负责：
 * 1. 初始化控制台环境（UTF-8 编码）
 * 2. 初始化数据库连接
 * 3. 运行主菜单循环
 * 4. 清理资源并退出
 */

#include <stdio.h>
#include <stdlib.h>
#include "types.h"
#include "db.h"
#include "ui.h"
#include "logic.h"

/* Windows 平台特定头文件 */
#ifdef _WIN32
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#endif

/**
 * @brief 配置控制台为 UTF-8 编码
 * 
 * 在 Windows 平台上设置控制台输入输出编码为 UTF-8，
 * 并启用虚拟终端处理以支持 ANSI 转义序列。
 * 
 * @note 此函数仅在 Windows 上有效，其他平台为空操作
 */
static void setup_console_utf8(void) {
#ifdef _WIN32
    /* 设置控制台输入输出编码为 UTF-8 */
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    
    /* 启用虚拟终端处理（支持 ANSI 转义序列） */
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD mode = 0;
        if (GetConsoleMode(hOut, &mode)) {
            mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            SetConsoleMode(hOut, mode);
        }
    }
    
    /* 配置输入句柄 */
    HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
    if (hIn != INVALID_HANDLE_VALUE) {
        DWORD mode = 0;
        if (GetConsoleMode(hIn, &mode)) {
            mode |= ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT;
            SetConsoleMode(hIn, mode);
        }
    }
#endif
}

/**
 * @brief 程序主入口
 * 
 * 执行以下步骤：
 * 1. 配置控制台 UTF-8 编码
 * 2. 显示欢迎信息
 * 3. 初始化数据库
 * 4. 运行主菜单循环
 * 5. 关闭数据库并退出
 * 
 * @return 0 正常退出
 * @return 1 数据库初始化失败
 */
int main(void) {
    /* 配置控制台环境 */
    setup_console_utf8();
    
    /* 显示欢迎信息 */
    printf("\n");
    printf("========================================\n");
    printf("      欢迎使用机房排课系统\n");
    printf("========================================\n");

    /* 初始化数据库 */
    if (db_init(DB_PATH) != SUCCESS) {
        printf("[错误] 数据库初始化失败\n");
        return 1;
    }

    printf("[提示] 数据库初始化成功\n");

    /* 主循环 */
    int running = 1;
    while (running) {
        ui_show_main_menu();
        int main_choice = ui_get_main_choice();

        switch (main_choice) {
            case 1: {
                /* 基础数据管理 */
                int back = 0;
                while (!back) {
                    ui_show_data_menu();
                    int data_choice = ui_get_data_choice();

                    switch (data_choice) {
                        case 1: {
                            /* 机房管理 */
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
                            /* 课程管理 */
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
                            /* 教师管理 */
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
                /* 排课管理 */
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
                /* 数据统计 */
                logic_show_statistics();
                ui_pause();
                break;

            case 0:
                /* 退出系统 */
                running = 0;
                ui_show_success("感谢使用，再见！");
                break;
        }
    }

    /* 清理资源 */
    db_close();
    return 0;
}
