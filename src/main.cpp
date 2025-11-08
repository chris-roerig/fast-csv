#include <gtk/gtk.h>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <iostream>

#ifdef __APPLE__
extern "C" void setup_mac_open_bridge(GtkApplication *app);
#endif

class CSVRenderer {
private:
    GtkApplication *app;
    GtkWidget *window;
    GtkWidget *tree_view;
    GtkListStore *store;
    std::vector<std::vector<std::string>> data;
    GtkTreeViewColumn *selected_column;
    GtkTreePath *selected_path;
    bool select_all_mode;
    bool text_wrap_enabled;

    static void toggle_wrap_callback(GtkWidget *widget, gpointer data) {
        CSVRenderer *renderer = static_cast<CSVRenderer*>(data);
        renderer->toggle_text_wrap();
    }

    static gboolean on_key_press(GtkWidget *widget, GdkEventKey *event, gpointer data) {
        CSVRenderer *renderer = static_cast<CSVRenderer*>(data);
        
        gboolean is_copy = FALSE;
        gboolean is_select_all = FALSE;
        gboolean is_toggle_wrap = FALSE;
        
#ifdef __APPLE__
        is_copy = (event->state & GDK_META_MASK) && event->keyval == GDK_KEY_c;
        is_select_all = (event->state & GDK_META_MASK) && event->keyval == GDK_KEY_a;
        is_toggle_wrap = (event->state & GDK_META_MASK) && event->keyval == GDK_KEY_w;
#else
        is_copy = (event->state & GDK_CONTROL_MASK) && event->keyval == GDK_KEY_c;
        is_select_all = (event->state & GDK_CONTROL_MASK) && event->keyval == GDK_KEY_a;
        is_toggle_wrap = (event->state & GDK_CONTROL_MASK) && event->keyval == GDK_KEY_w;
#endif
        
        if (is_toggle_wrap) {
            renderer->toggle_text_wrap();
            return TRUE;
        }
        
        if (is_select_all) {
            renderer->select_all_mode = true;
            renderer->selected_path = nullptr;
            renderer->selected_column = nullptr;
            gtk_widget_queue_draw(widget);
            return TRUE;
        }
        
        if (is_copy) {
            if (renderer->select_all_mode) {
                // Copy entire CSV data
                std::string csv_content;
                for (size_t i = 0; i < renderer->data.size(); i++) {
                    for (size_t j = 0; j < renderer->data[i].size(); j++) {
                        if (j > 0) csv_content += ",";
                        csv_content += renderer->data[i][j];
                    }
                    if (i < renderer->data.size() - 1) csv_content += "\n";
                }
                
                GtkClipboard *clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
                gtk_clipboard_set_text(clipboard, csv_content.c_str(), -1);
            } else if (renderer->selected_path && renderer->selected_column) {
                // Copy single cell
                GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(widget));
                GtkTreeIter iter;
                
                if (gtk_tree_model_get_iter(model, &iter, renderer->selected_path)) {
                    gchar *text;
                    gint col_index = g_list_index(gtk_tree_view_get_columns(GTK_TREE_VIEW(widget)), 
                                                 renderer->selected_column);
                    gtk_tree_model_get(model, &iter, col_index, &text, -1);
                    
                    GtkClipboard *clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
                    gtk_clipboard_set_text(clipboard, text, -1);
                    g_free(text);
                }
            }
        }
        return FALSE;
    }

    static gboolean on_button_press(GtkWidget *widget, GdkEventButton *event, gpointer data) {
        CSVRenderer *renderer = static_cast<CSVRenderer*>(data);
        GtkTreePath *path;
        GtkTreeViewColumn *column;
        
        if (event->button == 3) { // Right click
            GtkWidget *menu = gtk_menu_new();
            GtkWidget *wrap_item = gtk_menu_item_new_with_label(
                renderer->text_wrap_enabled ? "Disable Text Wrap" : "Enable Text Wrap");
            
            g_signal_connect(wrap_item, "activate", G_CALLBACK(toggle_wrap_callback), renderer);
            
            gtk_menu_shell_append(GTK_MENU_SHELL(menu), wrap_item);
            gtk_widget_show_all(menu);
            gtk_menu_popup_at_pointer(GTK_MENU(menu), (GdkEvent*)event);
            return TRUE;
        }
        
        if (gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(widget), event->x, event->y, &path, &column, NULL, NULL)) {
            renderer->select_all_mode = false;
            renderer->selected_path = path;
            renderer->selected_column = column;
            gtk_widget_queue_draw(widget);
        }
        return FALSE;
    }

    static void cell_data_func(GtkTreeViewColumn *column, GtkCellRenderer *renderer, 
                              GtkTreeModel *model, GtkTreeIter *iter, gpointer data) {
        CSVRenderer *csv_renderer = static_cast<CSVRenderer*>(data);
        GtkTreePath *path = gtk_tree_model_get_path(model, iter);
        
        bool is_selected = false;
        if (csv_renderer->select_all_mode) {
            is_selected = true;
        } else if (csv_renderer->selected_path && csv_renderer->selected_column == column &&
            gtk_tree_path_compare(path, csv_renderer->selected_path) == 0) {
            is_selected = true;
        }
        
        gint width = gtk_tree_view_column_get_width(column);
        if (width <= 0) width = 150;
        
        if (is_selected) {
            g_object_set(renderer, "background", "#3584e4", "foreground", "white", NULL);
            if (csv_renderer->text_wrap_enabled) {
                g_object_set(renderer, 
                    "wrap-width", width - 20,
                    "ellipsize", PANGO_ELLIPSIZE_NONE,
                    NULL);
            } else {
                g_object_set(renderer, 
                    "wrap-width", -1,
                    "ellipsize", PANGO_ELLIPSIZE_END,
                    NULL);
            }
        } else {
            g_object_set(renderer, "background", NULL, "foreground", NULL, NULL);
            g_object_set(renderer, 
                "wrap-width", -1,
                "ellipsize", PANGO_ELLIPSIZE_END,
                NULL);
        }
        
        gtk_tree_path_free(path);
    }

    static void open_file_dialog(GtkWidget *widget, gpointer data) {
        CSVRenderer *renderer = static_cast<CSVRenderer*>(data);
        
        GtkWidget *dialog = gtk_file_chooser_dialog_new("Open CSV File",
            GTK_WINDOW(renderer->window), GTK_FILE_CHOOSER_ACTION_OPEN,
            "_Cancel", GTK_RESPONSE_CANCEL,
            "_Open", GTK_RESPONSE_ACCEPT, NULL);
        
        GtkFileFilter *filter = gtk_file_filter_new();
        gtk_file_filter_set_name(filter, "CSV files");
        gtk_file_filter_add_pattern(filter, "*.csv");
        gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
        
        if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
            char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
            renderer->load_csv(filename);
            g_free(filename);
        }
        
        gtk_widget_destroy(dialog);
    }

    static void window_destroy_callback(GtkWidget *widget, gpointer data) {
        GtkApplication *app = static_cast<GtkApplication*>(data);
        g_application_quit(G_APPLICATION(app));
    }

    static void close_app(GtkWidget *widget, gpointer data) {
        GtkApplication *app = static_cast<GtkApplication*>(data);
        g_application_quit(G_APPLICATION(app));
    }

public:
    CSVRenderer(GtkApplication *application) : app(application), window(nullptr), selected_column(nullptr), selected_path(nullptr), store(nullptr), select_all_mode(false), text_wrap_enabled(false) {
    }

    static void on_activate(GtkApplication *app, gpointer user_data) {
        CSVRenderer *renderer = static_cast<CSVRenderer*>(user_data);
        if (!renderer->window) {
            renderer->create_window();
        }
        renderer->show();
    }

    static void on_open(GtkApplication *app, GFile **files, gint n_files, const gchar *hint, gpointer user_data) {
        CSVRenderer *renderer = static_cast<CSVRenderer*>(user_data);
        
        std::cout << "on_open called with " << n_files << " files" << std::endl;
        
        if (!renderer->window) {
            renderer->create_window();
        }
        
        // Load the first file
        if (n_files > 0) {
            gchar *path = g_file_get_path(files[0]);
            if (path) {
                std::cout << "Opening file via GtkApplication: " << path << std::endl;
                renderer->load_csv(std::string(path));
                g_free(path);
            } else {
                std::cout << "Failed to get file path" << std::endl;
            }
        } else {
            std::cout << "No files to open" << std::endl;
        }
        
        renderer->show();
    }

    void create_window() {
        window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
        gtk_window_set_title(GTK_WINDOW(window), "Fast CSV Viewer");
        gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);
        
        g_signal_connect(window, "destroy", G_CALLBACK(window_destroy_callback), app);
        
        // Create menu bar
        GtkWidget *menubar = gtk_menu_bar_new();
        GtkWidget *file_menu = gtk_menu_new();
        GtkWidget *file_item = gtk_menu_item_new_with_label("File");
        GtkWidget *open_item = gtk_menu_item_new_with_label("Open");
        GtkWidget *close_item = gtk_menu_item_new_with_label("Close");
        
        gtk_menu_item_set_submenu(GTK_MENU_ITEM(file_item), file_menu);
        gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), open_item);
        gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), close_item);
        gtk_menu_shell_append(GTK_MENU_SHELL(menubar), file_item);
        
        g_signal_connect(open_item, "activate", G_CALLBACK(open_file_dialog), this);
        g_signal_connect(close_item, "activate", G_CALLBACK(close_app), app);
        
        tree_view = gtk_tree_view_new();
        gtk_tree_view_set_enable_search(GTK_TREE_VIEW(tree_view), FALSE);
        gtk_tree_view_set_fixed_height_mode(GTK_TREE_VIEW(tree_view), FALSE);
        
        GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree_view));
        gtk_tree_selection_set_mode(selection, GTK_SELECTION_NONE);
        
        g_signal_connect(tree_view, "button-press-event", G_CALLBACK(on_button_press), this);
        g_signal_connect(tree_view, "key-press-event", G_CALLBACK(on_key_press), this);
        gtk_widget_set_can_focus(tree_view, TRUE);
        
        GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
        GtkWidget *scrolled = gtk_scrolled_window_new(NULL, NULL);
        
        gtk_container_add(GTK_CONTAINER(scrolled), tree_view);
        gtk_box_pack_start(GTK_BOX(vbox), menubar, FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(vbox), scrolled, TRUE, TRUE, 0);
        gtk_container_add(GTK_CONTAINER(window), vbox);
        
        gtk_application_add_window(app, GTK_WINDOW(window));
    }

    void load_csv(const std::string& filename) {
        std::ifstream file(filename);
        std::string line;
        data.clear();
        
        // Debug output
        std::cout << "Loading file: " << filename << std::endl;
        
        // Clear existing model
        if (store) {
            gtk_tree_view_set_model(GTK_TREE_VIEW(tree_view), NULL);
            g_object_unref(store);
        }
        
        // Remove existing columns
        GList *columns = gtk_tree_view_get_columns(GTK_TREE_VIEW(tree_view));
        for (GList *iter = columns; iter != NULL; iter = g_list_next(iter)) {
            gtk_tree_view_remove_column(GTK_TREE_VIEW(tree_view), GTK_TREE_VIEW_COLUMN(iter->data));
        }
        g_list_free(columns);
        
        while (std::getline(file, line)) {
            std::vector<std::string> row;
            std::stringstream ss(line);
            std::string cell;
            
            while (std::getline(ss, cell, ',')) {
                row.push_back(cell);
            }
            data.push_back(row);
        }
        
        std::cout << "Loaded " << data.size() << " rows" << std::endl;
        
        if (data.empty()) return;
        
        // Create columns
        int cols = data[0].size();
        GType *types = new GType[cols];
        for (int i = 0; i < cols; i++) {
            types[i] = G_TYPE_STRING;
            
            GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
            g_object_set(renderer, 
                "ellipsize", PANGO_ELLIPSIZE_END,
                "wrap-mode", PANGO_WRAP_WORD_CHAR,
                "yalign", 0.0,
                NULL);
            GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes(
                data[0][i].c_str(), renderer, "text", i, NULL);
            gtk_tree_view_column_set_resizable(column, TRUE);
            gtk_tree_view_column_set_fixed_width(column, 150);
            gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_FIXED);
            gtk_tree_view_column_set_sort_column_id(column, i);
            gtk_tree_view_column_set_cell_data_func(column, renderer, cell_data_func, this, NULL);
            gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), column);
        }
        
        store = gtk_list_store_newv(cols, types);
        
        // Set up sorting for each column
        for (int i = 0; i < cols; i++) {
            gtk_tree_sortable_set_sort_func(GTK_TREE_SORTABLE(store), i, 
                [](GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b, gpointer data) -> gint {
                    gchar *str_a, *str_b;
                    gint col = GPOINTER_TO_INT(data);
                    gtk_tree_model_get(model, a, col, &str_a, -1);
                    gtk_tree_model_get(model, b, col, &str_b, -1);
                    gint result = g_strcmp0(str_a, str_b);
                    g_free(str_a);
                    g_free(str_b);
                    return result;
                }, GINT_TO_POINTER(i), NULL);
        }
        
        delete[] types;
        
        // Add data (skip header)
        for (size_t i = 1; i < data.size(); i++) {
            GtkTreeIter iter;
            gtk_list_store_append(store, &iter);
            
            for (size_t j = 0; j < data[i].size() && j < cols; j++) {
                gtk_list_store_set(store, &iter, j, data[i][j].c_str(), -1);
            }
        }
        
        gtk_tree_view_set_model(GTK_TREE_VIEW(tree_view), GTK_TREE_MODEL(store));
    }

    void toggle_text_wrap() {
        text_wrap_enabled = !text_wrap_enabled;
        gtk_widget_queue_draw(tree_view);
    }

    void show() {
        if (window) {
            gtk_widget_show_all(window);
            gtk_window_present(GTK_WINDOW(window));
        }
    }
};

int main(int argc, char *argv[]) {
    GtkApplication *app = gtk_application_new("com.fastcsv.app", G_APPLICATION_HANDLES_OPEN);
    
    CSVRenderer renderer(app);
    
    g_signal_connect(app, "activate", G_CALLBACK(CSVRenderer::on_activate), &renderer);
    g_signal_connect(app, "open", G_CALLBACK(CSVRenderer::on_open), &renderer);
    
#ifdef __APPLE__
    setup_mac_open_bridge(app);
#endif
    
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    
    return status;
}
