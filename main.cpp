﻿// BEGIN single-file installer/builder with ZIP & GUI extractor
#include <QApplication>
#include <QCoreApplication>
#include <QtWidgets>
#include <QProcess>
#include <QCryptographicHash>
#include <QDataStream>
#include <QFileInfo>
#include <QDirIterator>


//todo
// GUI progressbar
// encryption on payload files
// Remote fallback (download missing pieces)
//multi archive md5 support possibly by appending a list of them instead of payload.zip


bool hasPayload=0;

#define MAGIC_FOOTER "QPKG"
#define FOOTER_SIZE (4 + sizeof(quint64))

struct ArchiveInfo {
    QByteArray data;
    QByteArray md5;
    QString name;
};

QString test;

static bool hasEmbeddedData(QByteArray &outZip, QByteArray &outMd5, QString &outName) {

    QFile f(QCoreApplication::applicationFilePath());
    if (!f.open(QIODevice::ReadOnly)) return false;
    if (f.size() < FOOTER_SIZE) return false;

    f.seek(f.size() - FOOTER_SIZE);
    QByteArray magic = f.read(4);
    if (magic != MAGIC_FOOTER) return false;
    QDataStream ds(&f);
    ds.setVersion(QDataStream::Qt_5_12);
    quint64 offset;
    ds >> offset;
    if (offset >= (quint64)f.size()) return false;

    f.seek(offset);
    QDataStream in(&f);
    in.setVersion(QDataStream::Qt_5_12);
    in >> outName;
    quint64 size;
    in >> size;
    in >> outMd5;
    in >> test;
    outZip = f.read(size);

    qDebug() << "Reading pathname" << test;
    qDebug() << "Reading footer at offset:" << (f.size() - FOOTER_SIZE);
    qDebug() << "Magic:" << magic;
    qDebug() << "Payload offset:" << offset;
    qDebug() << "Read ZIP size:" << size;
    qDebug() << "Read MD5:" << outMd5.toHex();
    qDebug() << "Computed MD5:" << QCryptographicHash::hash(outZip, QCryptographicHash::Md5).toHex();

    return true;
}

// Extractor Window
class InstallerWindow : public QWidget {
    Q_OBJECT
public:
    InstallerWindow(const ArchiveInfo& arch, QWidget* parent=nullptr)
      : QWidget(parent), archive(arch) {
        setWindowTitle("Installer");
        resize(500,400);

        bgLabel = new QLabel(this);
        bgLabel->resize(500,400);

        if (!QFile::exists(QApplication::applicationDirPath() + "/background.png")){
            bgLabel->setPixmap(QPixmap(":/background.png").scaled(size()));
        }else{
            bgLabel->setPixmap(QPixmap(QApplication::applicationDirPath() + "/background.png").scaled(size()));
        }

        bgLabel->setScaledContents(true);

        pathEdit = new QLineEdit(QDir::homePath(), this);
        pathEdit->setText(test);
     //   QPushButton* browseBn = new QPushButton("Browse…", this);
        passEdit = new QLineEdit(this);
        passEdit->setPlaceholderText("ZIP Password (if encrypted)");
        passEdit->setEchoMode(QLineEdit::Password);
        QPushButton* installBn = new QPushButton("Install", this);
        progress = new QProgressBar(this);
        logView = new QTextEdit(this);
        logView->setReadOnly(true);

        QGridLayout* g = new QGridLayout(this);
        g->addWidget(new QLabel("Install to:"),0,0);
        g->addWidget(pathEdit,0,1);
     //   g->addWidget(browseBn,0,2);
        g->addWidget(passEdit,1,0,1,3);
        g->addWidget(installBn,2,0,1,3);
        g->addWidget(progress,3,0,1,3);
        g->addWidget(logView,4,0,1,3);
        setLayout(g);

    //    connect(browseBn, &QPushButton::clicked, this, [&]() {
    //        QString dir = QFileDialog::getExistingDirectory(this,"Select Folder");
    //       if (!dir.isEmpty()) pathEdit->setText(dir);
    //    });
        connect(installBn, &QPushButton::clicked, this, &InstallerWindow::startInstall);
    }

private slots:
    void startInstall() {
        QString outPath = pathEdit->text();
        QDir(outPath).mkpath(".");
        QString zipName = archive.name.isEmpty() ? "payload.zip" : archive.name;
        QString tmpZip = "/tmp/payload.zip";
        qDebug() << "Writing ZIP to:" << tmpZip;

        QFile zip(tmpZip);
        if (!zip.open(QIODevice::WriteOnly)) {
            log("Failed to create temp file");
            return;
        }
        zip.write(archive.data);
        zip.close();

        // md5 verify
        QFile f(tmpZip);
        if (!f.open(QIODevice::ReadOnly)) {
            log("Failed to open temp ZIP");
            return;
        }
        QByteArray d = f.readAll();
        f.close();
        QByteArray md = QCryptographicHash::hash(d, QCryptographicHash::Md5);
        if (md != archive.md5) {
            log("MD5 mismatch on embedded archive!");
            return;
        }
        log("MD5 verified");

        // construct unzip command
        QStringList args;
     //   args << "-o" << tmpZip << "-d" << outPath;
          args << "-o" << tmpZip;
        if (!passEdit->text().isEmpty()) args << "-P" << passEdit->text();

        QProcess proc;
        connect(&proc, &QProcess::readyReadStandardOutput, [&](){
            QString line = proc.readAllStandardOutput();
            log(line.trimmed());
        });
        proc.start("unzip", args);
        proc.waitForFinished(-1);
        log("Extraction done");
        progress->setValue(100);
    }

    void log(const QString& msg) {
        logView->append(msg);
    }

private:
    ArchiveInfo archive;
    QLineEdit* pathEdit;
    QLineEdit* passEdit;
    QProgressBar* progress;
    QTextEdit* logView;
    QLabel* bgLabel;
};


bool copyRecursively(const QString &srcPath, const QString &dstPath) {
    QDir srcDir(srcPath);
    if (!srcDir.exists())
        return false;

    QDir dstDir(dstPath);
    if (!dstDir.exists())
        dstDir.mkpath(".");

    QFileInfoList entries = srcDir.entryInfoList(QDir::NoDotAndDotDot | QDir::AllEntries);
    for (const QFileInfo &entry : entries) {
        QString srcFilePath = entry.filePath();
        QString dstFilePath = dstDir.filePath(entry.fileName());

        if (entry.isDir()) {
            if (!copyRecursively(srcFilePath, dstFilePath))
                return false;
        } else {
            if (!QFile::copy(srcFilePath, dstFilePath))
                return false;
            QFile::setPermissions(dstFilePath, entry.permissions()); // Copy permissions too
        }
    }
    return true;
}

// Builder Window
class BuilderWindow : public QWidget {
    Q_OBJECT
public:
    BuilderWindow(QWidget* p=nullptr) : QWidget(p) {
        setWindowTitle("Installer Builder");
        resize(500,400);
        setAcceptDrops(true);

        bgLabel = new QLabel(this);
        bgLabel->resize(500,400);
        if (!QFile::exists(QApplication::applicationDirPath() + "/background.png")){
            bgLabel->setPixmap(QPixmap(":/background.png").scaled(size()));
        }else{
            bgLabel->setPixmap(QPixmap(QApplication::applicationDirPath() + "/background.png").scaled(size()));
        }
        bgLabel->setScaledContents(true);

        list = new QListWidget(this);
        QPushButton* addBn = new QPushButton("Add");
        QPushButton* clearBn = new QPushButton("Clear");
        QPushButton* buildBn = new QPushButton("Build Installer");
        passEdit = new QLineEdit(this);
        pathEdit = new QLineEdit(this);
        passEdit->setPlaceholderText("ZIP Password (optional)");
        splitSpin = new QSpinBox(this);
        splitSpin->setRange(0,100);
        splitSpin->setSuffix(" MB  (0=single)");


        QHBoxLayout* h1 = new QHBoxLayout;
        h1->addWidget(addBn);
        h1->addWidget(clearBn);
        h1->addWidget(new QLabel("Password:"));
        h1->addWidget(passEdit);

        QHBoxLayout* h2 = new QHBoxLayout;
        h2->addWidget(new QLabel("Split Size:"));
        h2->addWidget(splitSpin);
        h2->addStretch();
        h2->addWidget(buildBn);

         QHBoxLayout* h3 = new QHBoxLayout;
        h3->addWidget(new QLabel("path:"));
        h3->addWidget(pathEdit);
        pathEdit->setText("puts files back where they came from for now");

        QVBoxLayout* main = new QVBoxLayout(this);
        main->addWidget(list);
        main->addLayout(h1);
                main->addLayout(h3);
        main->addLayout(h2);
        setLayout(main);

        connect(addBn, &QPushButton::clicked, this, &BuilderWindow::doAdd);
        connect(clearBn, &QPushButton::clicked, [=](){ list->clear(); files.clear(); });
        connect(buildBn, &QPushButton::clicked, this, &BuilderWindow::doBuild);
    }

protected:
    void dragEnterEvent(QDragEnterEvent *e) override {
        if (e->mimeData()->hasUrls()) e->acceptProposedAction();
    }
    void dropEvent(QDropEvent *e) override {
        for (const QUrl& u : e->mimeData()->urls()) {
            QString p=u.toLocalFile();
            addFileOrDir(p);
        }
    }

private:
    QListWidget* list;
    QVector<QString> files;
    QLineEdit* passEdit;
     QLineEdit* pathEdit;
    QSpinBox* splitSpin;
    QLabel* bgLabel;

    void doAdd() {
        QStringList paths = QFileDialog::getOpenFileNames(this);
        for (auto &p: paths) addFileOrDir(p);
    }
    void addFileOrDir(const QString& p) {
        QFileInfo fi(p);
        if (fi.isDir()) {
            QDirIterator it(p, QDir::Files, QDirIterator::Subdirectories);
            while(it.hasNext()) addFileOrDir(it.next());
        } else {
            files.append(p);
            list->addItem(QDir::current().relativeFilePath(p));
        }
    }

    quint64 findFooterOffset(const QByteArray &binary) {
      //  const QByteArray MAGIC_FOOTER = "ZEND";  // Replace with yours
     //   const int FOOTER_SIZE = 4;
        const int OFFSET_SIZE = sizeof(quint64);
        const int TAIL_SIZE = FOOTER_SIZE + OFFSET_SIZE;

        if (binary.size() < TAIL_SIZE)
            return 0;  // Too small to have footer

        QByteArray footer = binary.right(TAIL_SIZE);
        QByteArray footerMagic = footer.mid(OFFSET_SIZE, FOOTER_SIZE);

        if (footerMagic != MAGIC_FOOTER)
            return 0;  // Not a valid embedded archive

        QDataStream ds(footer.left(OFFSET_SIZE));
        ds.setByteOrder(QDataStream::BigEndian);  // Match how you wrote the offset
        quint64 offset = 0;
        ds >> offset;

        return offset;
    }

    void doBuild() {
        if (files.isEmpty()) return;

        QString save=QFileDialog::getSaveFileName(this,"Save As","installer","All (*)");
        if (save.isEmpty()) return;

        QFileInfo info(save);
            QString fileNameOnly = info.fileName();  // ✅ just "installer" or "something.app"
               QString dirOnly = info.absolutePath();
QString appName=fileNameOnly;
               #ifdef __APPLE__

               QString macosPath = dirOnly + "/" + fileNameOnly + ".app" + "/Contents/MacOS";
               QDir().mkpath(macosPath);  // ✅ Creates Contents and MacOS if they don't exist

            QString plistPath =  dirOnly + "/" + fileNameOnly + ".app" +  "/Contents" + "/Info.plist";

            QFile plistFile(plistPath);
            if (plistFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QTextStream out(&plistFile);
                out << R"(<?xml version="1.0" encoding="UTF-8"?>)"
                    << "\n"
                    << R"(<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">)"
                    << "\n"
                    << R"(<plist version="1.0">)"
                    << "\n<dict>\n"
                    << "    <key>CFBundleExecutable</key>\n"
                    << "    <string>" << appName << "</string>\n"
                    << "    <key>CFBundleIdentifier</key>\n"
                    << "    <string>com.yourcompany." << appName.toLower() << "</string>\n"
                    << "    <key>CFBundleIconFile</key>\n"
                    << "    <string>mac128.png</string>\n"
                    << "    <key>CFBundleVersion</key>\n"
                    << "    <string>1.0</string>\n"
                    << "    <key>CFBundlePackageType</key>\n"
                    << "    <string>APPL</string>\n"
                    << "</dict>\n</plist>";
                //       QFile::copy(":/resources/icon.icns", resourcesPath + "/icon.icns");
                plistFile.close();
            }
#endif
            QString tmpZip;

            if (splitSpin->value()>0) {
               tmpZip = QApplication::applicationDirPath() + "/payload.zip";
               QFile::remove(tmpZip);
            }else{
                 QFile::remove("/tmp/payload.zip");
                  tmpZip = "/tmp/payload.zip";
            }

        QStringList args;
        if (!passEdit->text().isEmpty()) {
            args << "-r" << "-P" << passEdit->text();
        } else args << "-r";
        if (splitSpin->value()>0) {
            args << "-s" << QString::number(splitSpin->value())+"m";
        }
        args << tmpZip;
        for (auto&p:files) args << p;

        QProcess::execute("zip", args);

        QFile z(tmpZip);
        if (!z.open(QIODevice::ReadOnly)) return;
        QByteArray data=z.readAll();
        QByteArray md=QCryptographicHash::hash(data, QCryptographicHash::Md5);
        z.close();


        QFile out(dirOnly + "/" + fileNameOnly);
        if (!out.open(QIODevice::WriteOnly)) return;
        QFile self(QCoreApplication::applicationFilePath());
        if (!self.open(QIODevice::ReadOnly)) return;

if (!hasPayload){
        out.write(self.readAll());
        }else{
        QByteArray binaryData = self.readAll();
        quint64 payloadOffset = findFooterOffset(binaryData); // your logic
        QByteArray cleanBinary = binaryData.left(payloadOffset);
        out.write(cleanBinary);
}

        quint64 off = out.pos();
        QDataStream ds(&out);
        ds.setVersion(QDataStream::Qt_5_12);
        ds << QFileInfo(tmpZip).fileName() << quint64(data.size()) << md << pathEdit->text();
        ds.writeRawData(data.constData(), data.size());
        ds.writeRawData(MAGIC_FOOTER,4);
        //quint64 off = out.pos() - (data.size()+FOOTER_SIZE);
        ds << off;
        out.close();

#ifdef __APPLE__
        QString sourceBinary = dirOnly + "/" + fileNameOnly;
        QString targetBinary = macosPath + "/" + appName;
        QFile::copy(sourceBinary, targetBinary);
        QFile(targetBinary).setPermissions(QFileDevice::ExeOwner | QFileDevice::ReadOwner |     QFileDevice::ExeUser  | QFileDevice::ReadUser);

        QString sourceApp = QCoreApplication::applicationDirPath();
        QString sourceFrameworks = sourceApp + "/../Frameworks";
        QString sourcePlugins = sourceApp + "/../Plugins";
                QString sourceResources = sourceApp + "/../Resources";
        QString destFrameworks = dirOnly + "/" + fileNameOnly + ".app" ;

        if (copyRecursively(sourcePlugins,destFrameworks+"/Contents/Plugins")) {
            qDebug() << "Frameworks copied successfully.";
        } else {
            qWarning() << "Failed to copy Frameworks.";
        }

        if (copyRecursively(sourceResources,destFrameworks+"/Contents/Resources")) {
            qDebug() << "Frameworks copied successfully.";
        } else {
            qWarning() << "Failed to copy Frameworks.";
        }

        if (copyRecursively(sourceFrameworks, destFrameworks+"/Contents/Frameworks")) {
            qDebug() << "Frameworks copied successfully.";
        } else {
            qWarning() << "Failed to copy Frameworks.";
        }

QFile::remove(save);
#endif

        QMessageBox::information(this,"Done","Installer created");
    }
};

// Entry
int main(int argc, char* argv[]){
    QApplication a(argc,argv);
    QByteArray zip, md5; QString name;
    bool btest=0;
    if (hasEmbeddedData(zip,md5,name))
    hasPayload=1;

    if (argc==1 && hasPayload || btest) {
        ArchiveInfo arch{zip,md5,name};
        InstallerWindow w(arch);
        w.show();
        return a.exec();
    } else {
        BuilderWindow b;
        b.show();
        return a.exec();
    }
}
#include "main.moc"
// END single-file code
