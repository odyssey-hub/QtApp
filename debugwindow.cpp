#include "debugwindow.h"
#include "mainwindow.h"
#include "ui_debugwindow.h"
#include <QMessageBox>
#include <QFileDialog>

class Delegate : public QItemDelegate
{
public:
    Delegate(QObject * parent = 0) : QItemDelegate(parent) {}
    virtual QWidget * createEditor (QWidget *, const QStyleOptionViewItem &, const QModelIndex &) const
    {
        return 0;
    }
};

DebugWindow::DebugWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DebugWindow)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle("База данных отладки");
    createModels();
}

DebugWindow::~DebugWindow()
{
    MainWindow *mw = qobject_cast<MainWindow*> (parent());
    mw->isWindowOpen=false;
    delete ui;
}


void DebugWindow::createModels()
{
    MainWindow *mw = qobject_cast<MainWindow*> (parent());
    if (mw->debug_mode) createJoinModel();
    else createModelAction();
    createModelCode();
    configureTables();
}

void DebugWindow::createModelAction()
{//основная таблица (код,результат,время)
    modelActions = new QSqlTableModel(this);
    modelActions->setTable("ActionTable");
    modelActions->select();
    modelActions->setEditStrategy(QSqlTableModel::OnFieldChange);
    ui->tableActions->setModel(modelActions);
    ui->tableActions->setColumnHidden(0,true);
}

void DebugWindow::createModelCode()
{//таблица (код, описание)
    modelCode = new QSqlTableModel(this);
    modelCode->setTable("CodeTable");
    modelCode->select();
    modelCode->setEditStrategy(QSqlTableModel::OnFieldChange);
    ui->tableCode->setModel(modelCode);
}

void DebugWindow::createJoinModel()
{
    debugActionsModel = new QSqlQueryModel(this);
    ui->comboSortField->setItemText(0,"Описание");
    ui->comboSearchField->setItemText(0,"Описание");
    ui->comboSearchField->setEnabled(false);
    ui->comboSortField->setEnabled(false);
    ui->delTableActions_button->setEnabled(false);
    debug_stdquery = "SELECT CodeTable.Описание, ActionTable.Время, ActionTable.Результат FROM ActionTable ";
    debug_stdquery += "JOIN CodeTable ON CodeTable.Код = ActionTable.Код ";
    q.exec(debug_stdquery);
    debugActionsModel->setQuery(q);
    ui->tableActions->setModel(debugActionsModel);
}

void DebugWindow::configureTables()
{
    ui->tableCode->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableCode->horizontalHeader()->setStretchLastSection(true);
    ui->tableCode->setItemDelegateForColumn(0,new Delegate());
    ui->tableActions->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableActions->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableActions->horizontalHeader()->setStretchLastSection(true);
    ui->tableActions->resizeColumnsToContents();
}

void DebugWindow::on_delTableActions_button_clicked()
{//очистка таблицы
    QSqlQuery *q = new QSqlQuery();
    q->exec("DELETE FROM ActionTable");
    modelActions->select();
}

void DebugWindow::on_sort_button_clicked()
{//сортировка
   MainWindow *mw = qobject_cast<MainWindow*> (parent());
   if (!mw->debug_mode) sortActionModel();
   else sortJoinModel();
}

void DebugWindow::sortActionModel()
{
    if (ui->comboSortOrder->currentIndex() == 0) modelActions->setSort(ui->comboSortField->currentIndex(),Qt::DescendingOrder);
    else modelActions->setSort(ui->comboSortField->currentIndex(),Qt::AscendingOrder);
    modelActions->select();
}

void DebugWindow::sortJoinModel()
{
    QString qs = q.lastQuery();
    if (ui->searchLine->text() != "") {
       qs.remove("ORDER BY CodeTable.Описание ASC",Qt::CaseInsensitive);
       qs.remove("ORDER BY CodeTable.Описание DESC",Qt::CaseInsensitive);
    } else {
       qs =debug_stdquery;
    }
    qs += " ORDER BY CodeTable.Описание ";
    if (ui->comboSortOrder->currentIndex() == 0) qs += "ASC";
    else qs+= "DESC";
    q.exec(qs);
    debugActionsModel->setQuery(q);
}

void DebugWindow::on_deleteRecord_button_clicked()
{//удаление
    //получаем id записи
    int row = ui->tableActions->currentIndex().row();
    QModelIndex index = modelActions->index(row,0);
    QVariant data = modelActions->data(index);
    int id  = data.toInt();
    //удаляем из базы данных по id
    QSqlQuery *q = new QSqlQuery();
    q->prepare("DELETE FROM ActionTable WHERE id=?");
    q->addBindValue(id);
    q->exec();
    modelActions->select();
}

void DebugWindow::on_searchLine_editingFinished()
{//поиск
    MainWindow *mw = qobject_cast<MainWindow*> (parent());
    if (!mw->debug_mode) searchActionModel();
    else searchJoinModel();
}

void DebugWindow::searchActionModel()
{
    QString search_value = ui->searchLine->text();
    if (search_value == "") {
        modelActions->setFilter("");
        return;
    }
    QString search_field = ui->comboSearchField->currentText();
    if (ui->checkSearch->isChecked()){//по части
        QString filter_string = search_field+" LIKE "+"'%"+search_value+"%'";
        modelActions->setFilter(filter_string);
    } else {//полностью
        QString filter_string = search_field+" = '"+search_value+"'";
        modelActions->setFilter(filter_string);
    }
}

void DebugWindow::searchJoinModel()
{
    QString search_value = ui->searchLine->text();
    if (search_value == "") {
        debugActionsModel->setQuery(debug_stdquery);
        return;
    }
    QString qs = debug_stdquery;
    if (ui->checkSearch->isChecked()){//по части
        qs += " WHERE CodeTable.Описание LIKE '%"+search_value+"%'";
    } else {//полностью
        qs += " WHERE CodeTable.Описание = '"+search_value+"'";
    }
    q.exec(qs);
    debugActionsModel->setQuery(q);
}

void DebugWindow::on_altpath_Button_clicked()
{//альтернативное расположение файла БД
    //считываем файл, который хранит путь к файлу базы данных
    QFile f_dbpath(QDir::currentPath()+QString("/db_path.txt"));
    f_dbpath.open(QIODevice::ReadOnly);
    QTextStream in(&f_dbpath);
    QString dbpath = in.readLine();
    if (dbpath == "") dbpath= QDir::currentPath()+QString("/TestDB.db");
    f_dbpath.close();
    QFile dbFile(dbpath);
    //перемещаем файл БД
    QFileDialog *dirdialog = new QFileDialog;
    dirdialog->setFileMode(QFileDialog::DirectoryOnly);
    QString newpath = dirdialog->getExistingDirectory(this,"Выберите директорию");
    if (newpath == NULL) return;
    newpath += "/TestDB.db";
    dbFile.rename(newpath);
    //записываем в файл новое расположение
    f_dbpath.setFileName(QDir::currentPath()+QString("/db_path.txt"));
    if (f_dbpath.open(QIODevice::WriteOnly)){
        QTextStream out(&f_dbpath);
        out<<newpath;
    }
    QMessageBox::information(this,"Новое расположение БД","Файл БД теперь находится по адресу:"+newpath);
    //переподключаемся к базе данных
    QSqlDatabase::database().close();
    QSqlDatabase::database().setDatabaseName(newpath);
    QSqlDatabase::database().open();
}




