/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 4.8.7
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QMainWindow>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QStatusBar>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralWidget;
    QVBoxLayout *verticalLayout;
    QWidget *widget_1;
    QHBoxLayout *horizontalLayout_2;
    QLabel *label_2;
    QLineEdit *lePath;
    QPushButton *bBrowseIn;
    QSpacerItem *verticalSpacer_3;
    QWidget *widget_3;
    QHBoxLayout *horizontalLayout_3;
    QLabel *label_4;
    QLineEdit *leOutDir;
    QPushButton *bBrowseOut;
    QSpacerItem *verticalSpacer_2;
    QWidget *widget_2;
    QHBoxLayout *horizontalLayout;
    QLabel *label_3;
    QLineEdit *leOutFilename;
    QSpacerItem *horizontalSpacer;
    QLabel *label;
    QLineEdit *leLinesCount;
    QPushButton *bRun;
    QSpacerItem *verticalSpacer;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(575, 168);
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        verticalLayout = new QVBoxLayout(centralWidget);
        verticalLayout->setSpacing(0);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setSizeConstraint(QLayout::SetDefaultConstraint);
        verticalLayout->setContentsMargins(9, 9, 9, 9);
        widget_1 = new QWidget(centralWidget);
        widget_1->setObjectName(QString::fromUtf8("widget_1"));
        horizontalLayout_2 = new QHBoxLayout(widget_1);
        horizontalLayout_2->setSpacing(6);
        horizontalLayout_2->setContentsMargins(11, 11, 11, 11);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        horizontalLayout_2->setContentsMargins(0, 0, 0, 0);
        label_2 = new QLabel(widget_1);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setMinimumSize(QSize(60, 0));
        label_2->setScaledContents(false);

        horizontalLayout_2->addWidget(label_2);

        lePath = new QLineEdit(widget_1);
        lePath->setObjectName(QString::fromUtf8("lePath"));
        lePath->setMinimumSize(QSize(0, 30));
        QFont font;
        font.setPointSize(10);
        lePath->setFont(font);

        horizontalLayout_2->addWidget(lePath);

        bBrowseIn = new QPushButton(widget_1);
        bBrowseIn->setObjectName(QString::fromUtf8("bBrowseIn"));
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(bBrowseIn->sizePolicy().hasHeightForWidth());
        bBrowseIn->setSizePolicy(sizePolicy);
        bBrowseIn->setMinimumSize(QSize(80, 35));

        horizontalLayout_2->addWidget(bBrowseIn);


        verticalLayout->addWidget(widget_1);

        verticalSpacer_3 = new QSpacerItem(20, 10, QSizePolicy::Minimum, QSizePolicy::Fixed);

        verticalLayout->addItem(verticalSpacer_3);

        widget_3 = new QWidget(centralWidget);
        widget_3->setObjectName(QString::fromUtf8("widget_3"));
        horizontalLayout_3 = new QHBoxLayout(widget_3);
        horizontalLayout_3->setSpacing(6);
        horizontalLayout_3->setContentsMargins(11, 11, 11, 11);
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        horizontalLayout_3->setContentsMargins(0, 0, 0, 0);
        label_4 = new QLabel(widget_3);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setMinimumSize(QSize(60, 0));
        label_4->setScaledContents(false);

        horizontalLayout_3->addWidget(label_4);

        leOutDir = new QLineEdit(widget_3);
        leOutDir->setObjectName(QString::fromUtf8("leOutDir"));
        leOutDir->setMinimumSize(QSize(100, 30));
        leOutDir->setFont(font);

        horizontalLayout_3->addWidget(leOutDir);

        bBrowseOut = new QPushButton(widget_3);
        bBrowseOut->setObjectName(QString::fromUtf8("bBrowseOut"));
        sizePolicy.setHeightForWidth(bBrowseOut->sizePolicy().hasHeightForWidth());
        bBrowseOut->setSizePolicy(sizePolicy);
        bBrowseOut->setMinimumSize(QSize(80, 35));
        QFont font1;
        font1.setPointSize(8);
        font1.setBold(false);
        font1.setItalic(false);
        font1.setUnderline(false);
        font1.setWeight(50);
        bBrowseOut->setFont(font1);

        horizontalLayout_3->addWidget(bBrowseOut);


        verticalLayout->addWidget(widget_3);

        verticalSpacer_2 = new QSpacerItem(20, 10, QSizePolicy::Minimum, QSizePolicy::Fixed);

        verticalLayout->addItem(verticalSpacer_2);

        widget_2 = new QWidget(centralWidget);
        widget_2->setObjectName(QString::fromUtf8("widget_2"));
        horizontalLayout = new QHBoxLayout(widget_2);
        horizontalLayout->setSpacing(6);
        horizontalLayout->setContentsMargins(11, 11, 11, 11);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
        label_3 = new QLabel(widget_2);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setMinimumSize(QSize(60, 0));
        label_3->setScaledContents(false);

        horizontalLayout->addWidget(label_3);

        leOutFilename = new QLineEdit(widget_2);
        leOutFilename->setObjectName(QString::fromUtf8("leOutFilename"));
        leOutFilename->setMinimumSize(QSize(200, 30));
        leOutFilename->setFont(font);

        horizontalLayout->addWidget(leOutFilename);

        horizontalSpacer = new QSpacerItem(10, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        label = new QLabel(widget_2);
        label->setObjectName(QString::fromUtf8("label"));
        label->setScaledContents(false);

        horizontalLayout->addWidget(label);

        leLinesCount = new QLineEdit(widget_2);
        leLinesCount->setObjectName(QString::fromUtf8("leLinesCount"));
        sizePolicy.setHeightForWidth(leLinesCount->sizePolicy().hasHeightForWidth());
        leLinesCount->setSizePolicy(sizePolicy);
        leLinesCount->setMinimumSize(QSize(0, 30));
        leLinesCount->setFont(font);

        horizontalLayout->addWidget(leLinesCount);

        bRun = new QPushButton(widget_2);
        bRun->setObjectName(QString::fromUtf8("bRun"));
        sizePolicy.setHeightForWidth(bRun->sizePolicy().hasHeightForWidth());
        bRun->setSizePolicy(sizePolicy);
        bRun->setMinimumSize(QSize(80, 35));
        QFont font2;
        font2.setPointSize(9);
        font2.setBold(true);
        font2.setItalic(false);
        font2.setUnderline(false);
        font2.setWeight(75);
        bRun->setFont(font2);

        horizontalLayout->addWidget(bRun);


        verticalLayout->addWidget(widget_2);

        verticalSpacer = new QSpacerItem(20, 10, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer);

        MainWindow->setCentralWidget(centralWidget);
        statusBar = new QStatusBar(MainWindow);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        statusBar->setMinimumSize(QSize(0, 25));
        QFont font3;
        font3.setPointSize(9);
        statusBar->setFont(font3);
        MainWindow->setStatusBar(statusBar);
#ifndef QT_NO_SHORTCUT
        label_2->setBuddy(lePath);
        label_4->setBuddy(leOutDir);
        label_3->setBuddy(leOutFilename);
        label->setBuddy(leLinesCount);
#endif // QT_NO_SHORTCUT
        QWidget::setTabOrder(lePath, leOutDir);
        QWidget::setTabOrder(leOutDir, leOutFilename);
        QWidget::setTabOrder(leOutFilename, leLinesCount);
        QWidget::setTabOrder(leLinesCount, bBrowseIn);
        QWidget::setTabOrder(bBrowseIn, bBrowseOut);
        QWidget::setTabOrder(bBrowseOut, bRun);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "File Splitter", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("MainWindow", "Input File:", 0, QApplication::UnicodeUTF8));
        lePath->setText(QString());
        bBrowseIn->setText(QApplication::translate("MainWindow", "Browse...", 0, QApplication::UnicodeUTF8));
        label_4->setText(QApplication::translate("MainWindow", "Output Dir:", 0, QApplication::UnicodeUTF8));
        leOutDir->setText(QString());
        bBrowseOut->setText(QApplication::translate("MainWindow", "Browse...", 0, QApplication::UnicodeUTF8));
        label_3->setText(QApplication::translate("MainWindow", "Output File:", 0, QApplication::UnicodeUTF8));
        leOutFilename->setText(QString());
        label->setText(QApplication::translate("MainWindow", "Lines:", 0, QApplication::UnicodeUTF8));
        leLinesCount->setText(QApplication::translate("MainWindow", "1000", 0, QApplication::UnicodeUTF8));
        bRun->setText(QApplication::translate("MainWindow", "Run", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
