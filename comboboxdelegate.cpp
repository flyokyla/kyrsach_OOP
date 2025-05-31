#include "comboboxdelegate.h"
#include <QComboBox>

ComboBoxDelegate::ComboBoxDelegate(const QStringList &items, QObject *parent)
    : QStyledItemDelegate(parent), m_items(items)
{
}

QWidget *ComboBoxDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                                       const QModelIndex &index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)
    
    QComboBox *comboBox = new QComboBox(parent);
    comboBox->addItems(m_items);
    comboBox->setEditable(false);
    return comboBox;
}

void ComboBoxDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QComboBox *comboBox = qobject_cast<QComboBox*>(editor);
    if (comboBox) {
        QString value = index.model()->data(index, Qt::EditRole).toString();
        int idx = comboBox->findText(value);
        if (idx >= 0) {
            comboBox->setCurrentIndex(idx);
        }
    }
}

void ComboBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                   const QModelIndex &index) const
{
    QComboBox *comboBox = qobject_cast<QComboBox*>(editor);
    if (comboBox) {
        model->setData(index, comboBox->currentText(), Qt::EditRole);
    }
}

void ComboBoxDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                                           const QModelIndex &index) const
{
    Q_UNUSED(index)
    editor->setGeometry(option.rect);
} 