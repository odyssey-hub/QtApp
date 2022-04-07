#ifndef SYSTEMWINDOW_H
#define SYSTEMWINDOW_H

#include <QDialog>
#include <QTableWidgetItem>

namespace Ui {
class SystemWindow;
}

class SystemWindow : public QDialog
{
    Q_OBJECT

public:
    explicit SystemWindow(QWidget *parent = 0);
    ~SystemWindow();

private:
    Ui::SystemWindow *ui;
    QTableWidgetItem *item;
    void sys_version();
    void net_info();
    void resolution_info();
};

#endif // SYSTEMWINDOW_H
