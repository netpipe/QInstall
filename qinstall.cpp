#include "qinstall.h"
#include "ui_qinstall.h"
#include <QFile>
#include <QFileDialog>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QMessageBox>
#include <QCryptographicHash>
#include "quazip/quazip.h"
#include "quazip/quazipfile.h"
#include "quazip/JlCompress.h"

#ifdef WINDOWS
#include <windows.h>
#endif

#include <string.h>

#include <QDebug>

//https://www.strchr.com/creating_self-extracting_executables

#include "linuxfiles.h"

int ReadFromExeFile();

QByteArray fileChecksum(const QString &fileName);

void zip(QString filename , QString zip_filename)
{
   QFile infile(filename);
   QFile outfile(zip_filename);
   infile.open(QIODevice::ReadOnly);
   outfile.open(QIODevice::WriteOnly);
   QByteArray uncompressed_data = infile.readAll();
   QByteArray compressed_data = qCompress(uncompressed_data, 9);
   outfile.write(compressed_data);
   infile.close();
   outfile.close();
}

void unZip(QString zip_filename , QString filename)
{
   QFile infile(zip_filename);
   QFile outfile(filename);
   infile.open(QIODevice::ReadOnly);
   outfile.open(QIODevice::WriteOnly);
   QByteArray uncompressed_data = infile.readAll();
   QByteArray compressed_data = qUncompress(uncompressed_data);
   outfile.write(compressed_data);
   infile.close();
   outfile.close();
}


QInstall::QInstall(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::QInstall)
{
    ui->setupUi(this);
}

QInstall::~QInstall()
{
    delete ui;
}

void QInstall::on_installbtn_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"),
                                 "",
                                 tr("Zip File (*.zip)"));
//    zip("C:/test.txt", "C:/test.zip");
//    unZip("C:/test.zip", "C:/test2.txt");
}

void QInstall::on_btnRemove_clicked()
{
    QList<QTableWidgetItem*> itmlist = ui->tblFileList->selectedItems();
    if (itmlist.size() <= 0)
        return;
    int nCurRow = itmlist[0]->row();

    ui->tblFileList->removeRow(nCurRow);
}

void QInstall::on_btnAdd_clicked()
{
    QStringList files = QFileDialog::getOpenFileNames(
                              this,
                              "Select one or more files to open",
                              "",
                              "All Files (*.*)");

    int nCurIndex = ui->tblFileList->rowCount();

    for(int i = 0 ; i < files.size() ; i++) {
        QTableWidgetItem *item = new QTableWidgetItem(files.at(i));
        ui->tblFileList->insertRow(i+nCurIndex);
        ui->tblFileList->setItem(i+nCurIndex,0,item);
        ui->tblFileList->setColumnWidth(0, ui->tblFileList->width());
    }
}

void QInstall::on_btnCreate_clicked()
{
    QString saveFile = QFileDialog::getSaveFileName(this, "Select file to save","", "Zip File(*.zip)");
    QStringList list;
    for(int i = 0 ; i < ui->tblFileList->rowCount() ; i++) {
        QTableWidgetItem *item = ui->tblFileList->item(i,0);
        list.append(item->text());
    }
    if(JlCompress::compressFiles(saveFile, list)){
        QMessageBox::information(this, "Notice", "Compression success.");
        QByteArray checksum = fileChecksum(saveFile);
        if(ui->chkMd5->isChecked()) {
            QString checkFile = saveFile.section(".",0,0) + ".md5";
            QFile file(checkFile);
            file.open(QIODevice::WriteOnly);
            file.write(checksum);
            file.close();
        }
    }
#ifdef WINDOWS
    if(ui->chkExe->isChecked()) {
        QString exeFile = saveFile.section(".", 0,0) + ".exe";
        BYTE buff[4096];
        wchar_t wzcmd[4096] = {0};
        GetModuleFileNameA(NULL, (CHAR*)buff, sizeof(buff));
        QString cmd = QString("/c copy /b '%1'+'%2' '%3'").arg(QString::fromLocal8Bit((char*)buff)).arg(saveFile.replace("/","\\")).arg(exeFile.replace("/","\\"));
        cmd.replace("'", QString((char)34));
        cmd.toWCharArray(wzcmd);
        ShellExecuteA(NULL, "open", "cmd", cmd.toUtf8(), NULL, SW_HIDE);
    }
#endif
}

QByteArray fileChecksum(const QString &fileName)
{
    QFile f(fileName);
    if (f.open(QFile::ReadOnly)) {
        QCryptographicHash hash(QCryptographicHash::Md5);
        if (hash.addData(&f)) {
            return hash.result();
        }
    }
    return QByteArray();
}

void QInstall::on_btnOpen_clicked()
{
    QString file = QFileDialog::getOpenFileName(this, "Select file to open","", "Zip File(*.zip)");
    ui->editZipFilePath->setText(file);
}

void QInstall::on_btnDecompress_clicked()
{
    QString zipFile = ui->editZipFilePath->text();
    if(zipFile == "")
        return;

    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                      "",
                                                      QFileDialog::ShowDirsOnly
                                                      | QFileDialog::DontResolveSymlinks);
    if(dir == "")
        return;

    QStringList list = JlCompress::getFileList(zipFile);
    JlCompress::extractFiles(zipFile, list, dir);
    QMessageBox::information(this, "Notice", "Decompress success.");
}

void QInstall::on_btnBrowse_clicked()
{
    QString file = QFileDialog::getOpenFileName(this, "Select file to open","", "Exe File(*.exe)");
    ui->editFile->setText(file);

    QStringList list = JlCompress::getFileList(file);

    for(int i = 0 ; i < list.size() ; i++) {

        QTableWidgetItem *item = new QTableWidgetItem(list.at(i));
        ui->filelist->insertRow(i);
        ui->filelist->setItem(i,0,item);
        ui->filelist->setColumnWidth(0, ui->filelist->width());
    }
}

void QInstall::on_btnInstall_clicked()
{
//    QString zipFile = ui->editFile->text();
//    if(zipFile == "")
//        return;

    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                      "",
                                                      QFileDialog::ShowDirsOnly
                                                      | QFileDialog::DontResolveSymlinks);
    if(dir == "")
        return;
#ifdef WINDOWS
    ReadFromExeFile();
#endif
    QString zipFile = QCoreApplication::applicationDirPath() + "./1.zip";

    QStringList list = JlCompress::getFileList(zipFile);
    JlCompress::extractFiles(zipFile, list, dir);
    QMessageBox::information(this, "Notice", "Install success.");
}

#include "winfiles.cpp"
