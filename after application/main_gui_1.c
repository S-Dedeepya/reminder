#include <gtk/gtk.h>
#include "logic.h"
#include <string.h>

// --- Global Widget Pointers ---
GtkWidget *ps_view;
GtkWidget *s1_roll_entry, *s1_name_entry, *s1_year_spin, *s1_sem_spin;
GtkWidget *s2_roll_entry, *s2_name_entry, *s2_year_spin, *s2_sem_spin;
GtkWidget *log_view;
GtkWidget *alarm_calendar, *alarm_hour, *alarm_minute, *alarm_second, *alarm_status_label;


// --- Helper function to update the alarm label ---
void refresh_alarm_label() {
    if (alarm_status_label == NULL) return; // Guard against being called too early

    AlarmStatus status = check_alarm_status();
    time_t target_time = get_alarm_time();
    char buffer[128];

    gtk_widget_remove_css_class(alarm_status_label, "ok");
    gtk_widget_remove_css_class(alarm_status_label, "soft");
    gtk_widget_remove_css_class(alarm_status_label, "hard");
    gtk_widget_remove_css_class(alarm_status_label, "expired");

    switch (status) {
        case ALARM_NOT_SET:
            strcpy(buffer, "Status: No alarm has been set.");
            break;
        case ALARM_OK:
            strftime(buffer, sizeof(buffer), "Status: OK. Deadline is %Y-%m-%d %H:%M:%S", localtime(&target_time));
            gtk_widget_add_css_class(alarm_status_label, "ok");
            break;
        case ALARM_SOFT:
            strftime(buffer, sizeof(buffer), "SOFT ALARM! Deadline is %Y-%m-%d %H:%M:%S", localtime(&target_time));
            gtk_widget_add_css_class(alarm_status_label, "soft");
            break;
        case ALARM_HARD:
            strcpy(buffer, "!!! HARD ALARM! Less than 8 hours remaining !!!");
            gtk_widget_add_css_class(alarm_status_label, "hard");
            break;
        case ALARM_EXPIRED:
            strcpy(buffer, "!!! TIME IS UP !!! The deadline has passed.");
            gtk_widget_add_css_class(alarm_status_label, "expired");
            break;
    }

    gtk_label_set_text(GTK_LABEL(alarm_status_label), buffer);
}

// Timer function that calls the helper function every second
static gboolean update_alarm_on_timer(gpointer user_data) {
    refresh_alarm_label();
    return G_SOURCE_CONTINUE;
}


// --- Callback Functions ---

static void on_load_clicked(GtkButton *button, gpointer user_data) {
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

static void on_save_clicked(GtkButton *button, gpointer user_data) {
    Assignment data = {0};
    GtkTextBuffer *ps_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(ps_view));
    GtkTextIter start, end;
    gtk_text_buffer_get_start_iter(ps_buffer, &start);
    gtk_text_buffer_get_end_iter(ps_buffer, &end);
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
}

static void on_add_log_clicked(GtkButton *button, gpointer user_data) {
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(log_view));
    GtkTextIter start, end;
    gtk_text_buffer_get_start_iter(buffer, &start);
    gtk_text_buffer_get_end_iter(buffer, &end);
    char *log_text = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);
    if (strlen(log_text) > 0) {
        add_log_entry(log_text);
        gtk_text_buffer_set_text(buffer, "", -1);
    }
    g_free(log_text);
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


// This function builds the UI when the application is activated
static void activate(GtkApplication *app, gpointer user_data) {
    GtkWidget *window, *notebook, *details_grid, *log_box, *label, *button_box, *save_button, *load_button, *scrolled_window;

    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Assignment Deadline Manager");
    gtk_window_set_default_size(GTK_WINDOW(window), 600, 700);

    notebook = gtk_notebook_new();
    gtk_window_set_child(GTK_WINDOW(window), notebook);

    details_grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(details_grid), 10);
    gtk_grid_set_row_spacing(GTK_GRID(details_grid), 10);
    gtk_widget_set_margin_start(details_grid, 10);
    gtk_widget_set_margin_end(details_grid, 10);
    gtk_widget_set_margin_top(details_grid, 10);
    gtk_widget_set_margin_bottom(details_grid, 10);

    label = gtk_label_new("Problem Statement:");
    gtk_grid_attach(GTK_GRID(details_grid), label, 0, 0, 2, 1);
    ps_view = gtk_text_view_new();
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(ps_view), GTK_WRAP_WORD_CHAR);
    scrolled_window = gtk_scrolled_window_new();
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled_window), ps_view);
    gtk_widget_set_size_request(scrolled_window, -1, 150);
    gtk_grid_attach(GTK_GRID(details_grid), scrolled_window, 0, 1, 2, 1);

    label = gtk_label_new("--- Student 1 ---");
    gtk_grid_attach(GTK_GRID(details_grid), label, 0, 2, 2, 1);
    label = gtk_label_new("Roll No:");
    gtk_grid_attach(GTK_GRID(details_grid), label, 0, 3, 1, 1);
    s1_roll_entry = gtk_entry_new();
    gtk_grid_attach(GTK_GRID(details_grid), s1_roll_entry, 1, 3, 1, 1);
    label = gtk_label_new("Name:");
    gtk_grid_attach(GTK_GRID(details_grid), label, 0, 4, 1, 1);
    s1_name_entry = gtk_entry_new();
    gtk_grid_attach(GTK_GRID(details_grid), s1_name_entry, 1, 4, 1, 1);
    label = gtk_label_new("Year:");
    gtk_grid_attach(GTK_GRID(details_grid), label, 0, 5, 1, 1);
    s1_year_spin = gtk_spin_button_new_with_range(1, 4, 1);
    gtk_grid_attach(GTK_GRID(details_grid), s1_year_spin, 1, 5, 1, 1);
    label = gtk_label_new("Semester:");
    gtk_grid_attach(GTK_GRID(details_grid), label, 0, 6, 1, 1);
    s1_sem_spin = gtk_spin_button_new_with_range(1, 8, 1);
    gtk_grid_attach(GTK_GRID(details_grid), s1_sem_spin, 1, 6, 1, 1);

    label = gtk_label_new("--- Student 2 ---");
    gtk_grid_attach(GTK_GRID(details_grid), label, 0, 7, 2, 1);
    label = gtk_label_new("Roll No:");
    gtk_grid_attach(GTK_GRID(details_grid), label, 0, 8, 1, 1);
    s2_roll_entry = gtk_entry_new();
    gtk_grid_attach(GTK_GRID(details_grid), s2_roll_entry, 1, 8, 1, 1);
    label = gtk_label_new("Name:");
    gtk_grid_attach(GTK_GRID(details_grid), label, 0, 9, 1, 1);
    s2_name_entry = gtk_entry_new();
    gtk_grid_attach(GTK_GRID(details_grid), s2_name_entry, 1, 9, 1, 1);
    label = gtk_label_new("Year:");
    gtk_grid_attach(GTK_GRID(details_grid), label, 0, 10, 1, 1);
    s2_year_spin = gtk_spin_button_new_with_range(1, 4, 1);
    gtk_grid_attach(GTK_GRID(details_grid), s2_year_spin, 1, 10, 1, 1);
    label = gtk_label_new("Semester:");
    gtk_grid_attach(GTK_GRID(details_grid), label, 0, 11, 1, 1);
    s2_sem_spin = gtk_spin_button_new_with_range(1, 8, 1);
    gtk_grid_attach(GTK_GRID(details_grid), s2_sem_spin, 1, 11, 1, 1);

    button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_halign(button_box, GTK_ALIGN_CENTER);
    load_button = gtk_button_new_with_label("Load Details");
    g_signal_connect(load_button, "clicked", G_CALLBACK(on_load_clicked), NULL);
    gtk_box_append(GTK_BOX(button_box), load_button);
    save_button = gtk_button_new_with_label("Save Details");
    g_signal_connect(save_button, "clicked", G_CALLBACK(on_save_clicked), NULL);
    gtk_box_append(GTK_BOX(button_box), save_button);
    gtk_grid_attach(GTK_GRID(details_grid), button_box, 0, 12, 2, 1);
    label = gtk_label_new("Assignment Details");
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), details_grid, label);

    log_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_start(log_box, 10);
    gtk_widget_set_margin_end(log_box, 10);
    gtk_widget_set_margin_top(log_box, 10);
    gtk_widget_set_margin_bottom(log_box, 10);
    label = gtk_label_new("Enter your log update below:");
    gtk_box_append(GTK_BOX(log_box), label);
    log_view = gtk_text_view_new();
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(log_view), GTK_WRAP_WORD_CHAR);
    scrolled_window = gtk_scrolled_window_new();
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled_window), log_view);
    gtk_widget_set_vexpand(scrolled_window, TRUE);
    gtk_box_append(GTK_BOX(log_box), scrolled_window);
    GtkWidget *add_log_button = gtk_button_new_with_label("Add Log Entry");
    gtk_widget_set_halign(add_log_button, GTK_ALIGN_END);
    g_signal_connect(add_log_button, "clicked", G_CALLBACK(on_add_log_clicked), NULL);
    gtk_box_append(GTK_BOX(log_box), add_log_button);
    label = gtk_label_new("Project Log");
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), log_box, label);

    GtkWidget *alarm_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
    gtk_widget_set_margin_start(alarm_box, 20);
    gtk_widget_set_margin_end(alarm_box, 20);
    gtk_widget_set_margin_top(alarm_box, 20);
    gtk_widget_set_margin_bottom(alarm_box, 20);
    alarm_calendar = gtk_calendar_new();
    gtk_widget_set_halign(alarm_calendar, GTK_ALIGN_CENTER);
    gtk_box_append(GTK_BOX(alarm_box), alarm_calendar);
    GtkWidget *time_grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(time_grid), 10);
    gtk_widget_set_halign(time_grid, GTK_ALIGN_CENTER);
    alarm_hour = gtk_spin_button_new_with_range(0, 23, 1);
    alarm_minute = gtk_spin_button_new_with_range(0, 59, 1);
    alarm_second = gtk_spin_button_new_with_range(0, 59, 1);
    gtk_grid_attach(GTK_GRID(time_grid), gtk_label_new("Hour:"), 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(time_grid), alarm_hour, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(time_grid), gtk_label_new("Minute:"), 2, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(time_grid), alarm_minute, 3, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(time_grid), gtk_label_new("Second:"), 4, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(time_grid), alarm_second, 5, 0, 1, 1);
    gtk_box_append(GTK_BOX(alarm_box), time_grid);
    GtkWidget *set_alarm_button = gtk_button_new_with_label("Set/Update Alarm");
    gtk_widget_set_halign(set_alarm_button, GTK_ALIGN_CENTER);
    g_signal_connect(set_alarm_button, "clicked", G_CALLBACK(on_set_alarm_clicked), NULL);
    gtk_box_append(GTK_BOX(alarm_box), set_alarm_button);
    alarm_status_label = gtk_label_new("Status: No alarm has been set.");
    gtk_widget_set_halign(alarm_status_label, GTK_ALIGN_CENTER);
    gtk_label_set_wrap(GTK_LABEL(alarm_status_label), TRUE);
    gtk_box_append(GTK_BOX(alarm_box), alarm_status_label);
    GtkCssProvider *cssProvider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(cssProvider, ".ok { color: green; } .soft { color: orange; font-weight: bold; } .hard { color: red; font-weight: bold; } .expired { background-color: red; color: white; font-weight: bold; }", -1);
    gtk_style_context_add_provider_for_display(gdk_display_get_default(), GTK_STYLE_PROVIDER(cssProvider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    label = gtk_label_new("Deadline Alarm");
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), alarm_box, label);

    g_timeout_add_seconds(1, update_alarm_on_timer, NULL);
    refresh_alarm_label();
    gtk_widget_show(window);
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
