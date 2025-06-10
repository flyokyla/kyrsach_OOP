#include "aboutdialog.h"
#include "ui_aboutdialog.h"
#include <QPushButton>
#include <QCoreApplication>

aboutdialog::aboutdialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::aboutdialog)
{
    ui->setupUi(this);
    
    // Устанавливаем переведенный текст для кнопки OK
    QPushButton *okButton = ui->buttonBox->button(QDialogButtonBox::Ok);
    if (okButton) {
        okButton->setText(QCoreApplication::translate("QPlatformTheme", "OK"));
    }
    
    updateText();
}

aboutdialog::~aboutdialog() {
    delete ui;
}

void aboutdialog::updateText() {
    ui->label->setText(
        tr("Office Equipment Manager") + "\n\n" +
        tr("Program for managing office equipment\nof a construction organization") + "\n\n" +
        tr("Author") + ": Капытин Андрей Игоревич ИЦТМС 2-1\n\n" +
        tr("Version") + ": 1.0\n" +
        tr("Year") + ": 2025"
    );
}

void aboutdialog::changeEvent(QEvent *event) {
    if (event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
        updateText();
    }
    QDialog::changeEvent(event);
}
