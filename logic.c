#include "logic.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

const char *FILENAME = "stu_details.txt";
const char *TASKS_FILENAME = "tasks.txt";

void push_state(HistoryNode** stack, const Assignment* data) {
    HistoryNode* new_node = (HistoryNode*)malloc(sizeof(HistoryNode));
    if (!new_node) { return; }
    new_node->data = *data;
    new_node->next = *stack;
    *stack = new_node;
}

Assignment pop_state(HistoryNode** stack) {
    if (*stack == NULL) { return (Assignment){0}; }
    HistoryNode* temp = *stack;
    Assignment data = temp->data;
    *stack = temp->next;
    free(temp);
    return data;
}

void clear_stack(HistoryNode** stack) {
    while (*stack != NULL) {
        HistoryNode* temp = *stack;
        *stack = (*stack)->next;
        free(temp);
    }
}

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
    data.problem_statement[strcspn(data.problem_statement, "\n")] = 0;
    fscanf(file, "%s\n%s\n%d\n%d\n", data.student1.roll_no, data.student1.name, &data.student1.year, &data.student1.semester);
    fscanf(file, "%s\n%s\n%d\n%d\n", data.student2.roll_no, data.student2.name, &data.student2.year, &data.student2.semester);
    fclose(file);
    return data;
}

void overwrite_all_tasks(const AllTasks *all_tasks) {
    FILE *file = fopen(TASKS_FILENAME, "w");
    if (!file) { return; }
    for (int i = 0; i < all_tasks->task_count; i++) {
        fprintf(file, "%s\n%s\n%s\n%d\n%s\n", 
                all_tasks->tasks[i].name, 
                all_tasks->tasks[i].from_date, 
                all_tasks->tasks[i].to_date, 
                all_tasks->tasks[i].is_approved,
                all_tasks->tasks[i].approval_date);
    }
    fclose(file);
}

AllTasks load_tasks() {
    AllTasks all_tasks = { .tasks = NULL, .task_count = 0 };
    FILE *file = fopen(TASKS_FILENAME, "r");
    if (!file) { return all_tasks; }
    Task temp_task;
    char name_buf[100];
    while (fgets(name_buf, sizeof(name_buf), file) != NULL) {
        name_buf[strcspn(name_buf, "\n")] = 0;
        strcpy(temp_task.name, name_buf);
        char from_buf[20], to_buf[20], approved_buf[5], approval_date_buf[30];
        if (fgets(from_buf, sizeof(from_buf), file) == NULL) break;
        from_buf[strcspn(from_buf, "\n")] = 0;
        if (fgets(to_buf, sizeof(to_buf), file) == NULL) break;
        to_buf[strcspn(to_buf, "\n")] = 0;
        if (fgets(approved_buf, sizeof(approved_buf), file) == NULL) break;
        if (fgets(approval_date_buf, sizeof(approval_date_buf), file) == NULL) break;
        approval_date_buf[strcspn(approval_date_buf, "\n")] = 0;
        strcpy(temp_task.from_date, from_buf);
        strcpy(temp_task.to_date, to_buf);
        temp_task.is_approved = atoi(approved_buf);
        strcpy(temp_task.approval_date, approval_date_buf);
        all_tasks.task_count++;
        all_tasks.tasks = realloc(all_tasks.tasks, all_tasks.task_count * sizeof(Task));
        all_tasks.tasks[all_tasks.task_count - 1] = temp_task;
    }
    fclose(file);
    return all_tasks;
}

void free_all_tasks(AllTasks* all_tasks) {
    if (all_tasks && all_tasks->tasks) {
        free(all_tasks->tasks);
        all_tasks->tasks = NULL;
        all_tasks->task_count = 0;
    }
}

char *generate_master_log_text() {
    Assignment details = load_assignment_details();
    AllTasks tasks = load_tasks();
    char* log_text = (char*)malloc(8192 * sizeof(char));
    if (log_text == NULL) { free_all_tasks(&tasks); return NULL; }
    *log_text = '\0';
    sprintf(log_text, "PROJECT SUMMARY\n====================\n\n");
    sprintf(log_text + strlen(log_text), "Problem Statement:\n%s\n\n", details.problem_statement);
    sprintf(log_text + strlen(log_text), "Student 1: %s (%s)\n", details.student1.name, details.student1.roll_no);
    sprintf(log_text + strlen(log_text), "Student 2: %s (%s)\n\n", details.student2.name, details.student2.roll_no);
    sprintf(log_text + strlen(log_text), "TASKS\n====================\n\n");
    for (int i = 0; i < tasks.task_count; i++) {
        sprintf(log_text + strlen(log_text), "Task %d: %s\n", i + 1, tasks.tasks[i].name);
        sprintf(log_text + strlen(log_text), "  - Duration: %s to %s\n", tasks.tasks[i].from_date, tasks.tasks[i].to_date);
        if (tasks.tasks[i].is_approved) {
            sprintf(log_text + strlen(log_text), "  - Mentor Approval: Approved on %s\n\n", tasks.tasks[i].approval_date);
        } else {
            sprintf(log_text + strlen(log_text), "  - Mentor Approval: Not Approved\n\n");
        }
    }
    free_all_tasks(&tasks);
    return log_text;
}