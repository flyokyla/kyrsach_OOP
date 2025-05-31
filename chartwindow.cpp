#include "chartwindow.h"
#include "ui_chartwindow.h"
#include <QtCharts/QChartView>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QLegend>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>
#include <QtCharts/QPieSeries>
#include <QtCharts/QChart>
#include <QMap>

chartwindow::chartwindow(const QVector<EquipmentRecord> &records, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::chartwindow),
    m_records(records)
{
    ui->setupUi(this);

    ui->comboBox->addItem(tr("Equipment by Type"));
    ui->comboBox->addItem(tr("Price Distribution"));
    ui->comboBox->addItem(tr("Location Statistics"));

    updateChart(0);
}

chartwindow::~chartwindow() {
    delete ui;
}

void chartwindow::on_comboBox_currentIndexChanged(int index)
{
    updateChart(index);
}

void chartwindow::updateChart(int type)
{
    QChart *chart = new QChart();

    if (type == 0) { // Equipment by Type
        QMap<QString, int> typeCount;
        for (const EquipmentRecord& eq : m_records) {
            typeCount[eq.type]++;
        }

        QBarSeries *series = new QBarSeries();
        QBarSet *set = new QBarSet(tr("Equipment Count"));

        QStringList categories;
        for (auto it = typeCount.begin(); it != typeCount.end(); ++it) {
            *set << it.value();
            categories << it.key();
        }

        series->append(set);
        chart->addSeries(series);

        QBarCategoryAxis *axisX = new QBarCategoryAxis();
        axisX->append(categories);
        chart->addAxis(axisX, Qt::AlignBottom);
        series->attachAxis(axisX);

        QValueAxis *axisY = new QValueAxis();
        chart->addAxis(axisY, Qt::AlignLeft);
        series->attachAxis(axisY);

        chart->setTitle(tr("Equipment Count by Type"));
    }
    else if (type == 1) { // Price Distribution
        QBarSeries *series = new QBarSeries();
        QBarSet *set = new QBarSet(tr("Price"));

        QStringList categories;
        for (const EquipmentRecord& eq : m_records) {
            *set << eq.price;
            categories << eq.model;
        }

        series->append(set);
        chart->addSeries(series);

        QBarCategoryAxis *axisX = new QBarCategoryAxis();
        axisX->append(categories);
        chart->addAxis(axisX, Qt::AlignBottom);
        series->attachAxis(axisX);

        QValueAxis *axisY = new QValueAxis();
        chart->addAxis(axisY, Qt::AlignLeft);
        series->attachAxis(axisY);

        chart->setTitle(tr("Price Distribution by Equipment"));
    }
    else { // Location Statistics
        QMap<QString, int> locationCount;
        for (const EquipmentRecord& eq : m_records) {
            locationCount[eq.location]++;
        }

        QPieSeries *series = new QPieSeries();
        for (auto it = locationCount.begin(); it != locationCount.end(); ++it) {
            series->append(it.key(), it.value());
        }

        chart->addSeries(series);
        chart->setTitle(tr("Equipment Distribution by Location"));
    }

    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);

    ui->chartView->setChart(chart);
    ui->chartView->setRenderHint(QPainter::Antialiasing);
}
