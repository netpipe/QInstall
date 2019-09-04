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
#include <windows.h>
#include <string.h>

#include <QDebug>

//https://www.strchr.com/creating_self-extracting_executables

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

    ReadFromExeFile();
    QString zipFile = QCoreApplication::applicationDirPath() + "./1.zip";

    QStringList list = JlCompress::getFileList(zipFile);
    JlCompress::extractFiles(zipFile, list, dir);
    QMessageBox::information(this, "Notice", "Install success.");
}

int ReadFromExeFile() {
    BYTE buff[4096];
    DWORD read;
    BYTE* data;

    // Open exe file
    GetModuleFileNameA(NULL, (CHAR*)buff, sizeof(buff));
    //cout << buff << endl;

    HANDLE hFile = CreateFileA((CHAR*)buff, GENERIC_READ, FILE_SHARE_READ,
        NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (INVALID_HANDLE_VALUE == hFile)
        return ERROR;

    ReadFile(hFile, buff, sizeof(buff), &read, NULL);
    IMAGE_DOS_HEADER* dosheader = (IMAGE_DOS_HEADER*)buff;

    // Locate PE header
    IMAGE_NT_HEADERS32* header = (IMAGE_NT_HEADERS32*)(buff + dosheader->e_lfanew);
    if (dosheader->e_magic != IMAGE_DOS_SIGNATURE || header->Signature != IMAGE_NT_SIGNATURE) {
        CloseHandle(hFile);
        return ERROR;
    }

    // For each section
    IMAGE_SECTION_HEADER* sectiontable = (IMAGE_SECTION_HEADER*)((BYTE*)header + sizeof(IMAGE_NT_HEADERS32));
    DWORD maxpointer = 0, exesize = 0;
    for (int i = 0; i < header->FileHeader.NumberOfSections; i++) {
        if (sectiontable->PointerToRawData > maxpointer) {
            maxpointer = sectiontable->PointerToRawData;
            exesize = sectiontable->PointerToRawData + sectiontable->SizeOfRawData;
        }
        sectiontable++;
    }

    // Seek to the overlay
    DWORD filesize = GetFileSize(hFile, NULL);
    SetFilePointer(hFile, exesize, NULL, FILE_BEGIN);
    data = (BYTE*)malloc(filesize - exesize + 1);
    ReadFile(hFile, data, filesize - exesize, &read, NULL);
    CloseHandle(hFile);

    //cout << filesize << " " << exesize << endl;

    // Process the data
    if (filesize != exesize) {
        *(data + (filesize - exesize)) = '\0';
        //cout << data << endl;

        HANDLE hFile1 = CreateFileA((CHAR*)"1.zip", GENERIC_WRITE, 0,
            NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
        if (INVALID_HANDLE_VALUE == hFile1)
            return ERROR;
        DWORD writtensize;
        WriteFile(hFile1, data, filesize - exesize + 1, &writtensize, NULL);
        CloseHandle(hFile1);
    }

    free(data);
    return 0;
}
