#include <gtk/gtk.h>
#include <pthread.h>
#include "pv.h"
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

char *shmaddr;
char buffer[100];

GObject *Label;
static void activate(GtkApplication *app , gpointer data);
 
int buf2e,buf2f,keyshm2;
void set_label_text(const char *text){
    gtk_label_set_text((GtkLabel *)Label,text);
}

void worker(void *s){
    sleep(1);
    int fd;
    fd = open(s,O_WRONLY | O_CREAT,0666);
    int rd;
    while(1){
        sleep(1);
        P(buf2f);
        rd = shmaddr[0];
        for(int i = 0;i <rd;i++){
            buffer[i] = shmaddr[i + 1];
        }
        V(buf2e);
        write(fd,buffer,rd);
        char text[10];
        sprintf(text,"%d",rd);
        set_label_text(text);
        if(rd == 0) break;
    }
    close(fd);
    exit(0);
}

int main(int argc , char **argv)
{
    buf2e = semget(BUF2E,1,IPC_CREAT);
    buf2f = semget(BUF2F,1,IPC_CREAT);
    printf("Dest file is %s\n",argv[1]);
    GtkApplication *app;
    int app_status;
    keyshm2 = shmget(KEYSHM2,0,IPC_CREAT);
    shmaddr = shmat(keyshm2,NULL,0);
    app = gtk_application_new("com.GUItest3" , G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app , "activate" , G_CALLBACK(activate) , NULL);
    pthread_t id;
    pthread_create(&id,NULL,worker,argv[1]);
    app_status = g_application_run(G_APPLICATION(app) , 1 , argv);
    g_object_unref(app);
    shmdt(shmaddr);
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
    gtk_window_set_title((GtkWindow *)window,"third");
    gtk_application_add_window(app , GTK_WINDOW(window));
    Label = gtk_builder_get_object(builder,"label1");
    gtk_widget_show(GTK_WIDGET(window));
}