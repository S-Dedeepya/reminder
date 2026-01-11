#include "logic.h"  
#include <stdio.h>
#include <string.h>
#include <time.h>

const char* FILENAME = "stu_details.txt";

void save_assignment_details(const Assignment *assignment_data) {
    FILE *file = fopen(FILENAME, "w");
    if (file == NULL) {
        perror("Error opening file for writing");
        return;
    }
    fprintf(file, "Problem Statement:\n%s\n", assignment_data->problem_statement);
    fprintf(file, "----\n");
    fprintf(file, "Student 1 Roll No: %s\n", assignment_data->student1.roll_no);
    fprintf(file, "Student 1 Name: %s\n", assignment_data->student1.name);
    fprintf(file, "Student 1 Year: %d\n", assignment_data->student1.year);
    fprintf(file, "Student 1 Semester: %d\n", assignment_data->student1.semester);
    fprintf(file, "----\n");
    fprintf(file, "Student 2 Roll No: %s\n", assignment_data->student2.roll_no);
    fprintf(file, "Student 2 Name: %s\n", assignment_data->student2.name);
    fprintf(file, "Student 2 Year: %d\n", assignment_data->student2.year);
    fprintf(file, "Student 2 Semester: %d\n", assignment_data->student2.semester);
    fclose(file);
    printf("Details saved successfully to %s\n", FILENAME);
}

Assignment load_assignment_details() {
    Assignment loaded_data = {0};
    FILE *file = fopen(FILENAME, "r");
    if (file == NULL) {
        printf("No existing file found. Starting fresh.\n");
        return loaded_data;
    }
    fscanf(file, "Problem Statement:\n%2047[^\n]\n", loaded_data.problem_statement);
    fscanf(file, "----\n");
    fscanf(file, "Student 1 Roll No: %49[^\n]\n", loaded_data.student1.roll_no);
    fscanf(file, "Student 1 Name: %99[^\n]\n", loaded_data.student1.name);
    fscanf(file, "Student 1 Year: %d\n", &loaded_data.student1.year);
    fscanf(file, "Student 1 Semester: %d\n", &loaded_data.student1.semester);
    fscanf(file, "----\n");
    fscanf(file, "Student 2 Roll No: %49[^\n]\n", loaded_data.student2.roll_no);
    fscanf(file, "Student 2 Name: %99[^\n]\n", loaded_data.student2.name);
    fscanf(file, "Student 2 Year: %d\n", &loaded_data.student2.year);
    fscanf(file, "Student 2 Semester: %d\n", &loaded_data.student2.semester);
    fclose(file);
    printf("Details loaded successfully from %s\n", FILENAME);
    return loaded_data;
}

void add_log_entry(const char *log_text) {
    char filename[100];
    char timestamp[100];
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(filename, sizeof(filename), "Project_Log_%Y-%m-%d.txt", t);
    FILE *file = fopen(filename, "a");
    if (file == NULL) {
        perror("Error opening log file");
        return;
    }
    strftime(timestamp, sizeof(timestamp), "[%H:%M:%S]", t);
    fprintf(file, "%s %s\n\n", timestamp, log_text);
    fclose(file);
    printf("Log entry added to %s\n", filename);
}

static void save_target_tm_to_file(const struct tm *target) {
    FILE *fp = fopen("target_time.txt", "w");
    if (fp != NULL) {
        time_t t = mktime((struct tm *)target);
        fprintf(fp, "%ld", t);
        fclose(fp);
    }
}

void set_alarm_time(const struct tm *target_tm) {
    save_target_tm_to_file(target_tm);
    printf("Alarm time has been set.\n");
}

time_t get_alarm_time() {
    FILE *fp = fopen("target_time.txt", "r");
    time_t t = 0;
    if (fp != NULL) {
        if (fscanf(fp, "%ld", &t) != 1) {
            t = 0;
        }
        fclose(fp);
    }
    return t;
}

AlarmStatus check_alarm_status() {
    time_t now = time(NULL);
    time_t target_time = get_alarm_time();

    if (target_time == 0) {
        return ALARM_NOT_SET;
    }

    if (now > target_time) {
        return ALARM_EXPIRED;
    }

    double total_time_remaining = difftime(target_time, now);

    if (total_time_remaining <= (8 * 60 * 60)) { // 8 hours
        return ALARM_HARD;
    } else if (total_time_remaining <= (2 * 24 * 60 * 60)) { // 2 days
        return ALARM_SOFT;
    } else {
        return ALARM_OK;
    }
}
