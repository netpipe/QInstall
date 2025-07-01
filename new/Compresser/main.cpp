// BEGIN single-file installer/builder with ZIP & GUI extractor
#include <QApplication>
#include <QCoreApplication>
#include <QtWidgets>
#include <QProcess>
#include <QCryptographicHash>
#include <QDataStream>
#include <QFileInfo>
#include <QDirIterator>

#define MAGIC_FOOTER "QPKG"
#define FOOTER_SIZE (4 + sizeof(quint64))

struct ArchiveInfo {
    QByteArray data;
    QByteArray md5;
    QString name;
};

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
    outZip = f.read(size);


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
        bgLabel->setPixmap(QPixmap(":/background.png").scaled(size()));
        bgLabel->setScaledContents(true);

        pathEdit = new QLineEdit(QDir::homePath(), this);
        QPushButton* browseBn = new QPushButton("Browse…", this);
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
        g->addWidget(browseBn,0,2);
        g->addWidget(passEdit,1,0,1,3);
        g->addWidget(installBn,2,0,1,3);
        g->addWidget(progress,3,0,1,3);
        g->addWidget(logView,4,0,1,3);
        setLayout(g);

        connect(browseBn, &QPushButton::clicked, this, [&]() {
            QString dir = QFileDialog::getExistingDirectory(this,"Select Folder");
            if (!dir.isEmpty()) pathEdit->setText(dir);
        });
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
        args << "-o" << tmpZip << "-d" << outPath;
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
        bgLabel->setPixmap(QPixmap(":/background.png").scaled(size()));
        bgLabel->setScaledContents(true);

        list = new QListWidget(this);
        QPushButton* addBn = new QPushButton("Add");
        QPushButton* clearBn = new QPushButton("Clear");
        QPushButton* buildBn = new QPushButton("Build Installer");
        passEdit = new QLineEdit(this);
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

        QVBoxLayout* main = new QVBoxLayout(this);
        main->addWidget(list);
        main->addLayout(h1);
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

    void doBuild() {
        if (files.isEmpty()) return;

        QString save=QFileDialog::getSaveFileName(this,"Save As","installer","All (*)");
        if (save.isEmpty()) return;

        QString tmpZip = "/tmp/payload.zip";
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

        QFile out(save);
        if (!out.open(QIODevice::WriteOnly)) return;
        QFile self(QCoreApplication::applicationFilePath());
        if (!self.open(QIODevice::ReadOnly)) return;
        out.write(self.readAll());
        quint64 off = out.pos();
        QDataStream ds(&out);
        ds.setVersion(QDataStream::Qt_5_12);
        ds << QFileInfo(tmpZip).fileName() << quint64(data.size()) << md;
        ds.writeRawData(data.constData(), data.size());
        ds.writeRawData(MAGIC_FOOTER,4);
        //quint64 off = out.pos() - (data.size()+FOOTER_SIZE);
        ds << off;
        out.close();
        QMessageBox::information(this,"Done","Installer created");
    }
};

// Entry
int main(int argc, char* argv[]){
    QApplication a(argc,argv);
    QByteArray zip, md5; QString name;
    bool btest=0;
    if (argc==1 && hasEmbeddedData(zip,md5,name) || btest) {
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
