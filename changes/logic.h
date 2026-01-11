#ifndef LOGIC_H
#define LOGIC_H

#include <time.h>
#include <stdbool.h>

typedef struct {
    char roll_no[50];
    char name[100];
    int year;
    int semester;
} Student;

typedef struct {
    char problem_statement[2048];
    Student student1;
    Student student2;
} Assignment;

typedef struct {
    char name[100];
    char from_date[20];
    char to_date[20];
    bool is_approved;
} Task;

typedef struct {
    Task tasks[3];
} AllTasks;

typedef enum {
    ALARM_NOT_SET, ALARM_OK, ALARM_SOFT, ALARM_HARD, ALARM_EXPIRED
} AlarmStatus;

void save_assignment_details(const Assignment *assignment_data);
Assignment load_assignment_details();
void save_tasks(const AllTasks *all_tasks);
AllTasks load_tasks();
char *generate_master_log_text();
void set_alarm_time(const struct tm *target_tm);
time_t get_alarm_time();
AlarmStatus check_alarm_status();

#endif // LOGIC_H