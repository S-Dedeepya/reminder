#include <gtk/gtk.h>
#include "logic.h"
#include <string.h>
#include <ctype.h>
#include <time.h>

// --- Global Widget Pointers ---
GtkWidget *ps_view, *log_view, *tasks_box;
GtkWidget *s1_roll_entry, *s1_name_entry, *s1_year_spin, *s1_sem_spin;
GtkWidget *s2_roll_entry, *s2_name_entry, *s2_year_spin, *s2_sem_spin;
GtkWidget *save_button, *edit_details_button;
GtkWidget *notebook, *alarm_calendar, *alarm_status_label, *set_alarm_button;
GtkEntry *current_date_entry = NULL;

// --- UNDO/REDO GLOBALS ---
GtkWidget *undo_button, *redo_button;
HistoryNode *undo_stack = NULL, *redo_stack = NULL;
guint state_save_timer_id = 0;
gboolean is_loading_state = FALSE;

// --- ALARM GLOBALS ---
time_t deadline_start_time = 0;
time_t deadline_end_time = 0;

// --- FORWARD DECLARATIONS ---
void get_current_form_data(Assignment *data);
void add_task_widget(const Task* task_data);
void refresh_log_view();
void load_tasks_into_editor();
static void on_delete_task_clicked(GtkButton *button, gpointer user_data);
void show_error_dialog(GtkWidget* parent_window, const char* message);
static void save_all_ui_tasks(GtkWidget* button);

// --- HELPER & CALLBACK FUNCTIONS ---

void show_error_dialog(GtkWidget* parent_window, const char* message) {
    GtkAlertDialog *dialog = gtk_alert_dialog_new("%s", message);
    gtk_alert_dialog_show(dialog, GTK_WINDOW(parent_window));
    g_object_unref(dialog);
}

bool is_valid_date(const char* date_str) {
    int d, m, y;
    if (sscanf(date_str, "%d-%d-%d", &d, &m, &y) != 3) return false;
    if (y < 1900 || y > 2100 || m < 1 || m > 12 || d < 1 || d > 31) return false;
    int days_in_month[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if ((y % 4 == 0 && y % 100 != 0) || (y % 400 == 0)) days_in_month[2] = 29;
    if (d > days_in_month[m]) return false;
    return true;
}

time_t date_str_to_time_t(const char* date_str) {
    struct tm tm = {0};
    if (sscanf(date_str, "%d-%d-%d", &tm.tm_mday, &tm.tm_mon, &tm.tm_year) == 3) {
        tm.tm_mon -= 1; tm.tm_year -= 1900;
        return mktime(&tm);
    }
    return -1;
}

void save_deadline_to_file(time_t deadline) {
    FILE* file = fopen("alarm.txt", "w");
    if (file) {
        fprintf(file, "%ld", deadline);
        fclose(file);
    }
}

time_t load_deadline_from_file() {
    time_t deadline = 0;
    FILE* file = fopen("alarm.txt", "r");
    if (file) {
        fscanf(file, "%ld", &deadline);
        fclose(file);
    }
    return deadline;
}

static void on_delete_task_clicked(GtkButton *button, gpointer user_data) {
    GtkWidget* task_row = GTK_WIDGET(user_data);

    if (deadline_end_time > 0) {
        GtkWidget* to_entry = g_object_get_data(G_OBJECT(task_row), "to_entry");
        if (to_entry) {
            const char* to_date_str = gtk_editable_get_text(GTK_EDITABLE(to_entry));
            time_t task_to_time = date_str_to_time_t(to_date_str);
            struct tm *task_tm = localtime(&task_to_time);
            struct tm *deadline_tm = localtime(&deadline_end_time);
            if (task_tm->tm_mday == deadline_tm->tm_mday &&
                task_tm->tm_mon == deadline_tm->tm_mon &&
                task_tm->tm_year == deadline_tm->tm_year)
            {
                deadline_start_time = 0;
                deadline_end_time = 0;
                save_deadline_to_file(0);
                gtk_label_set_text(GTK_LABEL(alarm_status_label), "Alarm cleared as task was deleted.");
            }
        }
    }

    gtk_box_remove(GTK_BOX(tasks_box), task_row);
    save_all_ui_tasks(GTK_WIDGET(button));
}

gboolean on_date_entry_focus(GtkEventControllerFocus *controller, gboolean focus_in, gpointer user_data) {
    if (focus_in) {
        current_date_entry = GTK_ENTRY(user_data);
        gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook), 2);
    }
    return FALSE;
}

void on_calendar_date_selected(GtkCalendar *calendar, gpointer user_data) {
    if (current_date_entry) {
        GDateTime *gdt = gtk_calendar_get_date(calendar);
        char date_str[20];
        sprintf(date_str, "%02d-%02d-%d", g_date_time_get_day_of_month(gdt), g_date_time_get_month(gdt), g_date_time_get_year(gdt));
        gtk_editable_set_text(GTK_EDITABLE(current_date_entry), date_str);
        g_date_time_unref(gdt);
        
        const char *name = gtk_widget_get_name(GTK_WIDGET(current_date_entry));
        if (g_str_has_suffix(name, "_to_entry")) {
            gtk_widget_set_sensitive(set_alarm_button, TRUE);
        } else {
            gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook), 1);
        }
    }
}

void add_task_widget(const Task* task_data) {
    GtkWidget *row_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    GtkWidget *name_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget *name_label = gtk_label_new("<b>Task:</b>");
    gtk_label_set_use_markup(GTK_LABEL(name_label), TRUE);
    GtkWidget *name_entry = gtk_entry_new();
    gtk_widget_set_hexpand(name_entry, TRUE);
    GtkWidget *date_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget *from_label = gtk_label_new("From:");
    GtkWidget *from_entry = gtk_entry_new();
    gtk_widget_set_name(from_entry, "from_entry");
    GtkEventController *from_focus = gtk_event_controller_focus_new();
    g_signal_connect(from_focus, "enter", G_CALLBACK(on_date_entry_focus), from_entry);
    gtk_widget_add_controller(from_entry, from_focus);
    GtkWidget *to_label = gtk_label_new("To:");
    GtkWidget *to_entry = gtk_entry_new();
    gtk_widget_set_name(to_entry, "a_to_entry");
    GtkEventController *to_focus = gtk_event_controller_focus_new();
    g_signal_connect(to_focus, "enter", G_CALLBACK(on_date_entry_focus), to_entry);
    gtk_widget_add_controller(to_entry, to_focus);
    GtkWidget *actions_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget *approval_check = gtk_check_button_new_with_label("Mentor Approval");
    GtkWidget *save_task_button = gtk_button_new_with_label("Save Task");
    gtk_widget_set_halign(save_task_button, GTK_ALIGN_END);
    gtk_widget_set_hexpand(save_task_button, TRUE);
    GtkWidget *delete_button = gtk_button_new_with_label("Delete");
    bool is_already_approved = false;
    if(task_data) {
        gtk_editable_set_text(GTK_EDITABLE(name_entry), task_data->name);
        gtk_editable_set_text(GTK_EDITABLE(from_entry), task_data->from_date);
        gtk_editable_set_text(GTK_EDITABLE(to_entry), task_data->to_date);
        gtk_check_button_set_active(GTK_CHECK_BUTTON(approval_check), task_data->is_approved);
        if(task_data->is_approved) is_already_approved = true;
    }
    g_object_set_data(G_OBJECT(row_box), "name_entry", name_entry);
    g_object_set_data(G_OBJECT(row_box), "from_entry", from_entry);
    g_object_set_data(G_OBJECT(row_box), "to_entry", to_entry);
    g_object_set_data(G_OBJECT(row_box), "approval_check", approval_check);
    g_object_set_data(G_OBJECT(row_box), "is_already_approved", (gpointer)(long)is_already_approved);
    if(task_data && task_data->is_approved) g_object_set_data(G_OBJECT(row_box), "approval_date", g_strdup(task_data->approval_date));
    g_signal_connect(save_task_button, "clicked", G_CALLBACK(save_all_ui_tasks), NULL);
    g_signal_connect(delete_button, "clicked", G_CALLBACK(on_delete_task_clicked), row_box);
    gtk_box_append(GTK_BOX(name_box), name_label); gtk_box_append(GTK_BOX(name_box), name_entry);
    gtk_box_append(GTK_BOX(date_box), from_label); gtk_box_append(GTK_BOX(date_box), from_entry);
    gtk_box_append(GTK_BOX(date_box), to_label); gtk_box_append(GTK_BOX(date_box), to_entry);
    gtk_box_append(GTK_BOX(actions_box), approval_check); gtk_box_append(GTK_BOX(actions_box), save_task_button); gtk_box_append(GTK_BOX(actions_box), delete_button);
    gtk_box_append(GTK_BOX(row_box), name_box); gtk_box_append(GTK_BOX(row_box), date_box); gtk_box_append(GTK_BOX(row_box), actions_box);
    gtk_box_append(GTK_BOX(row_box), gtk_separator_new(GTK_ORIENTATION_HORIZONTAL));
    gtk_box_append(GTK_BOX(tasks_box), row_box);
}

static void save_all_ui_tasks(GtkWidget* button) {
    AllTasks all_tasks = {0}; int count = 0;
    for (GtkWidget *c = gtk_widget_get_first_child(tasks_box); c != NULL; c = gtk_widget_get_next_sibling(c)) count++;
    all_tasks.task_count = count;
    if (all_tasks.task_count > 0) {
        all_tasks.tasks = malloc(all_tasks.task_count * sizeof(Task));
        int i = 0; time_t prev_to_date = 0;
        for (GtkWidget *rb = gtk_widget_get_first_child(tasks_box); rb != NULL; rb = gtk_widget_get_next_sibling(rb)) {
            GtkWidget* ne=g_object_get_data(G_OBJECT(rb),"name_entry"); GtkWidget* fe=g_object_get_data(G_OBJECT(rb),"from_entry");
            GtkWidget* te=g_object_get_data(G_OBJECT(rb),"to_entry"); GtkWidget* ac=g_object_get_data(G_OBJECT(rb),"approval_check");
            const char* fds=gtk_editable_get_text(GTK_EDITABLE(fe)); const char* tds=gtk_editable_get_text(GTK_EDITABLE(te));
            char err[256];
            if (!is_valid_date(fds) || !is_valid_date(tds)) { sprintf(err, "Invalid date in Task %d.", i + 1); show_error_dialog(gtk_widget_get_ancestor(button, GTK_TYPE_WINDOW), err); free(all_tasks.tasks); return; }
            time_t from_t=date_str_to_time_t(fds); time_t to_t=date_str_to_time_t(tds);
            if(to_t<from_t){ sprintf(err, "Task %d 'To' date cannot be before 'From' date.", i + 1); show_error_dialog(gtk_widget_get_ancestor(button, GTK_TYPE_WINDOW), err); free(all_tasks.tasks); return; }
            if(i > 0 && from_t <= prev_to_date){ sprintf(err, "Task %d must start after the previous task ends.", i + 1); show_error_dialog(gtk_widget_get_ancestor(button, GTK_TYPE_WINDOW), err); free(all_tasks.tasks); return; }
            prev_to_date = to_t;
            strcpy(all_tasks.tasks[i].name, gtk_editable_get_text(GTK_EDITABLE(ne))); strcpy(all_tasks.tasks[i].from_date, fds); strcpy(all_tasks.tasks[i].to_date, tds);
            bool is_curr_app=gtk_check_button_get_active(GTK_CHECK_BUTTON(ac)); all_tasks.tasks[i].is_approved = is_curr_app;
            bool was_app=(bool)(long)g_object_get_data(G_OBJECT(rb),"is_already_approved");
            if (is_curr_app && !was_app) { time_t n=time(NULL); strftime(all_tasks.tasks[i].approval_date, 30, "%d-%m-%Y %H:%M", localtime(&n));
            } else { char* od=g_object_get_data(G_OBJECT(rb),"approval_date"); strcpy(all_tasks.tasks[i].approval_date, od ? od : "Not-Approved"); }
            i++;
        }
    }
    overwrite_all_tasks(&all_tasks); free_all_tasks(&all_tasks); refresh_log_view(); load_tasks_into_editor();
}

void load_tasks_into_editor() {
    GtkWidget *child;
    while ((child = gtk_widget_get_first_child(tasks_box)) != NULL) gtk_box_remove(GTK_BOX(tasks_box), child);
    AllTasks all_tasks = load_tasks();
    for (int i = 0; i < all_tasks.task_count; i++) add_task_widget(&all_tasks.tasks[i]);
    free_all_tasks(&all_tasks);
}

static void on_add_task_clicked(GtkButton *button, gpointer user_data) { add_task_widget(NULL); }
void refresh_log_view() {
    char *log_content = generate_master_log_text();
    if (log_content) { GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(log_view)); gtk_text_buffer_set_text(buffer, log_content, -1); free(log_content); }
}

void update_button_states(gboolean editable) {
    if (save_button) gtk_widget_set_visible(save_button, editable);
    if (undo_button) gtk_widget_set_visible(undo_button, editable);
    if (redo_button) gtk_widget_set_visible(redo_button, editable);
    if (edit_details_button) gtk_widget_set_visible(edit_details_button, !editable);
    if (undo_button) gtk_widget_set_sensitive(undo_button, undo_stack != NULL && undo_stack->next != NULL);
    if (redo_button) gtk_widget_set_sensitive(redo_button, redo_stack != NULL);
}

void set_details_editable(gboolean editable) {
    gtk_text_view_set_editable(GTK_TEXT_VIEW(ps_view), editable);
    gtk_editable_set_editable(GTK_EDITABLE(s1_roll_entry), editable); gtk_editable_set_editable(GTK_EDITABLE(s1_name_entry), editable);
    gtk_editable_set_editable(GTK_EDITABLE(s2_roll_entry), editable); gtk_editable_set_editable(GTK_EDITABLE(s2_name_entry), editable);
    gtk_widget_set_sensitive(s1_year_spin, editable); gtk_widget_set_sensitive(s1_sem_spin, editable);
    gtk_widget_set_sensitive(s2_year_spin, editable); gtk_widget_set_sensitive(s2_sem_spin, editable);
    update_button_states(editable);
}

void load_data_into_editor(const Assignment* data) {
    is_loading_state = TRUE;
    GtkTextBuffer *ps_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(ps_view));
    gtk_text_buffer_set_text(ps_buffer, data->problem_statement, -1);
    gtk_editable_set_text(GTK_EDITABLE(s1_roll_entry), data->student1.roll_no); gtk_editable_set_text(GTK_EDITABLE(s1_name_entry), data->student1.name);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(s1_year_spin), data->student1.year); gtk_spin_button_set_value(GTK_SPIN_BUTTON(s1_sem_spin), data->student1.semester);
    gtk_editable_set_text(GTK_EDITABLE(s2_roll_entry), data->student2.roll_no); gtk_editable_set_text(GTK_EDITABLE(s2_name_entry), data->student2.name);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(s2_year_spin), data->student2.year); gtk_spin_button_set_value(GTK_SPIN_BUTTON(s2_sem_spin), data->student2.semester);
    is_loading_state = FALSE;
}

void load_initial_details() {
    Assignment data = load_assignment_details();
    load_data_into_editor(&data);
}

void get_current_form_data(Assignment *data) {
    GtkTextBuffer *ps_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(ps_view));
    GtkTextIter start, end; gtk_text_buffer_get_start_iter(ps_buffer, &start); gtk_text_buffer_get_end_iter(ps_buffer, &end);
    char *ps_text = gtk_text_buffer_get_text(ps_buffer, &start, &end, FALSE);
    strncpy(data->problem_statement, ps_text, sizeof(data->problem_statement) - 1); data->problem_statement[sizeof(data->problem_statement) - 1] = '\0'; g_free(ps_text);
    strncpy(data->student1.roll_no, gtk_editable_get_text(GTK_EDITABLE(s1_roll_entry)), sizeof(data->student1.roll_no) - 1);
    strncpy(data->student1.name, gtk_editable_get_text(GTK_EDITABLE(s1_name_entry)), sizeof(data->student1.name) - 1);
    data->student1.year = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(s1_year_spin)); data->student1.semester = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(s1_sem_spin));
    strncpy(data->student2.roll_no, gtk_editable_get_text(GTK_EDITABLE(s2_roll_entry)), sizeof(data->student2.roll_no) - 1);
    strncpy(data->student2.name, gtk_editable_get_text(GTK_EDITABLE(s2_name_entry)), sizeof(data->student2.name) - 1);
    data->student2.year = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(s2_year_spin)); data->student2.semester = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(s2_sem_spin));
}

static gboolean save_state_to_undo_stack_callback(gpointer user_data) {
    Assignment current_data = {0}; get_current_form_data(&current_data); push_state(&undo_stack, &current_data);
    clear_stack(&redo_stack); update_button_states(TRUE); state_save_timer_id = 0; return G_SOURCE_REMOVE;
}

static void on_detail_field_changed(GtkEditable *editable, gpointer user_data) {
    if (is_loading_state) return; if (state_save_timer_id > 0) g_source_remove(state_save_timer_id);
    state_save_timer_id = g_timeout_add(500, save_state_to_undo_stack_callback, NULL);
}

static void on_undo_clicked(GtkButton *button, gpointer user_data) {
    if (undo_stack == NULL || undo_stack->next == NULL) return;
    Assignment current_state = pop_state(&undo_stack); push_state(&redo_stack, &current_state);
    load_data_into_editor(&(undo_stack->data)); update_button_states(TRUE);
}

static void on_redo_clicked(GtkButton *button, gpointer user_data) {
    if (redo_stack == NULL) return; Assignment new_state = pop_state(&redo_stack);
    push_state(&undo_stack, &new_state); load_data_into_editor(&new_state); update_button_states(TRUE);
}

static void on_save_clicked(GtkButton *button, gpointer user_data) {
    if (state_save_timer_id > 0) g_source_remove(state_save_timer_id);
    Assignment data = {0}; get_current_form_data(&data); save_assignment_details(&data);
    set_details_editable(FALSE); refresh_log_view(); clear_stack(&undo_stack); clear_stack(&redo_stack);
}

static void on_edit_details_clicked(GtkButton *button, gpointer user_data) {
    clear_stack(&undo_stack); clear_stack(&redo_stack);
    Assignment initial_data = {0}; get_current_form_data(&initial_data);
    push_state(&undo_stack, &initial_data); set_details_editable(TRUE);
}

static gboolean check_and_display_alarm_status(gpointer user_data) {
    if (deadline_start_time == 0 || deadline_end_time == 0) return G_SOURCE_CONTINUE;
    time_t now = time(NULL);
    double total_duration = difftime(deadline_end_time, deadline_start_time);
    if (total_duration <= 0) return G_SOURCE_CONTINUE;
    double time_left = difftime(deadline_end_time, now);
    char buffer[128];
    if (time_left < 0) { sprintf(buffer, "HARD ALARM: Deadline has passed!");
    } else if (time_left / total_duration <= 0.20) { sprintf(buffer, "HARD ALARM: Less than 20%% of time remaining!");
    } else { sprintf(buffer, "SOFT ALARM: Deadline is approaching."); }
    gtk_label_set_text(GTK_LABEL(alarm_status_label), buffer);
    return G_SOURCE_CONTINUE;
}

static void on_set_alarm_clicked(GtkButton *button, gpointer user_data) {
    if (!current_date_entry) {
        show_error_dialog(gtk_widget_get_ancestor(GTK_WIDGET(button), GTK_TYPE_WINDOW), "Please click a 'To' date field in the Tasks tab first.");
        return;
    }
    const char* to_date_str = gtk_editable_get_text(GTK_EDITABLE(current_date_entry));
    if (!is_valid_date(to_date_str)) {
        gtk_label_set_text(GTK_LABEL(alarm_status_label), "Invalid Date: Please select a valid date.");
        return;
    }
    
    deadline_end_time = date_str_to_time_t(to_date_str);
    deadline_start_time = time(NULL);

    if (deadline_end_time <= deadline_start_time) {
        gtk_label_set_text(GTK_LABEL(alarm_status_label), "Invalid Alarm: 'To' date must be in the future.");
        deadline_start_time = 0; deadline_end_time = 0;
        save_deadline_to_file(0);
    } else {
        save_deadline_to_file(deadline_end_time);
        gtk_label_set_text(GTK_LABEL(alarm_status_label), "Alarm is set. Status will update automatically.");
    }
    gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook), 1);
    gtk_widget_set_sensitive(button, FALSE);
}

static void activate(GtkApplication *app, gpointer user_data) {
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_string(provider, 
        ".accent-button { background-color: #3584e4; color: white; border-radius: 5px;}"
        "label.alarm-status { font-weight: bold; }");
    gtk_style_context_add_provider_for_display(gdk_display_get_default(), GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    GtkBuilder *builder = gtk_builder_new();
    GError *error = NULL;
    if (gtk_builder_add_from_file(builder, "assignment_gui.ui", &error) == 0) { g_printerr("Error: %s\n", error->message); g_clear_error(&error); return; }
    GtkWidget *window = GTK_WIDGET(gtk_builder_get_object(builder, "main_window"));
    gtk_window_set_application(GTK_WINDOW(window), app);
    notebook = GTK_WIDGET(gtk_builder_get_object(builder, "notebook"));
    alarm_calendar = GTK_WIDGET(gtk_builder_get_object(builder, "alarm_calendar"));
    alarm_status_label = GTK_WIDGET(gtk_builder_get_object(builder, "alarm_status_label"));
    set_alarm_button = GTK_WIDGET(gtk_builder_get_object(builder, "set_alarm_button"));
    ps_view = GTK_WIDGET(gtk_builder_get_object(builder, "ps_view"));
    s1_roll_entry=GTK_WIDGET(gtk_builder_get_object(builder,"s1_roll_entry")); s1_name_entry=GTK_WIDGET(gtk_builder_get_object(builder,"s1_name_entry")); s1_year_spin=GTK_WIDGET(gtk_builder_get_object(builder,"s1_year_spin")); s1_sem_spin=GTK_WIDGET(gtk_builder_get_object(builder,"s1_sem_spin"));
    s2_roll_entry=GTK_WIDGET(gtk_builder_get_object(builder,"s2_roll_entry")); s2_name_entry=GTK_WIDGET(gtk_builder_get_object(builder,"s2_name_entry")); s2_year_spin=GTK_WIDGET(gtk_builder_get_object(builder,"s2_year_spin")); s2_sem_spin=GTK_WIDGET(gtk_builder_get_object(builder,"s2_sem_spin"));
    save_button=GTK_WIDGET(gtk_builder_get_object(builder,"save_button")); edit_details_button=GTK_WIDGET(gtk_builder_get_object(builder,"edit_details_button"));
    undo_button=GTK_WIDGET(gtk_builder_get_object(builder,"undo_button")); redo_button=GTK_WIDGET(gtk_builder_get_object(builder,"redo_button"));
    log_view=GTK_WIDGET(gtk_builder_get_object(builder,"log_view"));
    tasks_box=GTK_WIDGET(gtk_builder_get_object(builder,"tasks_box"));
    GtkWidget* add_task_button=GTK_WIDGET(gtk_builder_get_object(builder,"add_task_button"));
    g_signal_connect(save_button,"clicked",G_CALLBACK(on_save_clicked),NULL);
    g_signal_connect(edit_details_button,"clicked",G_CALLBACK(on_edit_details_clicked),NULL);
    g_signal_connect(undo_button,"clicked",G_CALLBACK(on_undo_clicked),NULL);
    g_signal_connect(redo_button,"clicked",G_CALLBACK(on_redo_clicked),NULL);
    g_signal_connect(add_task_button,"clicked",G_CALLBACK(on_add_task_clicked),NULL);
    g_signal_connect(alarm_calendar,"day-selected",G_CALLBACK(on_calendar_date_selected),NULL);
    g_signal_connect(set_alarm_button,"clicked",G_CALLBACK(on_set_alarm_clicked),NULL);
    GtkTextBuffer*ps_buffer=gtk_text_view_get_buffer(GTK_TEXT_VIEW(ps_view));
    gtk_text_buffer_set_enable_undo(ps_buffer,FALSE);
    g_signal_connect(ps_buffer,"changed",G_CALLBACK(on_detail_field_changed),NULL);
    g_signal_connect(s1_roll_entry,"changed",G_CALLBACK(on_detail_field_changed),NULL); g_signal_connect(s1_name_entry,"changed",G_CALLBACK(on_detail_field_changed),NULL);
    g_signal_connect(s2_roll_entry,"changed",G_CALLBACK(on_detail_field_changed),NULL); g_signal_connect(s2_name_entry,"changed",G_CALLBACK(on_detail_field_changed),NULL);
    g_signal_connect(s1_year_spin,"value-changed",G_CALLBACK(on_detail_field_changed),NULL); g_signal_connect(s1_sem_spin,"value-changed",G_CALLBACK(on_detail_field_changed),NULL);
    g_signal_connect(s2_year_spin,"value-changed",G_CALLBACK(on_detail_field_changed),NULL); g_signal_connect(s2_sem_spin,"value-changed",G_CALLBACK(on_detail_field_changed),NULL);
    g_object_unref(builder);
    deadline_end_time = load_deadline_from_file();
    if(deadline_end_time > 0) {
        deadline_start_time = time(NULL);
        struct tm *deadline_tm = localtime(&deadline_end_time);
        gtk_calendar_select_day(GTK_CALENDAR(alarm_calendar), g_date_time_new_local(deadline_tm->tm_year + 1900, deadline_tm->tm_mon + 1, deadline_tm->tm_mday, 0,0,0));
    }
    load_initial_details(); load_tasks_into_editor(); refresh_log_view();
    set_details_editable(FALSE);
    g_timeout_add_seconds(1, check_and_display_alarm_status, NULL);
    gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char **argv) {
    GtkApplication *app;
    int status;
    app = gtk_application_new("org.gtk.assignment", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    clear_stack(&undo_stack);
    clear_stack(&redo_stack);
    return status;
}