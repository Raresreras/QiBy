#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "stdio.h"

#include <QLabel>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>
#include <QDebug>

QString vault;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("QiBy - Password manager");

    // Connect database
    DB_Connection = QSqlDatabase::addDatabase("QSQLITE");
    DB_Connection.setDatabaseName(QCoreApplication::applicationDirPath() + "/data.db");

    if (DB_Connection.open()){
        qDebug() << "Database is Connected";
        /*QSqlQuery checkQuery;
        checkQuery.prepare("create table if not exists vault1(id number(2),username varchar(32),password varchar(32))");
        checkQuery.exec();
        if (checkQuery.exec())
            qDebug() <<"Password table reseted";*/
    }
    else{
        qDebug() << "Connection failed";
        qDebug() << DB_Connection.lastError();
    }
    qDebug() << QCoreApplication::applicationDirPath();

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_comboBox_currentTextChanged(const QString &arg1)
{
    vault = ui->comboBox->currentText();
    ui->label->setText(vault);
}

void MainWindow::on_pushButton_released()
{
    QString username = ui->usernameInput->text();
    QString password = ui->passwordInput->text();

    //This works, need to add encryption and clean the code up
    DB_Connection.open();
    QSqlDatabase::database().transaction();
    QSqlQuery QueryInsertData(DB_Connection);
    QueryInsertData.prepare("INSERT INTO vault1(username, password) VALUES(:username, :password)");
    QueryInsertData.bindValue(":username", username);
    QueryInsertData.bindValue(":password", password);
    if (QueryInsertData.exec())
        qDebug() << "Query executed";
    else
        qDebug() << QueryInsertData.lastError();
    QSqlDatabase::database().commit();
    DB_Connection.close();
}


void MainWindow::on_pushButton_2_released()
{
    //TODO!!! clean code and add decryption
    DB_Connection.open();
    QSqlDatabase::database().transaction();
    QSqlQuery QueryExtractData(DB_Connection);
    QueryExtractData.prepare("SELECT * FROM vault1");

    int rownum = 0;
    if (QueryExtractData.exec()){
        ui->tableWidget->setRowCount(QueryExtractData.size());
        while (QueryExtractData.next()){
            ui->tableWidget->setItem(rownum, 1, new QTableWidgetItem(QueryExtractData.value("username").toString()));
            ui->tableWidget->setItem(rownum, 2, new QTableWidgetItem(QueryExtractData.value("password").toString()));
        }

    }
    else{
        qDebug() << QueryExtractData.lastError();
    }
    QSqlDatabase::database().commit();
    DB_Connection.close();
}

