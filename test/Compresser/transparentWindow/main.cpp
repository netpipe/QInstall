#include <QApplication>
#include <QWidget>
#include <QPainter>
#include <QBitmap>
#include <QPixmap>
#include <QPushButton>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QStyle>

class ShapedWindow : public QWidget {
    QPixmap shapeImage;
    QPoint dragPosition;

public:
    ShapedWindow(const QString& imagePath) {
        shapeImage.load(imagePath);
        resize(shapeImage.size());

        setWindowFlags(Qt::FramelessWindowHint | Qt::Window);
        setAttribute(Qt::WA_TranslucentBackground);
        setMask(shapeImage.mask());

        QPushButton* closeBtn = new QPushButton("X", this);
        QPushButton* minBtn = new QPushButton("_", this);
        QPushButton* maxBtn = new QPushButton("□", this);

        // Basic style
        QString style = "QPushButton { background-color: rgba(0,0,0,100); color: white; border: none; padding: 3px; }"
                        "QPushButton:hover { background-color: rgba(255,0,0,150); }";
        closeBtn->setStyleSheet(style);
        minBtn->setStyleSheet(style);
        maxBtn->setStyleSheet(style);

        // Position buttons
        closeBtn->setGeometry(width() - 35, 5, 30, 20);
        maxBtn->setGeometry(width() - 70, 5, 30, 20);
        minBtn->setGeometry(width() - 105, 5, 30, 20);

        connect(closeBtn, &QPushButton::clicked, this, &QWidget::close);
        connect(minBtn, &QPushButton::clicked, this, &QWidget::showMinimized);
        connect(maxBtn, &QPushButton::clicked, [this]() {
            if (isMaximized()) showNormal(); else showMaximized();
        });
    }

protected:
    void paintEvent(QPaintEvent*) override {
        QPainter painter(this);
        painter.drawPixmap(0, 0, shapeImage);
    }

    void mousePressEvent(QMouseEvent* event) override {
        if (event->button() == Qt::LeftButton) {
            dragPosition = event->globalPos() - frameGeometry().topLeft();
            event->accept();
        }
    }

    void mouseMoveEvent(QMouseEvent* event) override {
        if (event->buttons() & Qt::LeftButton) {
            move(event->globalPos() - dragPosition);
            event->accept();
        }
    }
};

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    ShapedWindow w(":/background.png"); // Ensure this image is in the same directory
    w.show();
    return a.exec();
}
