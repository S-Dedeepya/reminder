#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
void alarm();
int set_target(time_t *target_time,struct tm *target);
void studetails();
void logfiles();
struct node
{
    char roll_no[50];
    char name[50];
    int year;
    int semester;
};
void save_target_time(const char *filename, struct tm *target) {
    FILE *fp = fopen(filename, "w");
    if (fp != NULL) {
        fprintf(fp, "%d %d %d %d %d %d",(target->tm_year+1900), (target->tm_mon+1), target->tm_mday,target->tm_hour, target->tm_min, target->tm_sec);
        fclose(fp);
    }
}
time_t load_target_time(const char *filename) {
    FILE *fp = fopen(filename, "r");
    struct tm target = {0};
    time_t t = 0;
    int year,month;
    if (fp != NULL) {
        if (fscanf(fp, "%d %d %d %d %d %d",&year, &month, &target.tm_mday,&target.tm_hour, &target.tm_min, &target.tm_sec) == 6) {
            target.tm_year=year-1900;
            target.tm_mon= month-1;
            t = mktime(&target);
        }
        fclose(fp);
    }
    return t;
}
void studetails()
{
      // creating a file "stu_details.txt" , which is the master file,with student 1 details in it

    struct node s1;
    FILE *fp = NULL;
    fp = fopen("stu_details.txt", "w");
    if (fp == NULL)
    {
        printf("error");
        exit(1);
    }

    // storing the student 1 details in the master file

    printf("enter the roll num of student 1:");
    scanf("%s", s1.roll_no);
    printf("enter the student1  name:");
    scanf("%s", s1.name);
    printf("enter student1 year:");
    scanf("%d", &s1.year);
    printf("enter student1 semester");
    scanf("%d", &s1.semester);

    // Printing the stored student 1 data in created master file

    fprintf(fp, "Student 1 details :- \n");
    fprintf(fp, "\n");
    fprintf(fp, "Roll_no      : %s\n", s1.roll_no);
    fprintf(fp, "Student name : %s\n", s1.name);
    fprintf(fp, "Year         : %d\n", s1.year);
    fprintf(fp, "Semester     : %d\n", s1.semester);
    fprintf(fp, "\n");
    fprintf(fp, "\n");
    fclose(fp);

    // Appending the master file with Student 2 details 

    struct node s2;
    FILE *fps = NULL;
    fps = fopen("stu_details.txt", "a");
    if (fps == NULL)
    {
        printf("error");
        exit(1);
    }

    // storing the student 2 details

    printf("enter the roll num of student 2:");
    scanf("%s", s2.roll_no);
    printf("enter the student2  name:");
    scanf("%s", s2.name);
    printf("enter student2 year:");
    scanf("%d", &s2.year);
    printf("enter student2 semester");
    scanf("%d", &s2.semester);

    // Printing the stored student 2 data in the master file

    fprintf(fps, "Student 2 details :- \n");
    fprintf(fps, "\n");
    fprintf(fps, "Roll_no      : %s\n", s2.roll_no);
    fprintf(fps, "Student name : %s\n", s2.name);
    fprintf(fps, "Year         : %d\n", s2.year);
    fprintf(fps, "Semester     : %d\n", s2.semester);
    fprintf(fps, "\n");
    fprintf(fps, "\n");
    fclose(fps);

    // appending the master file with problem statement assigned to the two students

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
    fprintf(fps, "\n");
    fprintf(fps, "\n");
    fprintf(fps, "\n");
    fclose(fpsr);
}
void logfiles()
{
    char time[10];
    char date[10];
    char information[10000];
    printf("enter the date:");
    scanf("%s", date);
    printf("enter the time:");
    scanf("%s", time);
    while (getchar() != '\n');
    char filename[30];

    //creating log files dynamically with change in date and time

    snprintf(filename,sizeof(filename), "3203_3231_update_%s_%s.txt", date, time);
    FILE *log = fopen(filename, "w");
    if (log == NULL)
    {
        printf("error");
        exit(1);
    }
    fprintf(log, "%s update....:\n", date);

    // entering the information to store in log files

    printf("Enter the information you want to store in this log file:\n");
    while (1)
    {
        fgets(information, sizeof(information), stdin);
        if (strcmp(information, "\n") == 0)
            break;
        fputs(information, log);
    }
    fprintf(log, "\n");
    fprintf(log, "\n");
    fclose(log);

    // copying the data from log file to master file evrytime the log file is generated

    FILE *destination;
    char ch;
    log = fopen(filename, "r");
    if (log == NULL)
    {
        printf("error... problem here1");
        exit(1);
    }
    destination = fopen("stu_details.txt", "a");
    if (destination == NULL)
    {
        printf("error......problem here 2");
        fclose(log);
        exit(1);
    }
    while ((ch = fgetc(log)) != EOF)
    {
        fputc(ch, destination);
    }
    fclose(log);
    fclose(destination);
}
void alarm()
{
    struct tm *local;
    time_t soft_alarm;
    time_t now=time(NULL);
    local=localtime(&now);
    time_t target_time;
    struct tm target={0};
    printf("The system time is : %s",ctime(&now));
    char ch3;
    int track=0;
    while(1)
    {
        printf("Choose Y if you want to set alarm...\n");
        printf("Choose N if you want to exit...\n");
        scanf(" %c",&ch3);
        switch(ch3)
        {
            case 'Y':
            case 'y':
            if (set_target(&target_time, &target)) {
                save_target_time("target_time.txt",&target);
                track = 1;
                } else {
                printf("Target time is set incorrectly\n");
                }
            break;
            case 'N':
            case 'n':
            target_time=load_target_time("target_time.txt");
            if (target_time != 0) {
                printf("saved target time: %s", ctime(&target_time));
                track = 1;
                } else {
                printf("No saved target time found!\n");
            }
            break;
            default:
            printf("Invalid input...\n");
        }
        if(ch3=='N' || ch3 == 'n')
        break;
    }
    if(track)
    {
    double total_time=difftime(target_time,now);
    soft_alarm=(now)+(time_t)(0.8*total_time);
    if(now<soft_alarm)
    {
        printf("SOFT ALARM:\n");
        printf("DUE DATE IS ON : %s....",ctime(&target_time));
    }else if(now<target_time){
        printf("HARD ALARM:\n");
        printf("HURRY UP!!! 80 PERCENT OF TIME HAS PASSED!!");
    }else{
        printf("TIME IS UP!!!!");
    }
    }else{
        printf("Set the target time correctly");
    }
}
int set_target(time_t *target_time,struct tm *target)
{
    time_t now = time(NULL);
    struct tm *local = localtime(&now);
    int year,month,day,hour,min,second;
    printf("Enter the target year:");
    scanf(" %d",&year);
    if(year < local->tm_year+1900)
    {
        printf("Year can't be earlier\n");
        return 0;
    }else{
        target->tm_year=year-1900;
    }
    printf("Enter the target month(1-12):");
    scanf(" %d",&month);
    printf("%d",local->tm_mon);
    if (month < 1 || month > 12)
    {
        printf("Invalid month! Must be between 1 and 12.\n");
        return 0;
    }
    if (year == (local->tm_year+1900) && (month - 1) < local->tm_mon)
    {
        printf("Month can't be earlier than the current month!\n");
        return 0;
    }
    target->tm_mon = month - 1;
    printf("Enter the target day(0-31):");
    scanf(" %d",&day);
    if (day < 1 || day > 31)
    {
        printf("Invalid day! Must be between 1 and 31.\n");
        return 0;
    }
    if (year == (local->tm_year+1900) && (month - 1) == local->tm_mon && day < local->tm_mday)
    {
        printf("Day can't be earlier than the current day!\n");
        return 0;
    }
    target->tm_mday = day;
    printf("Enter the target hour(0-24):");
    scanf(" %d",&hour);
    if (hour < 0 || hour > 23)
    {
        printf("Invalid hour! Must be between 0 and 23.\n");
        return 0;
    }
    if (year == (local->tm_year+1900) && (month - 1) == local->tm_mon && day == local->tm_mday && hour < local->tm_hour)
    {
        printf("Hour can't be earlier than the current hour!\n");
        return 0;
    }
    target->tm_hour = hour;
    printf("Enter the target minute(0-59):");
    scanf(" %d",&min);
    if((min < 0 || min> 59))
    {
        printf("Invalid minute!!!\n");
        return 0;
    }
    if(min <local->tm_min && min<local->tm_min && hour==local->tm_hour && day==local->tm_mday && ((month-1)==local->tm_mon) && (year == local->tm_year+1900))
    {
        printf("Minute can't be earlier\n");
    }else{
        target->tm_min=min;
    }
    printf("Enter the target second (0-59): ");
    scanf("%d", &second);
    if (second < 0 || second > 59)
    {
        printf("Invalid second! Must be between 0 and 59.\n");
        return 0;
    }
    if (year == (local->tm_year+1900) && (month - 1) == local->tm_mon && day == local->tm_mday && hour == local->tm_hour && min == local->tm_min && second < local->tm_sec)
    {
        printf("Second can't be earlier than the current second!\n");
        return 0;
    }
    target->tm_sec = second;
    *target_time=mktime(target);
    printf("Target time: %s", ctime(target_time));
    return 1;
}
int main()
{
    char ch;
    while(1)
    {
        printf("\nChoose Y if you want to change student details....\n");
        printf("choose N if you want to create log files...\n");
        printf("choose X for alarm setting..\n");
        scanf(" %c", &ch);
        switch(ch)
        {
        case 'Y':
        case 'y':
        studetails();
        break;
        case 'N':
        case 'n':
        while ((getchar()) != '\n');
        logfiles();
        break;
        case 'X':
        case 'x': 
        break;
        default:
        printf("Invalid input...\n");
        }
        if(ch=='X' || ch == 'x') break;
    }
    alarm();
    return 0;
}