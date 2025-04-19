#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "stdio.h"

#include <QLabel>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>
#include <QDebug>

//DES encryption
#include <iostream>
#include <cstring>
#include <algorithm>
#include "DES.h"
#include "DES.cpp"
const int source_size = 32;
const char* HexaDigits = "0123456789abcdef";

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

    QString uiKey = ui->keyInput_2->text();
    char key[8] = { 0 };

    //Values for encryption
    QByteArray ba = uiKey.toLatin1();
    std::strncpy(key, ba.constData(), sizeof(key));

    char source[source_size] = { 0 };
    ba = password.toLatin1();
    std::strncpy(source, ba.constData(), sizeof(password.toLatin1()));
    size_t length = strlen(source);
    size_t chunkSize = 8;
    size_t totalChunks = (length + chunkSize - 1) / chunkSize;

    // Round Keys for DES Encryption and Decryption
    char roundKeys[16][8];
    DES_CreateKeys(key, roundKeys);

    char encrypted[64] = { 0 };

    for (size_t i = 0; i < totalChunks; i++) {
        char chunk[8] = { 0 }; // must be zero-initialized
        char cipherText[8] = { 0 };
        char original[9] = { 0 };
        size_t start = i * chunkSize;
        size_t end = std::min(start + chunkSize, length);

        std::copy(source + start, source + end, chunk);

        DES_Encrypt(chunk, roundKeys, cipherText);

        memcpy(encrypted + 8 * i, cipherText, 8); //8 * i sets an offset for the encrypted char array
    }

    password = QString::fromLatin1(encrypted);

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

    //Emptying memory where data has been stored
    std::fill(key, key + 8, 0);
    std::fill(source, source + source_size, 0);
}


void MainWindow::on_pushButton_2_released()
{
    //Decryption initialization
    QString uiKey = ui->keyInput->text();
    char key[8] = { 0 };

    //Values for decryption
    QByteArray ba = uiKey.toLatin1();
    std::strncpy(key, ba.constData(), sizeof(key));

    DB_Connection.open();
    QSqlDatabase::database().transaction();
    QSqlQuery QueryExtractData(DB_Connection);
    QueryExtractData.prepare("SELECT * FROM vault1");

    ui->tableWidget->setRowCount(0);
    ui->tableWidget->setRowCount(32);
    int rownum = 0;
    if (QueryExtractData.exec()){
        //ui->tableWidget->setRowCount(QueryExtractData.size());
        while (QueryExtractData.next()){
            QString password = QueryExtractData.value("password").toString();
            ba = password.toLatin1();
            char encrypted[64] = {0};
            std::strncpy(encrypted, ba.constData(), sizeof(password.toLatin1()));

            char decrypted[source_size] = { 0 };
            size_t length = strlen(encrypted);
            size_t chunkSize = 8;
            size_t totalChunks = (length + chunkSize - 1) / chunkSize;

            // Round Keys for DES Encryption and Decryption
            char roundKeys[16][8];
            DES_CreateKeys(key, roundKeys);

            //Decryption
            for (size_t i = 0; i < totalChunks; i++) {
                char original[9] = { 0 }; // must be zero-initialized
                char chunk[8] = { 0 };
                size_t start = i * chunkSize;
                size_t end = std::min(start + chunkSize, length);

                std::copy(encrypted + start, encrypted + end, chunk);

                DES_Decrypt(chunk, roundKeys, original);
                memcpy(decrypted + 8*i, original, 8);
            }

            password = QString::fromLatin1(decrypted);

            ui->tableWidget->setItem(rownum, 0, new QTableWidgetItem(QueryExtractData.value("username").toString()));
            ui->tableWidget->setItem(rownum, 1, new QTableWidgetItem(password));
            ui->tableWidget->setItem(rownum, 2, new QTableWidgetItem(QueryExtractData.value("platform").toString()));

            rownum++;
        }

    }
    else{
        qDebug() << QueryExtractData.lastError();
    }
    QSqlDatabase::database().commit();
    DB_Connection.close();

    //Emptying memory where data has been stored
    std::fill(key, key + 8, 0);
}


void MainWindow::on_pushButton_3_released()
{
    //delete button
    QString username = ui->usernameInput->text();
    QString platform = ui->platformInput->text();

    DB_Connection.open();
    QSqlDatabase::database().transaction();
    QSqlQuery QueryDeleteData(DB_Connection);
    QueryDeleteData.prepare("DELETE FROM vault1 WHERE username = :username AND platform = :platform");
    QueryDeleteData.bindValue(":username", username);
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

    QString uiKey = ui->keyInput_2->text();
    char key[8] = { 0 };

    //Encryption
    //Values for encryption
    QByteArray ba = uiKey.toLatin1();
    std::strncpy(key, ba.constData(), sizeof(key));

    char source[source_size] = { 0 };
    ba = password.toLatin1();
    std::strncpy(source, ba.constData(), sizeof(password.toLatin1()));
    size_t length = strlen(source);
    size_t chunkSize = 8;
    size_t totalChunks = (length + chunkSize - 1) / chunkSize;

    // Round Keys for DES Encryption and Decryption
    char roundKeys[16][8];
    DES_CreateKeys(key, roundKeys);

    char encrypted[64] = { 0 };

    for (size_t i = 0; i < totalChunks; i++) {
        char chunk[8] = { 0 }; // must be zero-initialized
        char cipherText[8] = { 0 };
        char original[9] = { 0 };
        size_t start = i * chunkSize;
        size_t end = std::min(start + chunkSize, length);

        std::copy(source + start, source + end, chunk);

        DES_Encrypt(chunk, roundKeys, cipherText);

        memcpy(encrypted + 8 * i, cipherText, 8); //8 * i sets an offset for the encrypted char array
    }

    password = QString::fromLatin1(encrypted);

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

