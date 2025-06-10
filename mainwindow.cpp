#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "equipmentdialog.h"
#include "aboutdialog.h"
#include "chartwindow.h"
#include "settings.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QPrintDialog>
#include <QPrinter>
#include <QKeyEvent>
#include <QShortcut>
#include <QMimeData>
#include <QClipboard>
#include <QMenu>
#include <QDir>
#include <QTextStream>
#include <QStatusBar>
#include <QComboBox>
#include <QLineEdit>
#include <QHeaderView>
#include <QMap>
#include <QRegularExpression>
#include <QPushButton>
#include <QLibraryInfo>
#include <QCoreApplication>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    
    m_tabWidget = ui->tabWidget;
    m_tabWidget->setTabsClosable(true); // Включаем возможность закрытия вкладок
    connect(m_tabWidget, &QTabWidget::currentChanged, this, &MainWindow::onTabChanged);
    connect(m_tabWidget, &QTabWidget::tabCloseRequested, this, &MainWindow::onTabCloseRequested);
    
    setupConnections();
    restoreState();
    loadLanguage(Settings::language());
    
    // Создаем первый таб если нет вкладок
    if (m_tabs.isEmpty()) {
        createNewTab();
    }
    
    // Добавляем горячие клавиши
    QShortcut* copyShortcut = new QShortcut(QKeySequence::Copy, this);
    connect(copyShortcut, &QShortcut::activated, this, &MainWindow::copySelectedCells);
    
    QShortcut* pasteShortcut = new QShortcut(QKeySequence::Paste, this);
    connect(pasteShortcut, &QShortcut::activated, this, &MainWindow::pasteFromClipboard);
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
    filterLayout->setContentsMargins(0, 0, 0, 0);
    QLabel *filterLabel = new QLabel(tr("Column Filter:"));
    
    tabData.columnFilterCombo = new QComboBox();
    tabData.columnFilterCombo->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    tabData.columnFilterCombo->setVisible(false); // Скрыто по умолчанию
    
    filterLayout->addWidget(filterLabel);
    filterLayout->addWidget(tabData.columnFilterCombo);
    filterLayout->addStretch();
    layout->addWidget(tabData.filterWidget);
    
    // Создаем таблицу
    tabData.tableView = new QTableView();
    tabData.tableView->setModel(tabData.proxyModel);
    tabData.tableView->setSortingEnabled(true);
    tabData.tableView->horizontalHeader()->setSectionsClickable(true);
    tabData.tableView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    tabData.tableView->setDragEnabled(true);
    tabData.tableView->setAcceptDrops(true);
    tabData.tableView->setDropIndicatorShown(true);
    tabData.tableView->setDragDropMode(QAbstractItemView::DragDrop);
    
    // Настраиваем размеры столбцов
    for (int i = 0; i < tabData.model->columnCount(); ++i) {
        tabData.tableView->setColumnWidth(i, 100);
    }
    
    // Устанавливаем высоту строк
    tabData.tableView->verticalHeader()->setDefaultSectionSize(30);
    
    // Подключаем клик по заголовку для выбора столбца поиска
    connect(tabData.tableView->horizontalHeader(), &QHeaderView::sectionClicked, 
            this, &MainWindow::onHeaderClicked);
    
    // Подключаем контекстное меню
    tabData.tableView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(tabData.tableView, &QTableView::customContextMenuRequested, 
            this, &MainWindow::showContextMenu);
    
    // Подключаем поиск по выбранному столбцу
    connect(tabData.searchEdit, &QLineEdit::textChanged, 
            this, &MainWindow::filterBySelectedColumn);
    
    // Подключаем фильтр по столбцам
    connect(tabData.columnFilterCombo, &QComboBox::currentTextChanged,
            this, &MainWindow::onColumnFilterChanged);
    
    layout->addWidget(tabData.tableView);
    
    // Добавляем TabData в список
    m_tabs.append(tabData);
    
    // Добавляем виджет на вкладку
    QString tabTitle = fileName.isEmpty() ? tr("New Document") : QFileInfo(fileName).baseName();
    int index = m_tabWidget->addTab(tabWidget, tabTitle);
    m_tabWidget->setCurrentIndex(index);
    
    return index;
}

TabData* MainWindow::getCurrentTab() const {
    int index = m_tabWidget->currentIndex();
    if (index >= 0 && index < m_tabs.size()) {
        return &(const_cast<MainWindow*>(this)->m_tabs[index]);
    }
    return nullptr;
}

void MainWindow::setupConnections() {
    // File menu
    connect(ui->actionNew, &QAction::triggered, this, &MainWindow::newDocument);
    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::openFile);
    connect(ui->actionSave, &QAction::triggered, this, &MainWindow::saveFile);
    connect(ui->actionSaveAs, &QAction::triggered, this, &MainWindow::saveAs);
    connect(ui->actionPrint, &QAction::triggered, this, &MainWindow::print);
    
    // Add CSV import/export menu
    QMenu *csvMenu = ui->menuFile->addMenu(tr("&CSV"));
    
    // Создаем действия CSV программно и сохраняем их
    m_exportCSVAction = new QAction(tr("Export to CSV..."), this);
    connect(m_exportCSVAction, &QAction::triggered, this, &MainWindow::exportToCSV);
    csvMenu->addAction(m_exportCSVAction);
    
    m_importCSVAction = new QAction(tr("Import from CSV..."), this);
    connect(m_importCSVAction, &QAction::triggered, this, &MainWindow::importFromCSV);
    csvMenu->addAction(m_importCSVAction);
    
    connect(ui->actionExit, &QAction::triggered, this, &QApplication::quit);
    
    // Edit menu
    connect(ui->actionAdd, &QAction::triggered, this, &MainWindow::addRecord);
    connect(ui->actionEdit, &QAction::triggered, this, &MainWindow::editRecord);
    connect(ui->actionRemove, &QAction::triggered, this, &MainWindow::removeRecord);
    
    // Подключаем существующие действия Copy и Paste
    connect(ui->actionCopy, &QAction::triggered, this, &MainWindow::copySelectedCells);
    connect(ui->actionPaste, &QAction::triggered, this, &MainWindow::pasteFromClipboard);
    
    // Help menu
    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::showAbout);
    
    // Chart menu
    connect(ui->actionChart, &QAction::triggered, this, &MainWindow::showChart);
    
    // Language menu
    connect(ui->actionEnglish, &QAction::triggered, this, &MainWindow::switchToEnglish);
    connect(ui->actionRussian, &QAction::triggered, this, &MainWindow::switchToRussian);
    connect(ui->actionSpanish, &QAction::triggered, this, &MainWindow::switchToSpanish);
}

void MainWindow::retranslateUi() {
    ui->retranslateUi(this);
    
    // Обновляем заголовки всех таблиц при смене языка
    for (TabData &tab : m_tabs) {
        tab.model->updateHeaders();
    }
    
    // Обновляем тексты для CSV меню
    if (m_exportCSVAction) {
        m_exportCSVAction->setText(tr("Export to CSV..."));
    }
    if (m_importCSVAction) {
        m_importCSVAction->setText(tr("Import from CSV..."));
    }
    
    updateWindowTitle();
}

void MainWindow::loadLanguage(const QString &lang) {
    if (m_currentLanguage == lang) return;

    m_currentLanguage = lang;
    
    // Удаляем текущие переводчики
    qApp->removeTranslator(&m_translator);
    qApp->removeTranslator(&m_qtTranslator);
    
    // 1. Загружаем системные переводы Qt (OK/Cancel/Close и т.д.)
    QString qtTransFile = QString("qt_%1").arg(lang);
    qDebug() << "Loading Qt translations for" << qtTransFile;
    
    // Список путей для поиска системных переводов Qt
    QStringList qtPaths;
    qtPaths << QLibraryInfo::location(QLibraryInfo::TranslationsPath);
    qtPaths << "/usr/share/qt5/translations";
    qtPaths << "/usr/share/qt/translations";
    qtPaths << "/usr/lib/qt/translations";
    qtPaths << "/usr/lib/x86_64-linux-gnu/qt5/translations";
    qtPaths << "/usr/lib/i386-linux-gnu/qt5/translations";
    qtPaths << qApp->applicationDirPath() + "/translations";
    qtPaths << QDir::currentPath() + "/translations";
    
    bool qtLoaded = false;
    for (const QString &path : qtPaths) {
        qDebug() << "Trying path:" << path;
        if (m_qtTranslator.load(qtTransFile, path)) {
            qApp->installTranslator(&m_qtTranslator);
            qDebug() << "Loaded Qt translations from:" << path;
            qtLoaded = true;
            break;
        }
    }
    
    if (!qtLoaded) {
        qWarning() << "Could not load Qt translations for" << lang << "from any location";
    }
    
    // 2. Загружаем .qm файл нашего приложения
    QString qmFile = QString("office_equipment_%1.qm").arg(lang);
    
    // Список путей для поиска файлов переводов
    QStringList searchPaths;
    searchPaths << qApp->applicationDirPath() + "/translations";
    searchPaths << qApp->applicationDirPath();
    searchPaths << QDir::currentPath() + "/translations";
    searchPaths << QDir::currentPath();
    
    bool loaded = false;
    for (const QString &path : searchPaths) {
        if (m_translator.load(qmFile, path)) {
            qApp->installTranslator(&m_translator);
            qDebug() << "Loaded application translations from:" << path;
            loaded = true;
            break;
        }
    }
    
    if (loaded) {
        retranslateUi();
    } else {
        qWarning() << "Cannot load translation file:" << qmFile << "in paths:" << searchPaths;
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
    if (fileName.isEmpty())
        return;
        
    QFile file(fileName);
    if (!file.exists()) {
        QMessageBox msgBox(this);
        msgBox.setWindowTitle(tr("Error"));
        msgBox.setText(tr("File does not exist: %1").arg(fileName));
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.addButton(QCoreApplication::translate("QPlatformTheme", "OK"), QMessageBox::AcceptRole);
        msgBox.exec();
        return;
    }
    
    int tabIndex = createNewTab(fileName);
    TabData *tab = &m_tabs[tabIndex];
    
    if (tab->model->loadFromFile(fileName)) {
        tab->fileName = fileName;
        m_tabWidget->setTabText(tabIndex, QFileInfo(fileName).baseName());
        updateWindowTitle();
        updateStatusBar();
    } else {
        QMessageBox msgBox(this);
        msgBox.setWindowTitle(tr("Error"));
        msgBox.setText(tr("Could not open file: %1").arg(fileName));
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.addButton(QCoreApplication::translate("QPlatformTheme", "OK"), QMessageBox::AcceptRole);
        msgBox.exec();
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
    printDialog.setWindowTitle(tr("Print Document"));
    
    // Перевод кнопок диалога печати
    printDialog.setOption(QAbstractPrintDialog::PrintDialogOption::PrintShowPageSize, true);
    QAbstractButton* printButton = printDialog.findChild<QAbstractButton*>("print");
    if (printButton) printButton->setText(QCoreApplication::translate("QPlatformTheme", "Print"));
    
    QAbstractButton* cancelButton = printDialog.findChild<QAbstractButton*>("cancel");
    if (cancelButton) cancelButton->setText(QCoreApplication::translate("QPlatformTheme", "Cancel"));
    
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

void MainWindow::showContextMenu(const QPoint &pos)
{
    QTableView* tableView = getCurrentTableView();
    if (!tableView)
        return;

    // Создаем контекстное меню
    QMenu contextMenu(this);
    
    // Получаем индекс для клика
    QModelIndex index = tableView->indexAt(pos);
    
    if (index.isValid()) {
        // Добавляем пункты для редактирования и удаления записи
        contextMenu.addAction(ui->actionEdit);
        contextMenu.addAction(ui->actionRemove);
        
        // Добавляем сепаратор
        contextMenu.addSeparator();
    }
    
    // Добавляем пункт для создания записи
    contextMenu.addAction(ui->actionAdd);
    
    // Добавляем Copy/Paste
    contextMenu.addSeparator();
    contextMenu.addAction(ui->actionCopy);
    contextMenu.addAction(ui->actionPaste);
    
    // Добавляем опции для экспорта/импорта
    contextMenu.addSeparator();
    contextMenu.addAction(m_exportCSVAction);
    contextMenu.addAction(m_importCSVAction);
    
    // Показываем контекстное меню
    contextMenu.exec(tableView->viewport()->mapToGlobal(pos));
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
    if (index >= 0 && index < m_tabWidget->count() && index < m_tabs.size()) {
        closeTab(index);
    }
}

bool MainWindow::closeTab(int index) {
    if (index < 0 || index >= m_tabs.size())
        return false;
    
    TabData &tab = m_tabs[index];
    if (tab.isModified) {
        // Создаем вручную QMessageBox для возможности перевода кнопок
        QMessageBox msgBox(this);
        msgBox.setWindowTitle(tr("Save Changes"));
        msgBox.setText(tr("The document has been modified. Do you want to save your changes?"));
        msgBox.setIcon(QMessageBox::Question);
        
        QPushButton *saveButton = msgBox.addButton(QCoreApplication::translate("QPlatformTheme", "Save"), QMessageBox::AcceptRole);
        QPushButton *discardButton = msgBox.addButton(QCoreApplication::translate("QPlatformTheme", "Discard"), QMessageBox::DestructiveRole);
        QPushButton *cancelButton = msgBox.addButton(QCoreApplication::translate("QPlatformTheme", "Cancel"), QMessageBox::RejectRole);
        
        msgBox.exec();
        
        if (msgBox.clickedButton() == saveButton) {
            // Сохраняем файл
            m_tabWidget->setCurrentIndex(index);
            if (tab.fileName.isEmpty()) {
                saveAs();
                // Если после saveAs файл всё ещё не имеет имени, значит пользователь отменил сохранение
                if (tab.fileName.isEmpty()) {
                    return false;
                }
            } else {
                tab.model->saveToFile(tab.fileName);
            }
        } else if (msgBox.clickedButton() == cancelButton) {
            return false;
        }
    }
    
    // Удаляем виджет вкладки
    QWidget *tabWidget = m_tabWidget->widget(index);
    if (tabWidget) {
        delete tabWidget;
    }
    
    // Удаляем объекты
    delete tab.model;
    delete tab.proxyModel;
    
    // Удаляем TabData
    m_tabs.removeAt(index);
    
    // Если это был последний таб, создаем новый
    if (m_tabs.isEmpty()) {
        createNewTab();
    }
    
    updateWindowTitle();
    updateStatusBar();
    
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

void MainWindow::keyPressEvent(QKeyEvent *event) {
    switch (event->key()) {
        case Qt::Key_C:
            if (event->modifiers() & Qt::ControlModifier) {
                copySelectedCells();
            }
            break;
            
        case Qt::Key_V:
            if (event->modifiers() & Qt::ControlModifier) {
                pasteFromClipboard();
            }
            break;
            
        default:
            QMainWindow::keyPressEvent(event);
    }
}

void MainWindow::copySelectedCells()
{
    QTableView* tableView = getCurrentTableView();
    if (!tableView)
        return;

    // Получаем выделенные ячейки
    QModelIndexList selectedIndexes = tableView->selectionModel()->selectedIndexes();
    if (selectedIndexes.isEmpty())
        return;

    // Находим минимальные и максимальные строки и столбцы для определения размеров буфера
    int minRow = INT_MAX, maxRow = 0, minCol = INT_MAX, maxCol = 0;
    for (const QModelIndex &index : selectedIndexes) {
        minRow = std::min(minRow, index.row());
        maxRow = std::max(maxRow, index.row());
        minCol = std::min(minCol, index.column());
        maxCol = std::max(maxCol, index.column());
    }

    // Создаем строку CSV
    QString csv;
    for (int row = minRow; row <= maxRow; ++row) {
        QStringList rowData;
        for (int col = minCol; col <= maxCol; ++col) {
            QModelIndex index = tableView->model()->index(row, col);
            if (tableView->selectionModel()->isSelected(index)) {
                QString data = tableView->model()->data(index).toString();
                // Экранируем кавычки и добавляем кавычки если нужно
                if (data.contains(',') || data.contains('"') || data.contains('\n')) {
                    data.replace("\"", "\"\"");
                    data = "\"" + data + "\"";
                }
                rowData.append(data);
            } else {
                rowData.append("");
            }
        }
        csv += rowData.join(",") + "\n";
    }

    // Помещаем данные в буфер обмена
    QMimeData* mimeData = new QMimeData;
    mimeData->setData("text/csv", csv.toUtf8());
    mimeData->setText(csv);
    QApplication::clipboard()->setMimeData(mimeData);
    
    statusBar()->showMessage(tr("Selected data copied to clipboard"), 3000);
}

void MainWindow::pasteFromClipboard()
{
    QTableView* tableView = getCurrentTableView();
    if (!tableView)
        return;

    // Получаем данные из буфера обмена
    const QMimeData* mimeData = QApplication::clipboard()->mimeData();
    if (!mimeData->hasText())
        return;

    // Получаем текущий индекс, куда будем вставлять данные
    QModelIndex currentIndex = tableView->currentIndex();
    if (!currentIndex.isValid()) {
        // Если нет текущего индекса, используем верхний левый угол
        currentIndex = tableView->model()->index(0, 0);
    }

    int startRow = currentIndex.row();
    int startCol = currentIndex.column();

    // Парсим CSV
    QString csv = mimeData->text();
    QStringList rows = csv.split('\n', Qt::SkipEmptyParts);
    
    // Создаем модифицированные записи
    for (int i = 0; i < rows.size(); ++i) {
        QString row = rows[i];
        QStringList columns;
        
        // Корректно обрабатываем CSV с учетом кавычек
        bool inQuote = false;
        int quoteStart = -1;
        QString col;
        
        for (int j = 0; j < row.size(); ++j) {
            const QChar c = row[j];
            
            if (c == '"') {
                if (inQuote) {
                    // Проверяем двойные кавычки
                    if (j + 1 < row.size() && row[j + 1] == '"') {
                        col += '"';
                        j++; // Пропускаем следующую кавычку
                    } else {
                        inQuote = false;
                    }
                } else {
                    inQuote = true;
                    quoteStart = j;
                }
            } else if (c == ',' && !inQuote) {
                columns.append(col);
                col.clear();
            } else {
                col += c;
            }
        }
        
        if (!col.isEmpty()) {
            columns.append(col);
        }
        
        // Обновляем данные модели
        for (int j = 0; j < columns.size(); ++j) {
            int row = startRow + i;
            int col = startCol + j;
            
            if (row < tableView->model()->rowCount() && col < tableView->model()->columnCount()) {
                QModelIndex idx = tableView->model()->index(row, col);
                tableView->model()->setData(idx, columns[j], Qt::EditRole);
            }
        }
    }
    
    statusBar()->showMessage(tr("Data pasted from clipboard"), 3000);
}

void MainWindow::exportToCSV()
{
    QTableView* tableView = getCurrentTableView();
    if (!tableView)
        return;

    // Получаем выделенные ячейки
    QModelIndexList selectedIndexes = tableView->selectionModel()->selectedIndexes();
    if (selectedIndexes.isEmpty()) {
        // Если ничего не выделено, экспортируем всю таблицу
        selectedIndexes.clear();
        for (int row = 0; row < tableView->model()->rowCount(); ++row) {
            for (int col = 0; col < tableView->model()->columnCount(); ++col) {
                selectedIndexes.append(tableView->model()->index(row, col));
            }
        }
    }

    if (selectedIndexes.isEmpty())
        return;

    // Запрашиваем имя файла для сохранения
    QString fileName = QFileDialog::getSaveFileName(this, 
                                                  tr("Export to CSV"), 
                                                  QDir::homePath(), 
                                                  tr("CSV Files (*.csv);;All Files (*)"));
    
    if (fileName.isEmpty())
        return;

    // Добавляем расширение .csv, если его нет
    if (!fileName.endsWith(".csv", Qt::CaseInsensitive)) {
        fileName += ".csv";
    }

    // Находим минимальные и максимальные строки и столбцы
    int minRow = INT_MAX, maxRow = 0, minCol = INT_MAX, maxCol = 0;
    for (const QModelIndex &index : selectedIndexes) {
        minRow = std::min(minRow, index.row());
        maxRow = std::max(maxRow, index.row());
        minCol = std::min(minCol, index.column());
        maxCol = std::max(maxCol, index.column());
    }

    // Открываем файл для записи
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox msgBox(this);
        msgBox.setWindowTitle(tr("Error"));
        msgBox.setText(tr("Cannot open file for writing"));
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.addButton(QCoreApplication::translate("QPlatformTheme", "OK"), QMessageBox::AcceptRole);
        msgBox.exec();
        return;
    }

    QTextStream stream(&file);
    
    // Записываем заголовки столбцов
    QStringList headers;
    for (int col = minCol; col <= maxCol; ++col) {
        headers << tableView->model()->headerData(col, Qt::Horizontal).toString();
    }
    stream << headers.join(",") << "\n";

    // Записываем данные
    for (int row = minRow; row <= maxRow; ++row) {
        QStringList rowData;
        for (int col = minCol; col <= maxCol; ++col) {
            QModelIndex index = tableView->model()->index(row, col);
            if (selectedIndexes.contains(index)) {
                QString data = tableView->model()->data(index).toString();
                // Экранируем кавычки и добавляем кавычки если нужно
                if (data.contains(',') || data.contains('"') || data.contains('\n')) {
                    data.replace("\"", "\"\"");
                    data = "\"" + data + "\"";
                }
                rowData.append(data);
            } else {
                rowData.append("");
            }
        }
        stream << rowData.join(",") << "\n";
    }

    file.close();
    statusBar()->showMessage(tr("Data exported to %1").arg(fileName), 3000);
}

void MainWindow::importFromCSV()
{
    EquipmentModel* model = getCurrentModel();
    if (!model)
        return;

    // Запрашиваем имя файла для импорта
    QString fileName = QFileDialog::getOpenFileName(this, 
                                                  tr("Import from CSV"), 
                                                  QDir::homePath(), 
                                                  tr("CSV Files (*.csv)"));
    
    if (fileName.isEmpty())
        return;

    // Открываем файл для чтения
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this, tr("Error"), tr("Cannot open file for reading"));
        return;
    }

    QTextStream stream(&file);
    
    // Читаем заголовок, чтобы определить колонки
    QString headerLine = stream.readLine();
    QStringList headers = headerLine.split(',');

    // Создаем карту соответствия заголовков и индексов колонок
    QMap<int, int> columnMap;
    for (int i = 0; i < headers.size(); ++i) {
        QString header = headers[i];
        for (int j = 0; j < model->columnCount(); ++j) {
            if (model->headerData(j, Qt::Horizontal).toString() == header) {
                columnMap[i] = j;
                break;
            }
        }
    }

    // Если не удалось найти соответствия для заголовков, предупреждаем
    if (columnMap.isEmpty()) {
        QMessageBox msgBox(this);
        msgBox.setWindowTitle(tr("Warning"));
        msgBox.setText(tr("Could not match CSV headers with table columns"));
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.addButton(QCoreApplication::translate("QPlatformTheme", "OK"), QMessageBox::AcceptRole);
        msgBox.exec();
        file.close();
        return;
    }

    // Читаем данные из CSV и добавляем их в модель
    int imported = 0;
    while (!stream.atEnd()) {
        QString line = stream.readLine();
        if (line.trimmed().isEmpty())
            continue;

        // Парсим CSV с учетом кавычек
        QStringList fields;
        bool inQuote = false;
        int quoteStart = -1;
        QString field;
        
        for (int i = 0; i < line.size(); ++i) {
            const QChar c = line[i];
            
            if (c == '"') {
                if (inQuote) {
                    // Проверяем двойные кавычки
                    if (i + 1 < line.size() && line[i + 1] == '"') {
                        field += '"';
                        i++; // Пропускаем следующую кавычку
                    } else {
                        inQuote = false;
                    }
                } else {
                    inQuote = true;
                    quoteStart = i;
                }
            } else if (c == ',' && !inQuote) {
                fields.append(field);
                field.clear();
            } else {
                field += c;
            }
        }
        
        if (!field.isEmpty()) {
            fields.append(field);
        }

        // Создаем новую запись и заполняем данными из CSV
        EquipmentRecord record;
        for (int i = 0; i < fields.size() && i < headers.size(); ++i) {
            if (columnMap.contains(i)) {
                int modelColumn = columnMap[i];
                QString value = fields[i];
                
                // Установка значений в запись в зависимости от колонки
                switch (modelColumn) {
                    case 0: record.id = value.trimmed(); break;
                    case 1: record.type = value.trimmed(); break;
                    case 2: record.model = value.trimmed(); break;
                    case 3: record.serial = value.trimmed(); break;
                    case 4: record.purchase_date = QDate::fromString(value.trimmed(), Qt::ISODate); break;
                    case 5: record.price = value.trimmed().toDouble(); break;
                    case 6: record.location = value.trimmed(); break;
                    case 7: record.status = value.trimmed(); break;
                    case 8: record.notes = value.trimmed(); break;
                    default: break;
                }
            }
        }

        // Проверяем валидность записи и добавляем ее в модель
        if (record.isValid() && !model->hasEquipmentId(record.id)) {
            model->addRecord(record);
            imported++;
        }
    }

    file.close();
    statusBar()->showMessage(tr("Imported %1 records from %2").arg(imported).arg(fileName), 3000);
}

QTableView* MainWindow::getCurrentTableView() const {
    TabData *tab = getCurrentTab();
    if (tab) {
        return tab->tableView;
    }
    return nullptr;
}

EquipmentModel* MainWindow::getCurrentModel() const {
    TabData *tab = getCurrentTab();
    if (tab) {
        return tab->model;
    }
    return nullptr;
}
