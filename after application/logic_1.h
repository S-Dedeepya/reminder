#ifndef LOGIC_H
#define LOGIC_H

#include <time.h>

// A structure to hold all the data for one student
typedef struct {
    char roll_no[50];
    char name[100];
    int year;
    int semester;
} Student;

// A structure to hold the entire assignment's details
typedef struct {
    char problem_statement[2048];
    Student student1;
    Student student2;
} Assignment;

// An enum to represent the alarm's state
typedef enum {
    ALARM_NOT_SET,
    ALARM_OK,
    ALARM_SOFT,
    ALARM_HARD,
    ALARM_EXPIRED
} AlarmStatus;

// --- Assignment Details Functions ---
void save_assignment_details(const Assignment *assignment_data);
Assignment load_assignment_details();

// --- Log File Function ---
void add_log_entry(const char *log_text);

// --- Alarm Functions ---
void set_alarm_time(const struct tm *target_tm);
time_t get_alarm_time();
AlarmStatus check_alarm_status();

#endif // LOGIC_H
