#ifndef CHARTWINDOW_H
#define CHARTWINDOW_H

#include <QDialog>
#include <QVector>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QPieSeries>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QValueAxis>
#include <QtCharts/QBarCategoryAxis>
#include "equipmentrecord.h"

namespace Ui {
class chartwindow;
}

class chartwindow : public QDialog
{
    Q_OBJECT

public:
    explicit chartwindow(const QVector<EquipmentRecord> &records, QWidget *parent = nullptr);
    ~chartwindow();

private slots:
    void on_comboBox_currentIndexChanged(int index);

private:
    Ui::chartwindow *ui;
    QVector<EquipmentRecord> m_records;

    void updateChart(int type);
};

#endif // CHARTWINDOW_H
