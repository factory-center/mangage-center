/********************************************************************************
** Form generated from reading UI file 'UpgradeProgram.ui'
**
** Created by: Qt User Interface Compiler version 5.12.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_UPGRADEPROGRAM_H
#define UI_UPGRADEPROGRAM_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QLabel>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QPushButton>

QT_BEGIN_NAMESPACE

class Ui_UpgradeProgramClass
{
public:
    QLabel *label;
    QProgressBar *progressBar;
    QPushButton *btConfirm;
    QPushButton *btCancel;
    QPushButton *btComplete;

    void setupUi(QDialog *UpgradeProgramClass)
    {
        if (UpgradeProgramClass->objectName().isEmpty())
            UpgradeProgramClass->setObjectName(QString::fromUtf8("UpgradeProgramClass"));
        UpgradeProgramClass->resize(411, 183);
        label = new QLabel(UpgradeProgramClass);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(20, 20, 371, 20));
        QFont font;
        font.setFamily(QString::fromUtf8("\351\273\221\344\275\223"));
        font.setPointSize(12);
        label->setFont(font);
        label->setAlignment(Qt::AlignCenter);
        progressBar = new QProgressBar(UpgradeProgramClass);
        progressBar->setObjectName(QString::fromUtf8("progressBar"));
        progressBar->setGeometry(QRect(40, 60, 381, 25));
        QFont font1;
        font1.setFamily(QString::fromUtf8("\351\273\221\344\275\223"));
        font1.setPointSize(16);
        progressBar->setFont(font1);
        progressBar->setValue(0);
        progressBar->setTextVisible(true);
        btConfirm = new QPushButton(UpgradeProgramClass);
        btConfirm->setObjectName(QString::fromUtf8("btConfirm"));
        btConfirm->setGeometry(QRect(230, 120, 141, 31));
        btCancel = new QPushButton(UpgradeProgramClass);
        btCancel->setObjectName(QString::fromUtf8("btCancel"));
        btCancel->setGeometry(QRect(40, 120, 131, 31));
        btComplete = new QPushButton(UpgradeProgramClass);
        btComplete->setObjectName(QString::fromUtf8("btComplete"));
        btComplete->setGeometry(QRect(230, 120, 141, 31));

        retranslateUi(UpgradeProgramClass);

        QMetaObject::connectSlotsByName(UpgradeProgramClass);
    } // setupUi

    void retranslateUi(QDialog *UpgradeProgramClass)
    {
        UpgradeProgramClass->setWindowTitle(QApplication::translate("UpgradeProgramClass", "UpgradeProgram", nullptr));
        label->setText(QApplication::translate("UpgradeProgramClass", "\347\250\213\345\272\217\346\255\243\345\234\250\345\215\207\347\272\247......", nullptr));
        btConfirm->setText(QApplication::translate("UpgradeProgramClass", "\347\241\256\345\256\232", nullptr));
        btCancel->setText(QApplication::translate("UpgradeProgramClass", "\345\217\226\346\266\210", nullptr));
        btComplete->setText(QApplication::translate("UpgradeProgramClass", "\345\256\214\346\210\220", nullptr));
    } // retranslateUi

};

namespace Ui {
    class UpgradeProgramClass: public Ui_UpgradeProgramClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_UPGRADEPROGRAM_H
