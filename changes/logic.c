#include "logic.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

const char *FILENAME = "stu_details.txt";
const char *TASKS_FILENAME = "tasks.txt";

void save_assignment_details(const Assignment *data) {
    FILE *file = fopen(FILENAME, "w");
    if (!file) { return; }
    fprintf(file, "%s\n", data->problem_statement);
    fprintf(file, "%s\n%s\n%d\n%d\n", data->student1.roll_no, data->student1.name, data->student1.year, data->student1.semester);
    fprintf(file, "%s\n%s\n%d\n%d\n", data->student2.roll_no, data->student2.name, data->student2.year, data->student2.semester);
    fclose(file);
}

Assignment load_assignment_details() {
    Assignment data = {0};
    FILE *file = fopen(FILENAME, "r");
    if (!file) { return data; }
    fgets(data.problem_statement, sizeof(data.problem_statement), file);
    data.problem_statement[strcspn(data.problem_statement, "\n")] = 0; // Remove newline
    fscanf(file, "%s\n%s\n%d\n%d\n", data.student1.roll_no, data.student1.name, &data.student1.year, &data.student1.semester);
    fscanf(file, "%s\n%s\n%d\n%d\n", data.student2.roll_no, data.student2.name, &data.student2.year, &data.student2.semester);
    fclose(file);
    return data;
}

void save_tasks(const AllTasks *all_tasks) {
    FILE *file = fopen(TASKS_FILENAME, "w");
    if (!file) { return; }
    for (int i = 0; i < 3; i++) {
        fprintf(file, "%s\n%s\n%s\n%d\n", all_tasks->tasks[i].name, all_tasks->tasks[i].from_date, all_tasks->tasks[i].to_date, all_tasks->tasks[i].is_approved);
    }
    fclose(file);
}

AllTasks load_tasks() {
    AllTasks all_tasks = {0};
    FILE *file = fopen(TASKS_FILENAME, "r");
    if (!file) { return all_tasks; }
    for (int i = 0; i < 3; i++) {
        int approved_status = 0;
        fgets(all_tasks.tasks[i].name, sizeof(all_tasks.tasks[i].name), file);
        all_tasks.tasks[i].name[strcspn(all_tasks.tasks[i].name, "\n")] = 0;
        fscanf(file, "%s\n%s\n%d\n", all_tasks.tasks[i].from_date, all_tasks.tasks[i].to_date, &approved_status);
        all_tasks.tasks[i].is_approved = approved_status;
    }
    fclose(file);
    return all_tasks;
}

char *generate_master_log_text() {
    Assignment details = load_assignment_details();
    AllTasks tasks = load_tasks();

    // Allocate a large buffer for the log text
    char* log_text = (char*)malloc(4096 * sizeof(char));
    if (log_text == NULL) return NULL;

    // Format the assignment details
    sprintf(log_text, "PROJECT SUMMARY\n====================\n\n");
    sprintf(log_text + strlen(log_text), "Problem Statement:\n%s\n\n", details.problem_statement);
    sprintf(log_text + strlen(log_text), "Student 1: %s (%s)\n", details.student1.name, details.student1.roll_no);
    sprintf(log_text + strlen(log_text), "Student 2: %s (%s)\n\n", details.student2.name, details.student2.roll_no);
    sprintf(log_text + strlen(log_text), "TASKS\n====================\n\n");

    // Format the tasks
    for (int i = 0; i < 3; i++) {
        sprintf(log_text + strlen(log_text), "Task %d: %s\n", i + 1, tasks.tasks[i].name);
        sprintf(log_text + strlen(log_text), "  - Duration: %s to %s\n", tasks.tasks[i].from_date, tasks.tasks[i].to_date);
        sprintf(log_text + strlen(log_text), "  - Mentor Approval: %s\n\n", tasks.tasks[i].is_approved ? "Approved" : "Not Approved");
    }
    return log_text;
}

// Alarm functions remain unchanged...
void set_alarm_time(const struct tm *target_tm) { /*...*/ }
time_t get_alarm_time() { /*...*/ }
AlarmStatus check_alarm_status() { /*...*/ }