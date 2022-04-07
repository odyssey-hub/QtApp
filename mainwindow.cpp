#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "debugwindow.h"
#include <QTimer>
#include <QTime>
#include <QMessageBox>
#include <QCloseEvent>
#include <QFileDialog>

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <syslog.h>




MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("Вариант 6");
    setWindowIcon(QIcon(":/icon/icon/system01.ico"));
    createTimer();
    debug_mode = false;
    readDirectory(openDirectory());
    connectDB();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{//переопределение выхода
    int quit = QMessageBox::warning(this,"Выход из приложения","Вы действительно хотите выйти?",tr("Да"),tr("Нет"));
    if (quit == 0) event->accept();
    else event->ignore();
}


void MainWindow::createTimer()
{//создание таймера
    tmr = new QTimer(this);
    tmr->setInterval(500);
    tmr->start();
    connect(tmr,SIGNAL(timeout()),this,SLOT(updateTime()));
    connect(ui->TimerButton,SIGNAL(clicked()),this,SLOT(timerOff()));
}

void MainWindow::connectDB()
{//подключение к БД
    //считываем расположение файла БД
    QFile f_dbpath(QDir::currentPath()+QString("/db_path.txt"));
    QString dbpath;
     //если первый запуск,то храним в текущей директории
    if (!f_dbpath.open(QIODevice::ReadOnly)) dbpath= QDir::currentPath()+QString("/TestDB.db");
    else{
        QTextStream in(&f_dbpath);
        dbpath = in.readLine();
        if (dbpath == "") dbpath= QDir::currentPath()+QString("/TestDB.db");
    }
    testDB = QSqlDatabase::addDatabase("QSQLITE");
    QFile dbFile(dbpath);
    if (!dbFile.exists()){//если файл БД не существует
        testDB.setDatabaseName(dbpath);
        if (!testDB.open()) QMessageBox::critical(this,"Ошибка","Не удалось подключиться к БД");
        QSqlQuery *q = new QSqlQuery(testDB);
        q->exec("CREATE TABLE CodeTable (Код VARCHAR(10), Описание VARCHAR(40))");
        fillCodeTable();
        q->exec("CREATE TABLE ActionTable (id INTEGER PRIMARY KEY AUTOINCREMENT, Код VARCHAR(40), Время VARCHAR(40), Результат VARCHAR(40))");
        QMessageBox::information(this,"Успех","Файл БД и таблицы успешно созданы");
    } else {
        testDB.setDatabaseName(dbpath);
        if (!testDB.open()) QMessageBox::critical(this,"Ошибка","Не удалось подключиться к БД");
    }
    q = new QSqlQuery(testDB);
    strquery = "INSERT INTO ActionTable (Код, Время, Результат) VALUES ('%1', '%2', '%3')";
}

void MainWindow::fillCodeTable()
{//заполнение таблицы кодов
    QSqlQuery *q = new QSqlQuery();
    QString strquery = "INSERT INTO CodeTable (Код, Описание) VALUES ('%1', '%2')";
    q->exec(strquery.arg("M1","Меню->О программе"));
    q->exec(strquery.arg("M2","Меню->Система"));
    q->exec(strquery.arg("SCH","Сис.вызов chdir"));
    q->exec(strquery.arg("ST","Сис.вызов mktemp"));
    q->exec(strquery.arg("SF","Сис.вызов mkfifo"));
    q->exec(strquery.arg("SCR","Сис.вызов creat"));
    q->exec(strquery.arg("SD","Сис.вызов unlink"));
    q->exec(strquery.arg("SOP","Сис.вызов opendir"));
    q->exec(strquery.arg("SRD","Сис.вызов readdir"));
}

void MainWindow::configureTableFiles()
{//настройки таблицы
    ui->tableFiles->setRowCount(0);
    ui->tableFiles->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableFiles->horizontalHeader()->setStretchLastSection(true);
    ui->tableFiles->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableFiles->resizeColumnsToContents();
}

char* MainWindow::openDirectory()
{//получение текущей директории
    char *buff = 0;
    char *path = getcwd(buff,PATH_MAX);
    if (path == NULL){
        switch (errno) {
        case EINVAL:QMessageBox::critical(this,"Ошибка вызова getcwd","Буфер не пустой");break;
        case ERANGE:QMessageBox::critical(this,"Ошибка вызова getcwd","Путь не помещается в буфер");break;
        case EACCES:QMessageBox::critical(this,"Ошибка вызова getcwd","Нет прав на чтение одного из компонентов пути файла");break;
        default:QMessageBox::critical(this,"Ошибка вызова getcwd","Неизвестная ошибка");  break;
        }
        if (debug_mode) q->exec(strquery.arg("SGD",QTime::currentTime().toString(),"Ошибка"+QString(errno)));
        return NULL;
    }
    ui->currdLabel->setText(QString(path));
    if (debug_mode) q->exec(strquery.arg("SGD",QTime::currentTime().toString(),"Успех"));
    return path;
}

void MainWindow::readDirectory(char *path)
{//чтение директории
    configureTableFiles();
    //открываем директорию
    DIR *dp = opendir(path);
    if (dp == NULL){
        switch(errno) {
        case EACCES: QMessageBox::critical(this,"Ошибка вызова opendir","Доступ запрещен");break;
        case ENOTDIR: QMessageBox::critical(this,"Ошибка вызова opendir","Это не каталог"); break;
        case ENFILE: QMessageBox::critical(this,"Ошибка вызова opendir","Система использует слишком много открытых потоков");break;
        case ENOENT: QMessageBox::critical(this,"Ошибка вызова opendir","Каталога не существует или введена пустая строка");break;
        case ENOMEM: QMessageBox::critical(this,"Ошиб(ка вызова opendir","Недостаточно памяти для выполнения операции"); break;
        case EMFILE: QMessageBox::critical(this,"Ошибка вызова opendir","Процесс использует слишком много открытых потоков");break;
        default:QMessageBox::critical(this,"Ошибка вызова opendir","Неизвестная ошибка"); break;
        }
        if (debug_mode) q->exec(strquery.arg("SOP",QTime::currentTime().toString(),"Ошибка"+QString(errno)));
        return;
    }
    if (debug_mode) q->exec(strquery.arg("SOP",QTime::currentTime().toString(),"Успех"));
    //читаем директорию
    struct dirent *de;
    int i=0;
    for(de = readdir(dp); de != NULL; de = readdir(dp)){
       ui->tableFiles->setRowCount(ui->tableFiles->rowCount()+1);
       item = new QTableWidgetItem();
       item->setText(QString::number(de->d_ino));
       ui->tableFiles->setItem(i,0,item);
       item = new QTableWidgetItem();
       switch (de->d_type) {
       case DT_BLK:item->setText("Блочный файл устройств"); break;
       case DT_CHR:item->setText("Символьный файл устройств"); break;
       case DT_DIR:item->setText("Директория"); break;
       case DT_FIFO:item->setText("Именованный канал"); break;
       case DT_LNK:item->setText("Символическая ссылка"); break;
       case DT_REG:item->setText("Обычный файл"); break;
       case DT_SOCK:item->setText("Сокет"); break;
       default:item->setText("Нейзвестный тип");
           break;
       }
       ui->tableFiles->setItem(i,1,item);
       item = new QTableWidgetItem();
       item->setText(QString(de->d_name));
       ui->tableFiles->setItem(i,2,item);
       i++;
    }
    ui->tableFiles->resizeColumnsToContents();
    closedir(dp);

    if (debug_mode) q->exec(strquery.arg("SRD",QTime::currentTime().toString(),"Успех"));
}

void MainWindow:: timerOn()
{//включить таймер
    tmr->start();
    disconnect(ui->TimerButton,SIGNAL(clicked()),this,SLOT(timerOn()));
    connect(ui->TimerButton,SIGNAL(clicked()),this,SLOT(timerOff()));
}

void MainWindow:: timerOff()
{//выключить таймер
    tmr->stop();
    disconnect(ui->TimerButton,SIGNAL(clicked()),this,SLOT(timerOff()));
    connect(ui->TimerButton,SIGNAL(clicked()),this,SLOT(timerOn()));
}

void MainWindow:: updateTime()
{//обновить таймер
     ui->statusBar->showMessage(QTime::currentTime().toString());
}

void MainWindow::on_action_triggered()//информация о системе
{
    if (!isWindowOpen){
        syswindow = new SystemWindow(this);
        syswindow->exec();
    }

    if (debug_mode) q->exec(strquery.arg("M2",QTime::currentTime().toString(),"Успех"));
}

void MainWindow::on_action_2_triggered()//информация о разработчике
{
    QMessageBox::information(this,"Информация о разработчике","Бушуев Максим ВИП-208 \nВариант 6");

    if (debug_mode) q->exec(strquery.arg("M1",QTime::currentTime().toString(),"Успех"));
}

void MainWindow::on_action_3_triggered()//выйти из программы
{
    int quit = QMessageBox::warning(this,"Выход из приложения","Вы действительно хотите выйти?",tr("Да"),tr("Нет"));
    if (quit == 0) qApp->exit();
    else return;

}


void MainWindow::on_changeDirButton_clicked()
{ // сменить директорию
    QFileDialog *dirdialog = new QFileDialog;
    dirdialog->setFileMode(QFileDialog::DirectoryOnly);
    QString qstr = dirdialog->getExistingDirectory(this,"Выбрать директорию");
    if (qstr == NULL) return;
    QByteArray ba = qstr.toLocal8Bit();
    char *path = ba.data();
    int err = chdir(path);
    if (err == -1){
        switch (errno) {
            case EFAULT:QMessageBox::critical(this,"Ошибка вызова chdir","Путь указывает за пределы адресного пространства"); break;
            case ENAMETOOLONG:QMessageBox::critical(this,"Ошибка вызова chdir","Путь слишком длинный"); break;
            case ENOENT:QMessageBox::critical(this,"Ошибка вызова chdir","Директории не существует"); break;
            case ENOMEM: QMessageBox::critical(this,"Ошибка вызова chdir","Ядру не хватило памяти");break;
            case ENOTDIR: QMessageBox::critical(this,"Ошибка вызова chdir","Это не директория");break;
            case EACCES: QMessageBox::critical(this,"Ошибка вызова chdir","Доступ запрещен");break;
            case ELOOP:QMessageBox::critical(this,"Ошибка вызова chdir","Зацикленная символическая ссылка"); break;
            case EIO: QMessageBox::critical(this,"Ошибка вызова chdir","Ошибка ввода");break;
        default:QMessageBox::critical(this,"Ошибка вызова chdir","Неизвестная ошибка");
            break;
        }
        if (debug_mode) q->exec(strquery.arg("SCH",QTime::currentTime().toString(),"Ошибка "+QString(errno)));
        return;
    }
    readDirectory(openDirectory());

    if (debug_mode) q->exec(strquery.arg("SCH",QTime::currentTime().toString(),"Успех"));
}

void MainWindow::on_makeFIFO_clicked()
{ //создать FIFO
    QFileDialog *fifodialog = new QFileDialog;
    fifodialog->setFileMode(QFileDialog::AnyFile);
    QString qstr = fifodialog->getSaveFileName(this,"Выберите, куда сохранить");
    if (qstr == NULL) return;
    QByteArray ba = qstr.toLocal8Bit();
    char *path = ba.data();
    int err = mkfifo(path,0777);
    if (err == -1){
        switch (errno) {
        case EACCES:QMessageBox::critical(this,"Ошибка вызова mkfifo","Доступ запрещен на один каталогов");break;
        case EEXIST:QMessageBox::critical(this,"Ошибка вызова mkfifo","Канал уже существует");break;
        case ENAMETOOLONG:QMessageBox::critical(this,"Ошибка вызова mkfifo","Слишком длинный путь");break;
        case ENOENT:QMessageBox::critical(this,"Ошибка вызова mkfifo","Компонента каталога пути не существует");break;
        case ENOSPC:QMessageBox::critical(this,"Ошибка вызова mkfifo","Недостаточно места на дисковом пространстве");break;
        case ENOTDIR:QMessageBox::critical(this,"Ошибка вызова mkfifo","Это не каталог");break;
        case EROFS:QMessageBox::critical(this,"Ошибка вызова mkfifo","Файловая система доступна только для чтения");break;
        default:QMessageBox::critical(this,"Ошибка вызова mkfifo","Неизвестная ошибка"); break;
        }
        if (debug_mode) q->exec(strquery.arg("SF",QTime::currentTime().toString(),"Ошибка"+QString(errno)));
        return;
    }
    QMessageBox::information(this,"Канал создан","Канал FIFO успешно создан по следующему пути:"+qstr);

    if (debug_mode) q->exec(strquery.arg("SF",QTime::currentTime().toString(),"Успех"));
}

void MainWindow::on_createUFile_clicked()
{//создать файл с уникальным именем
    char templat[18] = "bushuevMVXXXXXX";
    char *uniqname = mktemp(templat);
    if (uniqname == NULL) {
        QMessageBox::critical(this,"Ошибка вызова mktemp","Неудалось создать шаблон");
        if (debug_mode) q->exec(strquery.arg("ST",QTime::currentTime().toString(),"Ошибка"));
        return;
    }
    if (debug_mode) q->exec(strquery.arg("ST",QTime::currentTime().toString(),"Успех"));
    QFileDialog *dirdialog = new QFileDialog;
    dirdialog->setFileMode(QFileDialog::DirectoryOnly);
    QString qstr = dirdialog->getExistingDirectory(this,"Выбрать директорию");
    if (qstr == NULL) return;
    qstr += '/'+QString(uniqname);
    QByteArray ba = qstr.toLocal8Bit();
    char *path = ba.data();
    if (creat(path,0777) == -1){
        switch (errno) {
            case EEXIST:QMessageBox::critical(this,"Ошибка вызова creat","Файл уже существует");break;
            case EACCES:QMessageBox::critical(this,"Ошибка вызова creat","Доступ на один из каталогов в пути запрещен");break;
            case ENOMEM:QMessageBox::critical(this,"Ошибка вызова creat","Недостаточно памяти в системе");break;
            case ENAMETOOLONG:QMessageBox::critical(this,"Ошибка вызова creat","Путь слишком длинный");break;
        default:QMessageBox::critical(this,"Ошибка вызова creat","Неизвестная ошибка");break;
            if (debug_mode) q->exec(strquery.arg("SCR",QTime::currentTime().toString(),"Ошибка"+QString(errno)));
        }
        return;
    }
    QMessageBox::information(this,"Файл создан","Файл успешно создан по адресу:"+QString(path));
    uniq_file_path = path;
    ui->createUFile->setEnabled(false);
    ui->deleteUFile->setEnabled(true);

     if (debug_mode) q->exec(strquery.arg("SCR",QTime::currentTime().toString(),"Успех"));
}

void MainWindow::on_deleteUFile_clicked()
{//удалить файл с уникальным именем
    if (unlink(uniq_file_path) == -1){
        switch (errno) {
            case EACCES:QMessageBox::critical(this,"Ошибка вызова unlink","Доступ на один из каталогов в пути запрещен");break;
            case EPERM:QMessageBox::critical(this,"Ошибка вызова unlink","Удаление файла запрещено"); break;
            case ENOMEM:QMessageBox::critical(this,"Ошибка вызова unlink","Недостаточно памяти");break;
            case EROFS:QMessageBox::critical(this,"Ошибка вызова unlink","Файл доступен только для чтения"); break;
        default:QMessageBox::critical(this,"Ошибка вызова unlink","Неизвестная ошибка"); break;
        }
         if (debug_mode) q->exec(strquery.arg("SD",QTime::currentTime().toString(),"Ошибка"+QString(errno)));
        return;
    }
    QMessageBox::information(this,"Файл удалён","Файл успешно удалён");
    ui->createUFile->setEnabled(true);
    ui->deleteUFile->setEnabled(false);

     if (debug_mode) q->exec(strquery.arg("SD",QTime::currentTime().toString(),"Успех"));
}



void MainWindow::on_menuDebug_onoff_triggered()
{//включение/выключение режима отладки
    if (debug_mode){
        openlog("SemestrVar6",LOG_PID|LOG_CONS,LOG_USER);
        syslog(LOG_ERR,"Отладочный режим выключен");
        closelog();
        debug_mode = false;
        QMessageBox::information(this,"Отладочный режим","Отладочный режим выключен");
    } else {
        openlog("SemestrVar6",LOG_PID|LOG_CONS,LOG_USER);
        syslog(LOG_ERR,"Отладочный режим выключен");
        closelog();
        debug_mode = true;
        QMessageBox::information(this,"Отладочный режим","Отладочный режим включен");
    }
}


void MainWindow::on_MenuDB_triggered()
{//окно БД
        if (!isWindowOpen){
            debugwindow = new DebugWindow(this);
            debugwindow->show();
            isWindowOpen = true;
        }
}
