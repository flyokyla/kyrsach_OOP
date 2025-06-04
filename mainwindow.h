/**
 * @file mainwindow.h
 * @brief Главное окно приложения для управления офисным оборудованием
 * @author Капытин Андрей Игоревич ИЦТМС 2-1
 * @date 2025
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTranslator>
#include <QSortFilterProxyModel>
#include <QTabWidget>
#include <QTableView>
#include <QLineEdit>
#include <QComboBox>
#include <QSet>
#include <QLabel>
#include "equipmentmodel.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

/**
 * @brief Структура для хранения данных каждой вкладки
 * 
 * Содержит модель данных, прокси-модель для фильтрации и путь к файлу
 */
struct TabData {
    EquipmentModel *model;          ///< Модель данных оборудования
    QSortFilterProxyModel *proxyModel;   ///< Прокси-модель для фильтрации и сортировки
    QString fileName;               ///< Путь к файлу данных
    QTableView *tableView;
    QLineEdit *searchEdit;
    bool isModified;
    int selectedColumn;             ///< Выбранный столбец для поиска (-1 = поиск по ID)
    QComboBox *columnFilterCombo;   ///< Комбобокс для фильтрации по столбцам
    QWidget *filterWidget;          ///< Виджет с фильтрами
};

/**
 * @class MainWindow
 * @brief Главное окно приложения
 * 
 * Реализует основной интерфейс программы с поддержкой:
 * - Работы с несколькими файлами через вкладки
 * - Многоязычности (русский, английский, испанский)
 * - Поиска и фильтрации данных
 * - Печати и экспорта данных
 * - Контекстного меню
 * - Сохранения настроек
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /**
     * @brief Конструктор главного окна
     * @param parent Родительский виджет
     */
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    /**
     * @brief Обработчик события закрытия окна
     * @param event Событие закрытия
     */
    void closeEvent(QCloseEvent *event) override;
    
    /**
     * @brief Обработчик событий изменения состояния (смена языка)
     * @param event Событие изменения
     */
    void changeEvent(QEvent *event) override;

    /**
     * @brief Обработчик нажатия клавиш
     * @param event Событие клавиатуры
     */
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    /**
     * @brief Создает новый файл с пустой базой данных
     */
    void newDocument();
    /**
     * @brief Открывает существующий файл базы данных
     */
    void openFile();
    /**
     * @brief Сохраняет текущий файл
     */
    void saveFile();
    /**
     * @brief Сохраняет файл под новым именем
     */
    void saveAs();
    /**
     * @brief Добавляет новую запись оборудования
     */
    void addRecord();
    /**
     * @brief Редактирует выбранную запись оборудования
     */
    void editRecord();
    /**
     * @brief Удаляет выбранную запись оборудования
     */
    void removeRecord();
    /**
     * @brief Отображает окно с информацией о программе
     */
    void showAbout();
    /**
     * @brief Переключает язык интерфейса на русский
     */
    void switchToRussian();
    /**
     * @brief Переключает язык интерфейса на английский
     */
    void switchToEnglish();
    /**
     * @brief Переключает язык интерфейса на испанский
     */
    void switchToSpanish();
    /**
     * @brief Отображает окно с графиками и аналитикой
     */
    void showChart();
    /**
     * @brief Печатает содержимое текущей таблицы
     */
    void print();
    /**
     * @brief Фильтрует таблицу по введенному тексту
     * @param text Текст для поиска
     */
    void filterTable(const QString &text);
    /**
     * @brief Обработчик смены активной вкладки
     * @param index Индекс новой активной вкладки
     */
    void onTabChanged(int index);
    /**
     * @brief Закрывает текущую вкладку
     */
    void onTabCloseRequested(int index);
    /**
     * @brief Обработчик клика по заголовку столбца для выбора столбца поиска
     * @param logicalIndex Индекс столбца
     */
    void onHeaderClicked(int logicalIndex);
    /**
     * @brief Обработчик изменения фильтра по столбцу
     * @param filterValue Значение фильтра
     */
    void onColumnFilterChanged(const QString &filterValue);
    /**
     * @brief Фильтрует таблицу по выбранному столбцу
     * @param text Текст для поиска
     */
    void filterBySelectedColumn(const QString &text);
    
    /**
     * @brief Копирует выделенные ячейки в буфер обмена
     */
    void copySelectedCells();
    
    /**
     * @brief Вставляет данные из буфера обмена
     */
    void pasteFromClipboard();
    
    /**
     * @brief Экспортирует выделенные ячейки в CSV файл
     */
    void exportToCSV();
    
    /**
     * @brief Импортирует CSV данные из файла
     */
    void importFromCSV();

    /**
     * @brief Отображает контекстное меню таблицы
     * @param pos Позиция для отображения меню
     */
    void showContextMenu(const QPoint &pos);

private:
    Ui::MainWindow *ui;
    QTranslator m_translator;
    QString m_currentLanguage;
    QTabWidget *m_tabWidget;
    QVector<TabData> m_tabs;

    void createMenuBar();
    void setupConnections();
    void retranslateUi();
    void loadLanguage(const QString &lang);
    void loadFile(const QString &fileName);
    void saveState();
    void restoreState();
    void updateWindowTitle();
    void updateStatusBar();
    void setupColumnFilters(TabData &tabData);
    void updateColumnFilter(TabData &tabData, int column);
    
    /**
     * @brief Получает указатель на текущую вкладку
     * @return Указатель на TabData или nullptr
     */
    TabData* getCurrentTab() const;
    
    /**
     * @brief Получает указатель на текущую таблицу
     * @return Указатель на QTableView или nullptr
     */
    QTableView* getCurrentTableView() const;
    
    /**
     * @brief Получает указатель на текущую модель данных
     * @return Указатель на EquipmentModel или nullptr
     */
    EquipmentModel* getCurrentModel() const;

    int createNewTab(const QString &fileName = QString());
    bool closeTab(int index);
};
#endif // MAINWINDOW_H
