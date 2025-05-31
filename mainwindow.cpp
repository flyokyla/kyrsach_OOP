#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "equipmentdialog.h"
#include "aboutdialog.h"
#include "chartwindow.h"
#include "settings.h"
#include "comboboxdelegate.h"
#include "datedelegate.h"
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QPrintDialog>
#include <QPrinter>
#include <QTextDocument>
#include <QTextStream>
#include <QSortFilterProxyModel>
#include <QClipboard>
#include <QHeaderView>
#include <QContextMenuEvent>
#include <QTabBar>
#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>
#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableView>
#include <QLineEdit>
#include <QFile>
#include <QRegularExpression>
#include <QTranslator>
#include <QDebug>
#include <QLabel>
#include <QComboBox>
#include <QSet>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    
    m_tabWidget = ui->tabWidget;
    connect(m_tabWidget, &QTabWidget::currentChanged, this, &MainWindow::onTabChanged);
    connect(m_tabWidget, &QTabWidget::tabCloseRequested, this, &MainWindow::onTabCloseRequested);
    
    setupConnections();
    restoreState();
    loadLanguage(Settings::language());
    
    // Создаем первый таб
    createNewTab();
}

MainWindow::~MainWindow() {
    saveState();
    // Очищаем все табы
    for (TabData &tab : m_tabs) {
        delete tab.model;
        delete tab.proxyModel;
    }
    delete ui;
}

void MainWindow::changeEvent(QEvent *event) {
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
    }
    QMainWindow::changeEvent(event);
}

void MainWindow::closeEvent(QCloseEvent *event) {
    saveState();
    event->accept();
}

int MainWindow::createNewTab(const QString &fileName) {
    TabData tabData;
    tabData.model = new EquipmentModel(this);
    tabData.proxyModel = new QSortFilterProxyModel(this);
    tabData.proxyModel->setSourceModel(tabData.model);
    tabData.fileName = fileName;
    tabData.isModified = false;
    tabData.selectedColumn = -1; // По умолчанию поиск по ID (столбец 0)
    
    // Создаем виджет для таба
    QWidget *tabWidget = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(tabWidget);
    
    // Создаем поиск
    QHBoxLayout *searchLayout = new QHBoxLayout();
    tabData.searchEdit = new QLineEdit();
    tabData.searchEdit->setPlaceholderText(tr("Search by ID (click column header to change)..."));
    searchLayout->addWidget(tabData.searchEdit);
    searchLayout->addStretch();
    layout->addLayout(searchLayout);
    
    // Создаем фильтры по столбцам
    tabData.filterWidget = new QWidget();
    QHBoxLayout *filterLayout = new QHBoxLayout(tabData.filterWidget);
    filterLayout->addWidget(new QLabel(tr("Column Filter:")));
    
    tabData.columnFilterCombo = new QComboBox();
    tabData.columnFilterCombo->setEditable(true);
    tabData.columnFilterCombo->setVisible(false); // Скрыто по умолчанию
    filterLayout->addWidget(tabData.columnFilterCombo);
    filterLayout->addStretch();
    layout->addWidget(tabData.filterWidget);
    
    // Создаем таблицу
    tabData.tableView = new QTableView();
    setupTabView(tabData);
    layout->addWidget(tabData.tableView);
    
    // Подключаем поиск по выбранному столбцу
    connect(tabData.searchEdit, &QLineEdit::textChanged, [this, &tabData](const QString &text) {
        filterBySelectedColumn(text);
    });
    
    // Подключаем фильтр по столбцам
    connect(tabData.columnFilterCombo, QOverload<const QString &>::of(&QComboBox::currentTextChanged),
            [this](const QString &text) {
                onColumnFilterChanged(text);
            });
    
    m_tabs.append(tabData);
    
    QString tabTitle = fileName.isEmpty() ? tr("New Document") : QFileInfo(fileName).baseName();
    int index = m_tabWidget->addTab(tabWidget, tabTitle);
    m_tabWidget->setCurrentIndex(index);
    
    return index;
}

void MainWindow::setupTabView(TabData &tabData) {
    tabData.tableView->setModel(tabData.proxyModel);
    tabData.tableView->setSortingEnabled(true);
    tabData.tableView->setContextMenuPolicy(Qt::CustomContextMenu);
    tabData.tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tabData.tableView->setAlternatingRowColors(true);
    tabData.tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    tabData.tableView->verticalHeader()->setVisible(false);
    
    // Подключаем клик по заголовкам для выбора столбца поиска
    connect(tabData.tableView->horizontalHeader(), &QHeaderView::sectionClicked,
            this, &MainWindow::onHeaderClicked);
    
    // Устанавливаем делегаты для комбо-боксов
    QStringList typeItems = {"Printer", "Monitor", "Desktop PC", "Laptop", "Plotter", "Scanner", 
                           "Projector", "Tablet", "UPS", "Router", "Phone", "Server", 
                           "Keyboard", "Mouse", "Webcam", "Headset", "External HDD", 
                           "Network Switch", "Label Printer", "Shredder"};
    
    QStringList statusItems = {"Active", "Inactive", "Maintenance", "Retired"};
    
    ComboBoxDelegate *typeDelegate = new ComboBoxDelegate(typeItems, this);
    ComboBoxDelegate *statusDelegate = new ComboBoxDelegate(statusItems, this);
    DateDelegate *dateDelegate = new DateDelegate(this);
    
    tabData.tableView->setItemDelegateForColumn(1, typeDelegate); // Type column
    tabData.tableView->setItemDelegateForColumn(4, dateDelegate); // Purchase Date column
    tabData.tableView->setItemDelegateForColumn(7, statusDelegate); // Status column
    
    // Подключаем контекстное меню
    connect(tabData.tableView, &QTableView::customContextMenuRequested,
            this, &MainWindow::showContextMenu);
}

TabData* MainWindow::getCurrentTab() {
    int index = m_tabWidget->currentIndex();
    if (index >= 0 && index < m_tabs.size()) {
        return &m_tabs[index];
    }
    return nullptr;
}

void MainWindow::setupConnections() {
    // Подключения для действий файла
    connect(ui->actionNew, &QAction::triggered, this, &MainWindow::newDocument);
    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::openFile);
    connect(ui->actionSave, &QAction::triggered, this, &MainWindow::saveFile);
    connect(ui->actionSaveAs, &QAction::triggered, this, &MainWindow::saveAs);
    connect(ui->actionPrint, &QAction::triggered, this, &MainWindow::print);
    connect(ui->actionExit, &QAction::triggered, qApp, &QApplication::quit);
    
    // Подключения для действий редактирования
    connect(ui->actionAdd, &QAction::triggered, this, &MainWindow::addRecord);
    connect(ui->actionEdit, &QAction::triggered, this, &MainWindow::editRecord);
    connect(ui->actionRemove, &QAction::triggered, this, &MainWindow::removeRecord);
    
    // Подключения для аналитики
    connect(ui->actionChart, &QAction::triggered, this, &MainWindow::showChart);
    
    // Подключения для языков
    connect(ui->actionEnglish, &QAction::triggered, this, &MainWindow::switchToEnglish);
    connect(ui->actionRussian, &QAction::triggered, this, &MainWindow::switchToRussian);
    connect(ui->actionSpanish, &QAction::triggered, this, &MainWindow::switchToSpanish);
    
    // Подключения для справки
    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::showAbout);
}

void MainWindow::retranslateUi() {
    ui->retranslateUi(this);
    
    // Обновляем заголовки всех таблиц при смене языка
    for (TabData &tab : m_tabs) {
        tab.model->updateHeaders();
    }
    
    updateWindowTitle();
}

void MainWindow::loadLanguage(const QString &lang) {
    if (m_currentLanguage == lang) return;

    m_currentLanguage = lang;
    qApp->removeTranslator(&m_translator);
    
    // Загружаем .qm файл
    QString qmFile = QString("office_equipment_%1.qm").arg(lang);
    if (m_translator.load(qmFile, qApp->applicationDirPath())) {
        qApp->installTranslator(&m_translator);
        retranslateUi();
    } else {
        qWarning() << "Cannot load translation file:" << qmFile;
    }
    
    Settings::saveLanguage(lang);
}

void MainWindow::newDocument() {
    createNewTab();
}

void MainWindow::openFile() {
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), "", tr("Text Files (*.txt)"));
    if (!fileName.isEmpty()) {
        loadFile(fileName);
    }
}

void MainWindow::loadFile(const QString &fileName) {
    int tabIndex = createNewTab(fileName);
    TabData *tab = &m_tabs[tabIndex];
    
    if (tab->model->loadFromFile(fileName)) {
        tab->fileName = fileName;
        m_tabWidget->setTabText(tabIndex, QFileInfo(fileName).baseName());
        updateWindowTitle();
        updateStatusBar();
    } else {
        QMessageBox::warning(this, tr("Error"), tr("Could not open file"));
        closeTab(tabIndex);
    }
}

void MainWindow::saveFile() {
    TabData *tab = getCurrentTab();
    if (!tab) return;
    
    if (tab->fileName.isEmpty()) {
        saveAs();
        return;
    }
    tab->model->saveToFile(tab->fileName);
    tab->isModified = false;
    statusBar()->showMessage(tr("File saved"), 2000);
}

void MainWindow::saveAs() {
    TabData *tab = getCurrentTab();
    if (!tab) return;
    
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"), "", tr("Text Files (*.txt)"));
    if (!fileName.isEmpty()) {
        if (!fileName.endsWith(".txt")) fileName += ".txt";
        tab->model->saveToFile(fileName);
        tab->fileName = fileName;
        tab->isModified = false;
        
        int currentIndex = m_tabWidget->currentIndex();
        m_tabWidget->setTabText(currentIndex, QFileInfo(fileName).baseName());
        updateWindowTitle();
        statusBar()->showMessage(tr("File saved as: %1").arg(fileName), 2000);
    }
}

void MainWindow::addRecord() {
    TabData *tab = getCurrentTab();
    if (!tab) return;
    
    EquipmentRecord record;
    record.id = QString::number(tab->model->rowCount() + 1);
    equipmentdialog dialog(record, this);
    if (dialog.exec() == QDialog::Accepted) {
        tab->model->addRecord(dialog.getRecord());
        tab->isModified = true;
        updateStatusBar();
    }
}

void MainWindow::editRecord() {
    TabData *tab = getCurrentTab();
    if (!tab) return;
    
    QModelIndex index = tab->tableView->currentIndex();
    if (!index.isValid()) return;

    int row = tab->proxyModel->mapToSource(index).row();
    EquipmentRecord record = tab->model->getRecord(row);
    equipmentdialog dialog(record, this);
    if (dialog.exec() == QDialog::Accepted) {
        tab->model->updateRecord(row, dialog.getRecord());
        tab->isModified = true;
    }
}

void MainWindow::removeRecord() {
    TabData *tab = getCurrentTab();
    if (!tab) return;
    
    QModelIndex index = tab->tableView->currentIndex();
    if (!index.isValid()) return;

    int row = tab->proxyModel->mapToSource(index).row();
    tab->model->removeRecord(row);
    tab->isModified = true;
    updateStatusBar();
}

void MainWindow::showAbout() {
    aboutdialog dialog(this);
    dialog.exec();
}

void MainWindow::switchToEnglish() {
    loadLanguage("en");
}

void MainWindow::switchToRussian() {
    loadLanguage("ru");
}

void MainWindow::switchToSpanish() {
    loadLanguage("es");
}

void MainWindow::showChart() {
    TabData *tab = getCurrentTab();
    if (!tab) return;
    
    chartwindow dialog(tab->model->getAllRecords(), this);
    dialog.exec();
}

void MainWindow::print() {
    TabData *tab = getCurrentTab();
    if (!tab) return;
    
    QPrintDialog printDialog(this);
    if (printDialog.exec() == QDialog::Accepted) {
        QPrinter *printer = printDialog.printer();
        
        QTextDocument doc;
        QString html = "<h2>" + tr("Equipment List") + "</h2><table border='1'>";
        html += "<tr><th>" + tr("ID") + "</th><th>" + tr("Type") + "</th><th>" + tr("Model") + "</th>";
        html += "<th>" + tr("Serial") + "</th><th>" + tr("Date") + "</th><th>" + tr("Price") + "</th>";
        html += "<th>" + tr("Location") + "</th><th>" + tr("Status") + "</th><th>" + tr("Description") + "</th></tr>";
        
        for (const EquipmentRecord &record : tab->model->getAllRecords()) {
            html += QString("<tr><td>%1</td><td>%2</td><td>%3</td><td>%4</td><td>%5</td><td>%6</td><td>%7</td><td>%8</td><td>%9</td></tr>")
                    .arg(record.id).arg(record.type).arg(record.model).arg(record.serial)
                    .arg(record.purchase_date.toString("yyyy-MM-dd"))
                    .arg(record.price).arg(record.location)
                    .arg(record.status).arg(record.notes);
        }
        html += "</table>";
        
        doc.setHtml(html);
        doc.print(printer);
    }
}

void MainWindow::updateWindowTitle() {
    TabData *tab = getCurrentTab();
    if (tab) {
        QString title = tab->fileName.isEmpty() ? tr("New Document") : QFileInfo(tab->fileName).baseName();
        if (tab->isModified) {
            title += " *";
        }
        setWindowTitle(QString("%1 - Office Equipment Manager").arg(title));
    } else {
        setWindowTitle("Office Equipment Manager");
    }
}

void MainWindow::updateStatusBar() {
    TabData *tab = getCurrentTab();
    if (tab) {
        int recordCount = tab->model->rowCount();
        statusBar()->showMessage(tr("Records: %1").arg(recordCount));
    } else {
        statusBar()->clearMessage();
    }
}

void MainWindow::showContextMenu(const QPoint &pos) {
    TabData *tab = getCurrentTab();
    if (!tab) return;
    
    QModelIndex index = tab->tableView->indexAt(pos);
    if (!index.isValid()) return;
    
    QMenu contextMenu(this);
    contextMenu.addAction(ui->actionEdit);
    contextMenu.addAction(ui->actionRemove);
    contextMenu.exec(tab->tableView->mapToGlobal(pos));
}

void MainWindow::filterTable(const QString &text) {
    TabData *tab = getCurrentTab();
    if (tab) {
        tab->proxyModel->setFilterRegularExpression(QRegularExpression(text, QRegularExpression::CaseInsensitiveOption));
    }
}

void MainWindow::onTabChanged(int index) {
    Q_UNUSED(index)
    updateWindowTitle();
    updateStatusBar();
}

void MainWindow::onTabCloseRequested(int index) {
    closeTab(index);
}

bool MainWindow::closeTab(int index) {
    if (index < 0 || index >= m_tabs.size()) return false;
    
    TabData &tab = m_tabs[index];
    if (tab.isModified) {
        QMessageBox::StandardButton ret = QMessageBox::question(this, tr("Save Changes"),
            tr("The document has been modified. Do you want to save your changes?"),
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        
        if (ret == QMessageBox::Save) {
            // Сохраняем файл
            if (tab.fileName.isEmpty()) {
                // Логика Save As
            } else {
                tab.model->saveToFile(tab.fileName);
            }
        } else if (ret == QMessageBox::Cancel) {
            return false;
        }
    }
    
    // Удаляем объекты
    delete tab.model;
    delete tab.proxyModel;
    
    m_tabs.removeAt(index);
    m_tabWidget->removeTab(index);
    
    // Если это был последний таб, создаем новый
    if (m_tabs.isEmpty()) {
        createNewTab();
    }
    
    return true;
}

void MainWindow::saveState() {
    Settings::saveWindowGeometry(saveGeometry());
    Settings::saveWindowState(QMainWindow::saveState());
}

void MainWindow::restoreState() {
    restoreGeometry(Settings::windowGeometry());
    QMainWindow::restoreState(Settings::windowState());
}

void MainWindow::onHeaderClicked(int logicalIndex) {
    TabData *tab = getCurrentTab();
    if (!tab) return;
    
    // Устанавливаем выбранный столбец
    tab->selectedColumn = logicalIndex;
    
    // Обновляем placeholder текст для поиска
    QStringList columnNames = {"ID", "Type", "Model", "Serial", "Purchase Date", 
                              "Price", "Location", "Status", "Notes"};
    
    if (logicalIndex >= 0 && logicalIndex < columnNames.size()) {
        tab->searchEdit->setPlaceholderText(tr("Search by %1...").arg(columnNames[logicalIndex]));
        
        // Настраиваем фильтр по столбцам
        setupColumnFilters(*tab);
        updateColumnFilter(*tab, logicalIndex);
    }
    
    // Применяем текущий поиск к новому столбцу
    filterBySelectedColumn(tab->searchEdit->text());
}

void MainWindow::filterBySelectedColumn(const QString &text) {
    TabData *tab = getCurrentTab();
    if (!tab) return;
    
    if (text.isEmpty()) {
        tab->proxyModel->setFilterRegularExpression(QRegularExpression());
        return;
    }
    
    // Если столбец не выбран, ищем по ID (столбец 0)
    int columnToFilter = tab->selectedColumn >= 0 ? tab->selectedColumn : 0;
    
    tab->proxyModel->setFilterKeyColumn(columnToFilter);
    tab->proxyModel->setFilterRegularExpression(
        QRegularExpression(text, QRegularExpression::CaseInsensitiveOption)
    );
}

void MainWindow::setupColumnFilters(TabData &tabData) {
    tabData.columnFilterCombo->clear();
    tabData.columnFilterCombo->setVisible(true);
    tabData.filterWidget->setVisible(true);
}

void MainWindow::updateColumnFilter(TabData &tabData, int column) {
    if (!tabData.model) return;
    
    // Получаем уникальные значения для выбранного столбца
    QSet<QString> uniqueValues;
    for (int row = 0; row < tabData.model->rowCount(); ++row) {
        QModelIndex index = tabData.model->index(row, column);
        QString value = tabData.model->data(index, Qt::DisplayRole).toString();
        if (!value.isEmpty()) {
            uniqueValues.insert(value);
        }
    }
    
    // Заполняем комбобокс уникальными значениями
    tabData.columnFilterCombo->clear();
    tabData.columnFilterCombo->addItem(tr("All")); // Показать все
    
    QStringList sortedValues = uniqueValues.values();
    sortedValues.sort();
    tabData.columnFilterCombo->addItems(sortedValues);
}

void MainWindow::onColumnFilterChanged(const QString &filterValue) {
    TabData *tab = getCurrentTab();
    if (!tab) return;
    
    if (filterValue == tr("All") || filterValue.isEmpty()) {
        // Убираем фильтр, показываем все записи
        tab->proxyModel->setFilterRegularExpression(QRegularExpression());
    } else {
        // Применяем фильтр для выбранного столбца
        int columnToFilter = tab->selectedColumn >= 0 ? tab->selectedColumn : 0;
        
        tab->proxyModel->setFilterKeyColumn(columnToFilter);
        tab->proxyModel->setFilterFixedString(filterValue);
    }
}
