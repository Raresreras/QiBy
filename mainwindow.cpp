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
    //input button
    QString username = ui->usernameInput->text();
    QString password = ui->passwordInput->text();
    QString platform = ui->platformInput->text();

    //This works, need to add encryption and clean the code up
    DB_Connection.open();
    QSqlDatabase::database().transaction();
    QSqlQuery QueryInsertData(DB_Connection);
    QueryInsertData.prepare("INSERT INTO vault1(username, password, platform) VALUES(:username, :password, :platform)");
    QueryInsertData.bindValue(":username", username);
    QueryInsertData.bindValue(":password", password);
    QueryInsertData.bindValue(":platform", platform);
    if (QueryInsertData.exec())
        qDebug() << "Query executed";
    else
        qDebug() << QueryInsertData.lastError();
    QSqlDatabase::database().commit();
    DB_Connection.close();
}


void MainWindow::on_pushButton_2_released()
{
    //update button
    qDebug() << ui->keyInput->text();

    //TODO!!! clean code and add decryption
    DB_Connection.open();
    QSqlDatabase::database().transaction();
    QSqlQuery QueryExtractData(DB_Connection);
    QueryExtractData.prepare("SELECT * FROM vault1");

    ui->tableWidget->setRowCount(0); //!TODO! fix QueryExtractData.size() returning only -1
    ui->tableWidget->setRowCount(32);
    int rownum = 0;
    if (QueryExtractData.exec()){
        //ui->tableWidget->setRowCount(QueryExtractData.size());
        while (QueryExtractData.next()){
            ui->tableWidget->setItem(rownum, 0, new QTableWidgetItem(QueryExtractData.value("username").toString()));
            ui->tableWidget->setItem(rownum, 1, new QTableWidgetItem(QueryExtractData.value("password").toString()));
            ui->tableWidget->setItem(rownum, 2, new QTableWidgetItem(QueryExtractData.value("platform").toString()));
            rownum++;
        }

    }
    else{
        qDebug() << QueryExtractData.lastError();
    }
    QSqlDatabase::database().commit();
    DB_Connection.close();
}


void MainWindow::on_pushButton_3_released()
{
    //delete button
    QString username = ui->usernameInput->text();
    QString password = ui->passwordInput->text();
    QString platform = ui->platformInput->text();

    DB_Connection.open();
    QSqlDatabase::database().transaction();
    QSqlQuery QueryDeleteData(DB_Connection);
    QueryDeleteData.prepare("DELETE FROM vault1 WHERE username = :username AND password = :password AND platform = :platform");
    QueryDeleteData.bindValue(":username", username);
    QueryDeleteData.bindValue(":password", password);
    QueryDeleteData.bindValue(":platform", platform);
    if (QueryDeleteData.exec())
        qDebug() << "Query executed";
    else
        qDebug() << QueryDeleteData.lastError();
    QSqlDatabase::database().commit();
    DB_Connection.close();
}


void MainWindow::on_pushButton_4_released()
{
    //update button
    QString username = ui->usernameInput->text();
    QString password = ui->passwordInput->text();
    QString platform = ui->platformInput->text();

    DB_Connection.open();
    QSqlDatabase::database().transaction();
    QSqlQuery QueryUpdateData(DB_Connection);
    QueryUpdateData.prepare("UPDATE vault1 SET password =:password WHERE username = :username AND platform = :platform");
    QueryUpdateData.bindValue(":username", username);
    QueryUpdateData.bindValue(":password", password);
    QueryUpdateData.bindValue(":platform", platform);
    if (QueryUpdateData.exec())
        qDebug() << "Query executed";
    else
        qDebug() << QueryUpdateData.lastError();
    QSqlDatabase::database().commit();
    DB_Connection.close();
}

