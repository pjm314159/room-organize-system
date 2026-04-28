#include <stdio.h>
#include <stdlib.h>
#include "types.h"
#include "db.h"
#include "ui.h"
#include "logic.h"

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#endif

static void setup_console_utf8(void) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD mode = 0;
        if (GetConsoleMode(hOut, &mode)) {
            mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            SetConsoleMode(hOut, mode);
        }
    }
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

int main(void) {
    setup_console_utf8();
    
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
