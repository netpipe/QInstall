#include <QApplication>
#include <QCoreApplication>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QDir>
#include <QDirIterator>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QDropEvent>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QCryptographicHash>
#include <QDataStream>
#include <QProcess>
#include <QMessageBox>
#include <QTextStream>

#define MAGIC_FOOTER "QPKG"
#define MAX_TRAILER_SIZE 20 * 1024 * 1024 // 20MB max payload size

struct FileEntry {
    QString relativePath;
    QByteArray data;
    QByteArray md5;
};
bool hasEmbeddedData(QByteArray &outPayloadData) {
    QFile self(QCoreApplication::applicationFilePath());
    if (!self.open(QIODevice::ReadOnly)) return false;

    // Footer is: [MAGIC_FOOTER (4 bytes)] + [payload offset (quint64 = 8 bytes)]
    if (self.size() < 12) return false;

    self.seek(self.size() - 12); // Go to where footer starts
    QByteArray magic = self.read(4);
    if (magic != MAGIC_FOOTER) return false;

    QDataStream stream(&self);
    stream.setVersion(QDataStream::Qt_5_12);
    quint64 payloadOffset;
    stream >> payloadOffset;

    if (payloadOffset >= self.size() || payloadOffset < 0) return false;

    self.seek(payloadOffset);
    outPayloadData = self.read(self.size() - payloadOffset - 12); // just payload
    return true;
}


void runExtractor() {
    QByteArray payload;
    if (!hasEmbeddedData(payload)) {
        qWarning("No embedded data found.");
        return;
    }

    QDataStream stream(payload);
    stream.setVersion(QDataStream::Qt_5_12);

    quint32 fileCount;
    stream >> fileCount;

    for (quint32 i = 0; i < fileCount; ++i) {
        QString path;
        quint32 size;
        QByteArray md5;
        stream >> path >> size >> md5;

        QByteArray fileData(size, '\0');
        stream.readRawData(fileData.data(), size);

        QByteArray calcMd5 = QCryptographicHash::hash(fileData, QCryptographicHash::Md5);
        if (calcMd5 != md5) {
            qWarning("MD5 mismatch for: %s", qPrintable(path));
            continue;
        }

        QFile out(path);
        QDir().mkpath(QFileInfo(out).absolutePath());
        if (out.open(QIODevice::WriteOnly)) {
            out.write(fileData);
            out.close();
            qInfo("Extracted: %s", qPrintable(path));
        }
    }

    QMessageBox::information(nullptr, "Done", "Extraction complete.");
}
class InstallerBuilder : public QWidget {
    Q_OBJECT
public:
    InstallerBuilder(QWidget *parent = nullptr) : QWidget(parent) {
        setWindowTitle("Self-Installer Builder");
        resize(500, 400);
        setAcceptDrops(true);

        list = new QListWidget(this);
        QPushButton *addBtn = new QPushButton("Add Files/Folders");
        QPushButton *clearBtn = new QPushButton("Clear");
        QPushButton *buildBtn = new QPushButton("Build Installer");

        QHBoxLayout *btnLayout = new QHBoxLayout;
        btnLayout->addWidget(addBtn);
        btnLayout->addWidget(clearBtn);

        QVBoxLayout *mainLayout = new QVBoxLayout(this);
        mainLayout->addWidget(list);
        mainLayout->addLayout(btnLayout);
        mainLayout->addWidget(buildBtn);

        connect(addBtn, &QPushButton::clicked, this, &InstallerBuilder::addFiles);
        connect(clearBtn, &QPushButton::clicked, this, &InstallerBuilder::clearList);
        connect(buildBtn, &QPushButton::clicked, this, &InstallerBuilder::buildInstaller);
    }

protected:
    void dragEnterEvent(QDragEnterEvent *event) override {
        if (event->mimeData()->hasUrls()) event->acceptProposedAction();
    }

    void dropEvent(QDropEvent *event) override {
        for (const QUrl &url : event->mimeData()->urls()) {
            QFileInfo fi(url.toLocalFile());
            if (fi.isDir()) addDirectory(fi.absoluteFilePath(), fi.absolutePath());
            else addFile(fi.absoluteFilePath(), fi.dir().absolutePath());
        }
    }

private:
    QListWidget *list;
    QList<FileEntry> files;

    void addFiles() {
        QStringList paths = QFileDialog::getOpenFileNames(this);
        for (const QString &path : paths) {
            QFileInfo fi(path);
            addFile(path, fi.dir().absolutePath());
        }
    }

    void clearList() {
        list->clear();
        files.clear();
    }

    void addFile(const QString &path, const QString &baseDir) {
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly)) return;
        QByteArray data = file.readAll();
        QByteArray md5 = QCryptographicHash::hash(data, QCryptographicHash::Md5);
        QString rel = QDir(baseDir).relativeFilePath(path);
        files.append({rel, data, md5});
        list->addItem(rel);
    }

    void addDirectory(const QString &path, const QString &baseDir) {
        QDirIterator it(path, QDir::Files, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            addFile(it.next(), baseDir);
        }
    }

    void buildInstaller() {
        if (files.isEmpty()) return;

        QString savePath = QFileDialog::getSaveFileName(this, "Save Installer As");
        if (savePath.isEmpty()) return;

        QFile out(savePath);
        if (!out.open(QIODevice::WriteOnly)) {
            QMessageBox::critical(this, "Error", "Failed to write output file.");
            return;
        }

        // Copy this binary
        QFile self(QCoreApplication::applicationFilePath());
        if (!self.open(QIODevice::ReadOnly)) return;
        out.write(self.readAll());

        // Append payload
        qint64 payloadStart = out.pos();

        QDataStream stream(&out);
        stream.setVersion(QDataStream::Qt_5_12);
        stream << quint32(files.size());
        for (const FileEntry &e : files) {
            stream << e.relativePath << quint32(e.data.size()) << e.md5;
            stream.writeRawData(e.data.constData(), e.data.size());
        }

        // Append footer: magic + offset
        out.write(MAGIC_FOOTER, 4);
        QDataStream footer(&out);
        footer.setVersion(QDataStream::Qt_5_12);
        footer << quint64(payloadStart);

        out.close();

        QMessageBox::information(this, "Success", "Installer built.");
    }
};
#include "main.moc"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // Detect if there's a payload
    QByteArray payload;
    bool hasPayload = hasEmbeddedData(payload);

    if (hasPayload && (argc == 1)) {
        runExtractor();
        return 0;
    }

    // Otherwise launch builder
    InstallerBuilder w;
    w.show();
    return app.exec();
}
