#include "mainwindow.h"
#include <iostream>
#include <QApplication>

int main(int argc, char *argv[])
{
   if (argc > 1){
        if (strcmp(argv[1],"-h") == 0){
            std::cout<<"Разработчик:Бушуев Максим ВИП-208"<<std::endl;
            std::cout<<"Вариант 6"<<std::endl;
            std::cout<<"Версия операционной системы:" <<QSysInfo::kernelVersion().toStdString()<<std::endl;
            std::cout<<"Тип ядра операционной системы:" <<QSysInfo::kernelType().toStdString()<<std::endl;
            return 0;
        }
        else std::cout<<"Возможен только один ключ: -h"<<std::endl;
    }
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
