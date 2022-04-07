#ifndef DEBUGWINDOW_H
#define DEBUGWINDOW_H

#include <QDialog>
#include <QtSql>

namespace Ui {
class DebugWindow;
}

class DebugWindow : public QDialog
{
    Q_OBJECT

public:
    explicit DebugWindow(QWidget *parent = 0);
    ~DebugWindow();

private slots:
    void on_delTableActions_button_clicked();

    void on_sort_button_clicked();

    void on_deleteRecord_button_clicked();

    void on_searchLine_editingFinished();

    void on_altpath_Button_clicked();

private:
    Ui::DebugWindow *ui;
    QSqlDatabase testDB;
    QString debug_stdquery;
    QSqlQuery q;
    QSqlTableModel *modelActions;
    QSqlTableModel *modelCode;
    QSqlQueryModel *debugActionsModel;
    void createModelCode();
    void createModelAction();
    void configureTables();
    void createModels();
    void createJoinModel();
    void sortActionModel();
    void sortJoinModel();
    void searchActionModel();
    void searchJoinModel();
};

#endif // DEBUGWINDOW_H
