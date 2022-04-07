#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtSql>
#include "systemwindow.h"
#include "debugwindow.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    bool debug_mode;
    bool isWindowOpen;
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QTimer *tmr;
    QTableWidgetItem *item;
    SystemWindow *syswindow;
    DebugWindow *debugwindow;
    QSqlDatabase testDB;
    QSqlQuery *q;
    QSqlQuery q2;
    QString strquery;
    char *uniq_file_path;
    bool w_isOpen;

    void createTimer();
    char* openDirectory();
    void readDirectory(char *path);
    void fillCodeTable();
    void connectDB();
    void configureTableFiles();
    virtual void closeEvent(QCloseEvent *event);
private slots:
    void updateTime();
    void timerOn();
    void timerOff();

    void on_action_triggered();
    void on_action_2_triggered();
    void on_action_3_triggered();
    void on_changeDirButton_clicked();
    void on_makeFIFO_clicked();
    void on_createUFile_clicked();
    void on_deleteUFile_clicked();
    void on_menuDebug_onoff_triggered();
    void on_MenuDB_triggered();
};

#endif // MAINWINDOW_H
