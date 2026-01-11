#include <stdio.h>
#include <stdlib.h>
struct node
{
    char roll_no[50];
    char name[50];
    int year;
    int semester;
};
int main()
{
    struct node s1;
    FILE *fp = NULL;
    fp = fopen("stu_details.txt", "w");
    if (fp == NULL)
    {
        printf("error");
        exit(1);
    }
    printf("enter the roll num of student 1:");
    scanf("%s", &s1.roll_no);
    printf("enter the student1  name:");
    scanf("%s", &s1.name);
    printf("enter student1 year:");
    scanf("%d", &s1.year);
    printf("enter student1 semester");
    scanf("%d", &s1.semester);
    fprintf(fp, "Student 1 details :- \n");
    fprintf(fp, "\n");
    fprintf(fp, "Roll_no      : %s\n", s1.roll_no);
    fprintf(fp, "Student name : %s\n", s1.name);
    fprintf(fp, "Year         : %d\n", s1.year);
    fprintf(fp, "Semester     : %d\n", s1.semester);
    fprintf(fp, "\n");
    fprintf(fp, "\n");
    fclose(fp);
    struct node s2;
    FILE *fps = NULL;
    fps = fopen("stu_details.txt", "a");
    if (fps == NULL)
    {
        printf("error");
        exit(1);
    }
    printf("enter the roll num of student 2:");
    scanf("%s", &s2.roll_no);
    printf("enter the student2  name:");
    scanf("%s", &s2.name);
    printf("enter student2 year:");
    scanf("%d", &s2.year);
    printf("enter student2 semester");
    scanf("%d", &s2.semester);
    fprintf(fps, "Student 2 details :- \n");
    fprintf(fps, "\n");
    fprintf(fps, "Roll_no      : %s\n", s2.roll_no);
    fprintf(fps, "Student name : %s\n", s2.name);
    fprintf(fps, "Year         : %d\n", s2.year);
    fprintf(fps, "Semester     : %d\n", s2.semester);
    fprintf(fps, "\n");
    fprintf(fps, "\n");
    fclose(fps);
    char problem_statement[1000];
    FILE *fpsr = NULL;
    fpsr = fopen("stu_details.txt", "a");
    if (fpsr == NULL)
    {
        printf("error");
        exit(1);
    }
    printf("enter the problem statement:");
    scanf(" %[^\n]", problem_statement);
    fprintf(fpsr, "Problem_statement :\n");

    fputs(problem_statement, fpsr);
    fclose(fpsr);
    return 0;
}