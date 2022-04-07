#include "systemwindow.h"
#include "ui_systemwindow.h"
#include "mainwindow.h"
#include <QMessageBox>
#include <QDesktopWidget>

#include <sys/utsname.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <ifaddrs.h>
#include <linux/if_link.h>
#include <unistd.h>

SystemWindow::SystemWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SystemWindow)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle("Информация о системе");
    sys_version();
    net_info();
    resolution_info();

}

SystemWindow::~SystemWindow()
{
    delete ui;
}

void SystemWindow::sys_version()
{//информация о системе
    struct utsname sysname;
    uname(&sysname);
    if (errno == EFAULT)
    {
        QMessageBox::critical(this,"Ошибка","Ошибка системного вызова uname");
        return;
    }
    ui->versionLabel->setText(QString::fromStdString(std::string(sysname.version)));
}

void SystemWindow::net_info()
{//информация о сетевых интерфейсах
    ui->tableNetwork->setEditTriggers(QAbstractItemView::NoEditTriggers);
    struct ifaddrs *ifaddr,*ifa;
    int family,s;
    char host[NI_MAXHOST];
    if (getifaddrs(&ifaddr) == -1)
    {
        QMessageBox::critical(this,"Ошибка","Ошибка системного вызова getifaddrs");
        return;
    }
    int i=0;
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next){
        if (ifa->ifa_addr == NULL) continue;
        family = ifa->ifa_addr->sa_family;
        if (family == AF_INET || family == AF_INET6){
            ui->tableNetwork->setRowCount(ui->tableNetwork->rowCount()+1);
            item = new QTableWidgetItem;
            item->setText(QString(ifa->ifa_name));
            ui->tableNetwork->setItem(i,0,item);
            item = new QTableWidgetItem;
            item->setText((family == AF_INET) ? "AF_INET":"AF_INET6");
            ui->tableNetwork->setItem(i,1,item);
            s = getnameinfo(ifa->ifa_addr, (family == AF_INET) ? sizeof(struct sockaddr_in): sizeof(struct sockaddr_in6),host,NI_MAXHOST,NULL,0,NI_NUMERICHOST);
            if (s == 0){
                item = new QTableWidgetItem;
                item->setText(QString(host));
                ui->tableNetwork->setItem(i,2,item);
            }
            if (ifa->ifa_netmask != NULL){
                s = getnameinfo(ifa->ifa_netmask, (family == AF_INET) ? sizeof(struct sockaddr_in): sizeof(struct sockaddr_in6),host,NI_MAXHOST,NULL,0,NI_NUMERICHOST);
                if (s == 0){
                    item = new QTableWidgetItem;
                    item->setText(QString(host));
                    ui->tableNetwork->setItem(i,3,item);
                }
            }
             s = getnameinfo(ifa->ifa_broadaddr, (family == AF_INET) ? sizeof(struct sockaddr_in): sizeof(struct sockaddr_in6),host,NI_MAXHOST,NULL,0,NI_NUMERICHOST);
             if (s == 0){
                 item = new QTableWidgetItem;
                 item->setText(QString(host));
                 ui->tableNetwork->setItem(i,4,item);
             } else {
                 item = new QTableWidgetItem;
                 item->setText("-");
                 ui->tableNetwork->setItem(i,4,item);
             }
             i++;
        }
    }
    freeifaddrs(ifa);
    ui->tableNetwork->resizeColumnsToContents();
}

void SystemWindow::resolution_info()
{//текущее разрешение экранак
    QDesktopWidget *desk = new QDesktopWidget();
    QString width = QString::number(desk->screenGeometry().width());
    QString height = QString::number(desk->screenGeometry().height());
    ui->resolutionLabel->setText(width+"x"+height);
}
