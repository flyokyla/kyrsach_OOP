#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QDialog>
#include <QEvent>

namespace Ui {
class aboutdialog;
}

class aboutdialog : public QDialog
{
    Q_OBJECT

public:
    explicit aboutdialog(QWidget *parent = nullptr);
    ~aboutdialog();

protected:
    void changeEvent(QEvent *event) override;

private:
    Ui::aboutdialog *ui;
    void updateText();
};

#endif // ABOUTDIALOG_H
