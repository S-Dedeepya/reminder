#include <gtk/gtk.h>
#include "logic.h"
#include <string.h>
#include <ctype.h>

// --- Global Widget Pointers ---
GtkWidget *ps_view, *s1_roll_entry, *s1_name_entry, *s1_year_spin, *s1_sem_spin;
GtkWidget *s2_roll_entry, *s2_name_entry, *s2_year_spin, *s2_sem_spin;
GtkWidget *log_view;
GtkWidget *alarm_calendar, *alarm_hour, *alarm_minute, *alarm_second, *alarm_status_label;
GtkWidget *save_button, *edit_details_button;
GtkWidget *task1_name_entry, *task1_from_entry, *task1_to_entry, *task1_approval_check, *task1_save_button;
GtkWidget *task2_name_entry, *task2_from_entry, *task2_to_entry, *task2_approval_check, *task2_save_button;
GtkWidget *task3_name_entry, *task3_from_entry, *task3_to_entry, *task3_approval_check, *task3_save_button;

// --- Forward Declarations for Functions ---
void refresh_log_view();
void on_date_entry_changed(GtkEditable *editable, gpointer user_data);
static void on_set_alarm_clicked(GtkButton *button, gpointer user_data);

// --- Helper & Callback Functions ---
void set_details_editable(gboolean editable) {
    gtk_text_view_set_editable(GTK_TEXT_VIEW(ps_view), editable);
    gtk_editable_set_editable(GTK_EDITABLE(s1_roll_entry), editable);
    gtk_editable_set_editable(GTK_EDITABLE(s1_name_entry), editable);
    gtk_editable_set_editable(GTK_EDITABLE(s2_roll_entry), editable);
    gtk_editable_set_editable(GTK_EDITABLE(s2_name_entry), editable);
    gtk_widget_set_sensitive(s1_year_spin, editable);
    gtk_widget_set_sensitive(s1_sem_spin, editable);
    gtk_widget_set_sensitive(s2_year_spin, editable);
    gtk_widget_set_sensitive(s2_sem_spin, editable);
    gtk_widget_set_visible(save_button, editable);
    gtk_widget_set_visible(edit_details_button, !editable);
}

void load_tasks_into_editor() {
    AllTasks all_tasks = load_tasks();
    gtk_editable_set_text(GTK_EDITABLE(task1_name_entry), all_tasks.tasks[0].name);
    gtk_editable_set_text(GTK_EDITABLE(task1_from_entry), all_tasks.tasks[0].from_date);
    gtk_editable_set_text(GTK_EDITABLE(task1_to_entry), all_tasks.tasks[0].to_date);
    gtk_check_button_set_active(GTK_CHECK_BUTTON(task1_approval_check), all_tasks.tasks[0].is_approved);
    gtk_editable_set_text(GTK_EDITABLE(task2_name_entry), all_tasks.tasks[1].name);
    gtk_editable_set_text(GTK_EDITABLE(task2_from_entry), all_tasks.tasks[1].from_date);
    gtk_editable_set_text(GTK_EDITABLE(task2_to_entry), all_tasks.tasks[1].to_date);
    gtk_check_button_set_active(GTK_CHECK_BUTTON(task2_approval_check), all_tasks.tasks[1].is_approved);
    gtk_editable_set_text(GTK_EDITABLE(task3_name_entry), all_tasks.tasks[2].name);
    gtk_editable_set_text(GTK_EDITABLE(task3_from_entry), all_tasks.tasks[2].from_date);
    gtk_editable_set_text(GTK_EDITABLE(task3_to_entry), all_tasks.tasks[2].to_date);
    gtk_check_button_set_active(GTK_CHECK_BUTTON(task3_approval_check), all_tasks.tasks[2].is_approved);
}

void load_details_into_editor() {
    Assignment data = load_assignment_details();
    GtkTextBuffer *ps_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(ps_view));
    gtk_text_buffer_set_text(ps_buffer, data.problem_statement, -1);
    gtk_editable_set_text(GTK_EDITABLE(s1_roll_entry), data.student1.roll_no);
    gtk_editable_set_text(GTK_EDITABLE(s1_name_entry), data.student1.name);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(s1_year_spin), data.student1.year);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(s1_sem_spin), data.student1.semester);
    gtk_editable_set_text(GTK_EDITABLE(s2_roll_entry), data.student2.roll_no);
    gtk_editable_set_text(GTK_EDITABLE(s2_name_entry), data.student2.name);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(s2_year_spin), data.student2.year);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(s2_sem_spin), data.student2.semester);
}

void refresh_log_view() {
    char *log_content = generate_master_log_text();
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(log_view));
    gtk_text_buffer_set_text(buffer, log_content, -1);
    free(log_content);
}

void refresh_alarm_label() {
    if (alarm_status_label == NULL) return;
    AlarmStatus status = check_alarm_status();
    time_t target_time = get_alarm_time();
    char buffer[128];
    const char* classes[] = {"ok", "soft", "hard", "expired"};
    for(int i=0; i<4; i++) gtk_widget_remove_css_class(alarm_status_label, classes[i]);

    switch (status) {
        case ALARM_NOT_SET: strcpy(buffer, "Status: No alarm has been set."); break;
        case ALARM_OK: strftime(buffer, sizeof(buffer), "Status: OK. Deadline is %Y-%m-%d %H:%M:%S", localtime(&target_time)); gtk_widget_add_css_class(alarm_status_label, "ok"); break;
        case ALARM_SOFT: strftime(buffer, sizeof(buffer), "SOFT ALARM! Deadline is %Y-%m-%d %H:%M:%S", localtime(&target_time)); gtk_widget_add_css_class(alarm_status_label, "soft"); break;
        case ALARM_HARD: strcpy(buffer, "!!! HARD ALARM! Less than 8 hours remaining !!!"); gtk_widget_add_css_class(alarm_status_label, "hard"); break;
        case ALARM_EXPIRED: strcpy(buffer, "!!! TIME IS UP !!! The deadline has passed."); gtk_widget_add_css_class(alarm_status_label, "expired"); break;
    }
    gtk_label_set_text(GTK_LABEL(alarm_status_label), buffer);
}

static gboolean update_alarm_on_timer(gpointer user_data) {
    refresh_alarm_label();
    return G_SOURCE_CONTINUE;
}

void on_date_entry_changed(GtkEditable *editable, gpointer user_data) {
    g_signal_handlers_block_by_func(editable, G_CALLBACK(on_date_entry_changed), user_data);
    const char *text = gtk_editable_get_text(editable);
    char clean_text[20] = {0};
    int j = 0;
    for (int i = 0; text[i] != '\0' && j < 10; i++) {
        if (isdigit(text[i])) {
            clean_text[j++] = text[i];
            if (j == 2 || j == 5) {
                clean_text[j++] = '-';
            }
        }
    }
    gtk_editable_set_text(editable, clean_text);
    gtk_editable_set_position(editable, -1);
    g_signal_handlers_unblock_by_func(editable, G_CALLBACK(on_date_entry_changed), user_data);
}

void save_single_task(int task_index) {
    AllTasks all_tasks = load_tasks();
    GtkWidget *name_entry, *from_entry, *to_entry, *approval_check;

    if (task_index == 0) {
        name_entry = task1_name_entry; from_entry = task1_from_entry; to_entry = task1_to_entry; approval_check = task1_approval_check;
    } else if (task_index == 1) {
        name_entry = task2_name_entry; from_entry = task2_from_entry; to_entry = task2_to_entry; approval_check = task2_approval_check;
    } else {
        name_entry = task3_name_entry; from_entry = task3_from_entry; to_entry = task3_to_entry; approval_check = task3_approval_check;
    }

    strncpy(all_tasks.tasks[task_index].name, gtk_editable_get_text(GTK_EDITABLE(name_entry)), 99);
    strncpy(all_tasks.tasks[task_index].from_date, gtk_editable_get_text(GTK_EDITABLE(from_entry)), 19);
    strncpy(all_tasks.tasks[task_index].to_date, gtk_editable_get_text(GTK_EDITABLE(to_entry)), 19);
    all_tasks.tasks[task_index].is_approved = gtk_check_button_get_active(GTK_CHECK_BUTTON(approval_check));

    save_tasks(&all_tasks);
    refresh_log_view();
}

static void on_task1_save_clicked(GtkButton *b, gpointer user_data) { save_single_task(0); }
static void on_task2_save_clicked(GtkButton *b, gpointer user_data) { save_single_task(1); }
static void on_task3_save_clicked(GtkButton *b, gpointer user_data) { save_single_task(2); }

static void on_save_clicked(GtkButton *button, gpointer user_data) {
    Assignment data = {0};
    GtkTextBuffer *ps_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(ps_view));
    GtkTextIter start, end;
    gtk_text_buffer_get_start_iter(ps_buffer, &start); gtk_text_buffer_get_end_iter(ps_buffer, &end);
    char *ps_text = gtk_text_buffer_get_text(ps_buffer, &start, &end, FALSE);
    strncpy(data.problem_statement, ps_text, sizeof(data.problem_statement) - 1);
    g_free(ps_text);
    strncpy(data.student1.roll_no, gtk_editable_get_text(GTK_EDITABLE(s1_roll_entry)), sizeof(data.student1.roll_no) - 1);
    strncpy(data.student1.name, gtk_editable_get_text(GTK_EDITABLE(s1_name_entry)), sizeof(data.student1.name) - 1);
    data.student1.year = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(s1_year_spin));
    data.student1.semester = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(s1_sem_spin));
    strncpy(data.student2.roll_no, gtk_editable_get_text(GTK_EDITABLE(s2_roll_entry)), sizeof(data.student2.roll_no) - 1);
    strncpy(data.student2.name, gtk_editable_get_text(GTK_EDITABLE(s2_name_entry)), sizeof(data.student2.name) - 1);
    data.student2.year = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(s2_year_spin));
    data.student2.semester = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(s2_sem_spin));
    save_assignment_details(&data);
    set_details_editable(FALSE);
    refresh_log_view();
}

static void on_edit_details_clicked(GtkButton *button, gpointer user_data) {
    set_details_editable(TRUE);
}

static void on_set_alarm_clicked(GtkButton *button, gpointer user_data) {
    struct tm target_tm = {0};
    GDateTime *gdt = gtk_calendar_get_date(GTK_CALENDAR(alarm_calendar));
    target_tm.tm_year = g_date_time_get_year(gdt) - 1900;
    target_tm.tm_mon = g_date_time_get_month(gdt) - 1;
    target_tm.tm_mday = g_date_time_get_day_of_month(gdt);
    target_tm.tm_hour = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(alarm_hour));
    target_tm.tm_min = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(alarm_minute));
    target_tm.tm_sec = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(alarm_second));
    target_tm.tm_isdst = -1;
    set_alarm_time(&target_tm);
    g_date_time_unref(gdt);
    refresh_alarm_label();
}

static void activate(GtkApplication *app, gpointer user_data) {
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider,
        "label.ok { color: #28a745; font-weight: bold; } "
        "label.soft { color: #fd7e14; font-weight: bold; } "
        "label.hard { color: #dc3545; font-weight: bold; } "
        "label.expired { background-color: #dc3545; color: white; } "
        ".accent-button { background-color: #3584e4; color: white; border-radius: 5px;}",
        -1);
    gtk_style_context_add_provider_for_display(gdk_display_get_default(), GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    GtkBuilder *builder = gtk_builder_new();
    GError *error = NULL;
    if (gtk_builder_add_from_file(builder, "assignment_gui.ui", &error) == 0) {
        g_printerr("Error loading UI file: %s\n", error->message);
        g_clear_error(&error); return;
    }
    GtkWidget *window = GTK_WIDGET(gtk_builder_get_object(builder, "main_window"));
    gtk_window_set_application(GTK_WINDOW(window), app);

    ps_view = GTK_WIDGET(gtk_builder_get_object(builder, "ps_view"));
    s1_roll_entry = GTK_WIDGET(gtk_builder_get_object(builder, "s1_roll_entry"));
    s1_name_entry = GTK_WIDGET(gtk_builder_get_object(builder, "s1_name_entry"));
    s1_year_spin = GTK_WIDGET(gtk_builder_get_object(builder, "s1_year_spin"));
    s1_sem_spin = GTK_WIDGET(gtk_builder_get_object(builder, "s1_sem_spin"));
    s2_roll_entry = GTK_WIDGET(gtk_builder_get_object(builder, "s2_roll_entry"));
    s2_name_entry = GTK_WIDGET(gtk_builder_get_object(builder, "s2_name_entry"));
    s2_year_spin = GTK_WIDGET(gtk_builder_get_object(builder, "s2_year_spin"));
    s2_sem_spin = GTK_WIDGET(gtk_builder_get_object(builder, "s2_sem_spin"));
    save_button = GTK_WIDGET(gtk_builder_get_object(builder, "save_button"));
    edit_details_button = GTK_WIDGET(gtk_builder_get_object(builder, "edit_details_button"));
    task1_name_entry = GTK_WIDGET(gtk_builder_get_object(builder, "task1_name_entry"));
    task1_from_entry = GTK_WIDGET(gtk_builder_get_object(builder, "task1_from_entry"));
    task1_to_entry = GTK_WIDGET(gtk_builder_get_object(builder, "task1_to_entry"));
    task1_approval_check = GTK_WIDGET(gtk_builder_get_object(builder, "task1_approval_check"));
    task1_save_button = GTK_WIDGET(gtk_builder_get_object(builder, "task1_save_button"));
    task2_name_entry = GTK_WIDGET(gtk_builder_get_object(builder, "task2_name_entry"));
    task2_from_entry = GTK_WIDGET(gtk_builder_get_object(builder, "task2_from_entry"));
    task2_to_entry = GTK_WIDGET(gtk_builder_get_object(builder, "task2_to_entry"));
    task2_approval_check = GTK_WIDGET(gtk_builder_get_object(builder, "task2_approval_check"));
    task2_save_button = GTK_WIDGET(gtk_builder_get_object(builder, "task2_save_button"));
    task3_name_entry = GTK_WIDGET(gtk_builder_get_object(builder, "task3_name_entry"));
    task3_from_entry = GTK_WIDGET(gtk_builder_get_object(builder, "task3_from_entry"));
    task3_to_entry = GTK_WIDGET(gtk_builder_get_object(builder, "task3_to_entry"));
    task3_approval_check = GTK_WIDGET(gtk_builder_get_object(builder, "task3_approval_check"));
    task3_save_button = GTK_WIDGET(gtk_builder_get_object(builder, "task3_save_button"));
    log_view = GTK_WIDGET(gtk_builder_get_object(builder, "log_view"));
    alarm_calendar = GTK_WIDGET(gtk_builder_get_object(builder, "alarm_calendar"));
    alarm_hour = GTK_WIDGET(gtk_builder_get_object(builder, "alarm_hour"));
    alarm_minute = GTK_WIDGET(gtk_builder_get_object(builder, "alarm_minute"));
    alarm_second = GTK_WIDGET(gtk_builder_get_object(builder, "alarm_second"));
    alarm_status_label = GTK_WIDGET(gtk_builder_get_object(builder, "alarm_status_label"));

    g_signal_connect(save_button, "clicked", G_CALLBACK(on_save_clicked), NULL);
    g_signal_connect(edit_details_button, "clicked", G_CALLBACK(on_edit_details_clicked), NULL);
    g_signal_connect(task1_save_button, "clicked", G_CALLBACK(on_task1_save_clicked), NULL);
    g_signal_connect(task2_save_button, "clicked", G_CALLBACK(on_task2_save_clicked), NULL);
    g_signal_connect(task3_save_button, "clicked", G_CALLBACK(on_task3_save_clicked), NULL);
    g_signal_connect(task1_from_entry, "changed", G_CALLBACK(on_date_entry_changed), NULL);
    g_signal_connect(task1_to_entry, "changed", G_CALLBACK(on_date_entry_changed), NULL);
    g_signal_connect(task2_from_entry, "changed", G_CALLBACK(on_date_entry_changed), NULL);
    g_signal_connect(task2_to_entry, "changed", G_CALLBACK(on_date_entry_changed), NULL);
    g_signal_connect(task3_from_entry, "changed", G_CALLBACK(on_date_entry_changed), NULL);
    g_signal_connect(task3_to_entry, "changed", G_CALLBACK(on_date_entry_changed), NULL);
    GtkWidget* set_alarm_button = GTK_WIDGET(gtk_builder_get_object(builder, "set_alarm_button"));
    g_signal_connect(set_alarm_button, "clicked", G_CALLBACK(on_set_alarm_clicked), NULL);

    g_object_unref(builder);

    load_details_into_editor();
    load_tasks_into_editor();
    refresh_log_view();
    g_timeout_add_seconds(1, update_alarm_on_timer, NULL);
    refresh_alarm_label();
    set_details_editable(FALSE);
    gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char **argv) {
    GtkApplication *app;
    int status;
    app = gtk_application_new("org.gtk.assignment", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}