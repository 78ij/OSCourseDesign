#include <gtk/gtk.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string>
#include <string.h>
#include <fstream>
#include <iostream>
#include <pwd.h>
#include <map>
using namespace std;

GObject *Label;
GObject *CPUProg;
GObject *CPULabel;
GObject *CPUGraph;
GObject *MEMProg;
GObject *MEMLabel;
GObject *MEMGraph;
GtkListStore *ListStore;
static void activate(GtkApplication *app , gpointer data);
double MemHistory[10] = {0};
double CPUHistory[10] = {0};
int total = 0;
int idle = 0;
int memall;

gboolean  draw_callback (GtkWidget *widget, cairo_t *cr, gpointer data)
{
    guint width, height;
    GtkStyleContext *context;
    context = gtk_widget_get_style_context (widget);

    width = gtk_widget_get_allocated_width (widget);
    height = gtk_widget_get_allocated_height (widget);
    gtk_render_background (context, cr, 0, 0, width, height);


    cairo_set_source_rgb(cr,0,0,0);
    cairo_set_line_width(cr,2.5);
    for(int i = 0;i < 9;i++){
        double x = width / 9 * i;
        double y = height - height * MemHistory[i];
        double x2 = width / 9  * (i +1);
        double y2 = height - height * MemHistory[i+1];
        cairo_move_to(cr,x,y);
        cairo_line_to(cr,x2,y2);
    }
    cairo_stroke(cr);
 return FALSE;
}

gboolean  draw_callback_cpu (GtkWidget *widget, cairo_t *cr, gpointer data)
{
    guint width, height;
    GtkStyleContext *context;
    context = gtk_widget_get_style_context (widget);

    width = gtk_widget_get_allocated_width (widget);
    height = gtk_widget_get_allocated_height (widget);

    cairo_set_source_rgb(cr,0,0,0);
    cairo_set_line_width(cr,2.5);
    gtk_render_background (context, cr, 0, 0, width, height);
    for(int i = 0;i < 9;i++){
        double x = width / 9 * i;
        double y = height - height * CPUHistory[i];
        double x2 = width / 9  * (i +1);
        double y2 = height - height * CPUHistory[i+1];
        cairo_move_to(cr,x,y);
        cairo_line_to(cr,x2,y2);
    }
    cairo_stroke(cr);
    return FALSE;
}

void fillinfolabel(){
    int infofd = open("/proc/cpuinfo",O_RDONLY);
    char buf[5000];
    read(infofd,buf,5000);

    string st(buf);
    close(infofd);
    infofd = open("/proc/version",O_RDONLY);
    int size = read(infofd,buf,5000);
    close(infofd);
    buf[size] = '\0';
    int pos = st.find("processor");
    st.replace(pos,9,"逻辑处理核心编号");

    pos = st.find("vendor_id");
    st.replace(pos,9,"处理器制造商");

    pos = st.find("cpu family");
    st.replace(pos,10,"CPU产品系列代号");

    pos = st.find("model");
    st.replace(pos,5,"CPU代数");

    pos = st.find("model name");
    st.replace(pos,10,"CPU名称");

    pos = st.find("stepping");
    st.replace(pos,8,"CPU版本");

    pos = st.find("microcode");
    st.replace(pos,9,"CPU微代码");

    pos = st.find("cpu MHz");
    st.replace(pos,7,"CPU主频（MHz)");

    pos = st.find("cache size");
    st.replace(pos,10,"CPU Cache大小");

    pos = st.find("physical id");
    st = st.substr(0,pos);
    st += "\nLinux版本：\t" + string(buf);
    gtk_label_set_text((GtkLabel *)Label,st.c_str());
}

void updatememinfo(){
    ifstream memstream("/proc/meminfo");
    string tmp;
    int total;
    int free;
    memstream >> tmp;
    memstream >> total;
    //cout << tmp;
    memstream >> tmp;
    memstream >> tmp;
    memstream >> tmp;
    memstream >> tmp;
    memstream >> tmp;
    memstream >> free;
    // cout << free;
    memstream.seekg(memstream.beg);
    gtk_progress_bar_set_fraction((GtkProgressBar *)MEMProg,(double)(total - free) / total);

    string out = "内存总量： " + to_string(total) + "kB\n" + "内存可用： " + to_string(free) + "kB\n";
    for(int i = 0;i <9;i++){
        MemHistory[i] = MemHistory[i + 1];
    }
    MemHistory[9] = (double)(total - free) / total;
    memall = total;
    gtk_widget_queue_draw_area(GTK_WIDGET(MEMGraph),0,0,300,200);
    gtk_label_set_text((GtkLabel *)MEMLabel,out.c_str());
}

void updateCPUinfo(){
    ifstream cpustream("/proc/stat");
    string tmp;
    int ntmp;
    cpustream >> tmp;
    int ttotal = 0,tidle = 0;
    for(int i = 0;i < 10;i++){
        cpustream >> ntmp;
        ttotal += ntmp;
        if(i == 3) tidle = ntmp;
    }
    if(total != 0 || idle != 0)
        gtk_progress_bar_set_fraction((GtkProgressBar *)CPUProg,1 - ((double)tidle - idle) / (double)(ttotal - total));
     for(int i = 0;i <9;i++){
        CPUHistory[i] = CPUHistory[i + 1];
    }
    CPUHistory[9] = 1 - ((double)tidle - idle) / (double)(ttotal - total);
    gtk_widget_queue_draw_area(GTK_WIDGET(CPUGraph),0,0,300,200);
    string out = "CPU总运行时间： " + to_string(ttotal) + "\n" + "CPU总空闲时间： " + to_string(tidle) + "\n";
    gtk_label_set_text((GtkLabel *)CPULabel,out.c_str());
    total = ttotal;
    idle = tidle;
    
}

void updateproc(){
    gtk_list_store_clear(ListStore);
    GtkTreeIter   iter;
    DIR *dp = opendir("/proc");
    struct dirent *dirp;
    struct stat statbuf;
    struct passwd *pwd;
    string cmd;
    string path;
    char temp[1000];
    while(( dirp = readdir(dp)) != NULL){
        if(strcmp(dirp->d_name,".") == 0 || strcmp(dirp->d_name,"..") == 0){
            continue;
        }
        path = "/proc/";
        path = path + string(dirp->d_name);
        lstat(path.c_str(),&statbuf);
        if(S_ISDIR(statbuf.st_mode)){
            if(isdigit((dirp->d_name)[0])){
                int pid = atoi(dirp->d_name);
                int uid;
                path = path + "/status";
                ifstream cpustream(path);
                cpustream.getline(temp,1000);
                cmd = string(temp);
                cmd = cmd.substr(6);
                for(int i = 1;i < 8;i++){
                    cpustream.getline(temp,1000);
                }                
                cpustream >> temp;
                cpustream >> uid;
                int memcur;
                for(int i = 0;i < 13;i++){
                    cpustream.getline(temp,1000);
                }
                cpustream >> temp;
                cpustream >> memcur;
                cpustream.close();
                double memcap = (double)memcur / memall * 100;
                pwd = getpwuid(uid);
                //cout << pid << " " << pwd->pw_name << " " << memcap << endl;
                gtk_list_store_append (ListStore, &iter);  /* Acquire an iterator */
                gtk_list_store_set(ListStore,&iter,
                    0,pid,
                    1,pwd->pw_name,
                    2,memcap,
                    3,cmd.c_str(),-1);
            }
        }
    }
    closedir(dp);

}

void update(){
    updatememinfo();
    updateCPUinfo();
}

void *worker(void * parameters){
    int i = 0;
    while(1){
        usleep(1000000);
        update();
        if(i % 10 == 0)updateproc();
        if(i == 0)    fillinfolabel();
        i++;
    }   
}

int main(int argc , char **argv)
{
    
    GtkApplication *app;
    int app_status;
    app = gtk_application_new("com.ProcDisp" , G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app , "activate" , G_CALLBACK(activate) , NULL);
    pthread_t id;
    pthread_create(&id,NULL,worker,NULL);
    app_status = g_application_run(G_APPLICATION(app) , 1 , argv);
    g_object_unref(app);
    return app_status;
}

 
static void activate(GtkApplication *app , gpointer data)
{
    GtkBuilder *builder;
    GObject *window;
 
    //创建一个 GtkBuilder 。
    builder = gtk_builder_new();
    //将描述 UI 的文件添加到 Builder 中。
    gtk_builder_add_from_file(builder , "UI.glade" , NULL);
 
    //可以用该函数获取 Builder 创建的对象，根据 example.ui 文件中设置的 object 的 id 来获取。
    window = gtk_builder_get_object(builder , "window1");
    gtk_widget_set_size_request((GtkWidget *)window,800,600);
    gtk_application_add_window(app , GTK_WINDOW(window));
    Label = gtk_builder_get_object(builder,"InfoLabel");
    MEMProg = gtk_builder_get_object(builder,"MEMprog");
    CPUProg = gtk_builder_get_object(builder,"CPUprog");
    MEMLabel = gtk_builder_get_object(builder,"Memlabel");
    CPULabel = gtk_builder_get_object(builder,"CPUlabel");
    MEMGraph = gtk_builder_get_object(builder,"MEMgraph");
    CPUGraph = gtk_builder_get_object(builder,"CPUgraph");
    ListStore = (GtkListStore *)gtk_builder_get_object(builder,"proclist");
    g_signal_connect (MEMGraph, "draw",
                    G_CALLBACK (draw_callback), NULL);
    g_signal_connect (CPUGraph, "draw",
                    G_CALLBACK (draw_callback_cpu), NULL);
    gtk_widget_show(GTK_WIDGET(window));
}