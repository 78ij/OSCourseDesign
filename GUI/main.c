#include <gtk/gtk.h>
 
static void print_hello(GtkWidget *button , gpointer data);
static void activate(GtkApplication *app , gpointer data);
static void print_entry(GtkWidget * button , gpointer entry);
 
int main(int argc , char **argv)
{
    GtkApplication *app;
    int app_status;
 
    app = gtk_application_new("com.wa.aw" , G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app , "activate" , G_CALLBACK(activate) , NULL);
    app_status = g_application_run(G_APPLICATION(app) , argc , argv);
 
    g_object_unref(app);
    return app_status;
}

 
static void activate(GtkApplication *app , gpointer data)
{
    GtkBuilder *builder;
    GObject *window;
    GObject *button;
    GObject *entry;
 
    //创建一个 GtkBuilder 。
    builder = gtk_builder_new();
    //将描述 UI 的文件添加到 Builder 中。
    gtk_builder_add_from_file(builder , "UI2.glade" , NULL);
 
    //可以用该函数获取 Builder 创建的对象，根据 example.ui 文件中设置的 object 的 id 来获取。
    window = gtk_builder_get_object(builder , "window1");
    gtk_application_add_window(app , GTK_WINDOW(window));
    gtk_widget_show(GTK_WIDGET(window));
}