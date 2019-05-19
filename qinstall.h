#ifndef QINSTALL_H
#define QINSTALL_H

#include <QMainWindow>

namespace Ui {
class QInstall;
}

class QInstall : public QMainWindow
{
    Q_OBJECT

public:
    explicit QInstall(QWidget *parent = 0);
    ~QInstall();

private slots:
    void on_installbtn_clicked();

private:
    Ui::QInstall *ui;
};

#endif // QINSTALL_H
