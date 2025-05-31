#include "datedelegate.h"
#include <QDateEdit>
#include <QDate>

DateDelegate::DateDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

QWidget *DateDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                                   const QModelIndex &index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)

    QDateEdit *editor = new QDateEdit(parent);
    editor->setDisplayFormat("yyyy-MM-dd");
    editor->setCalendarPopup(true);
    editor->setDate(QDate::currentDate());
    return editor;
}

void DateDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QString value = index.model()->data(index, Qt::EditRole).toString();
    QDateEdit *dateEdit = static_cast<QDateEdit*>(editor);
    
    QDate date = QDate::fromString(value, Qt::ISODate);
    if (date.isValid()) {
        dateEdit->setDate(date);
    } else {
        dateEdit->setDate(QDate::currentDate());
    }
}

void DateDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                               const QModelIndex &index) const
{
    QDateEdit *dateEdit = static_cast<QDateEdit*>(editor);
    QDate date = dateEdit->date();
    QString value = date.toString(Qt::ISODate);
    model->setData(index, value, Qt::EditRole);
}

void DateDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                                       const QModelIndex &index) const
{
    Q_UNUSED(index)
    editor->setGeometry(option.rect);
} 