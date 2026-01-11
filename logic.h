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

typedef struct HistoryNode {
    Assignment data;
    struct HistoryNode* next;
} HistoryNode;

typedef struct {
    char name[100];
    char from_date[20];
    char to_date[20];
    bool is_approved;
    char approval_date[30];
} Task;

typedef struct {
    Task* tasks;
    int task_count;
} AllTasks;

void save_assignment_details(const Assignment *assignment_data);
Assignment load_assignment_details();
void overwrite_all_tasks(const AllTasks *all_tasks);
AllTasks load_tasks();
void free_all_tasks(AllTasks* all_tasks);
char *generate_master_log_text();

void push_state(HistoryNode** stack, const Assignment* data);
Assignment pop_state(HistoryNode** stack);
void clear_stack(HistoryNode** stack);

#endif // LOGIC_H