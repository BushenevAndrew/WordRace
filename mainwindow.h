#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QMainWindow>
#include <QSettings>

class GameWidget;    // Виджет игрового поля
class QLineEdit;     // Поле для ввода текста
class QLabel;        // Метка для отображения текста
class QPushButton;   // Кнопка
class QSpinBox;      // Поле для ввода целых чисел
class QComboBox;     // Выпадающий список

/**
 * @brief Главное окно приложения "Word Race Game".
 *
 * Управляет:
 * • Интерфейсом пользователя
 * • Запуском и остановкой игры
 * • Настройками игры
 * • Редактором слов
 * • Отображением статистики
 * • Взаимодействием с игровым виджетом
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT  // Макрос для поддержки сигналов и слотов

public:
    /**
     * @brief Конструктор главного окна.
     *
     * @param parent Родительский виджет.
     */
    MainWindow(QWidget *parent = nullptr);

    /**
     * @brief Деструктор главного окна.
     *
     * Сохраняет настройки окна при закрытии.
     */
    ~MainWindow();

private slots:
    /**
     * @brief Слот для запуска игры.
     *
     * Загружает настройки, проверяет наличие слов,
     * инициализирует игровой виджет и запускает игру.
     */
    void onStartGame();

    /**
     * @brief Слот для остановки игры.
     *
     * Останавливает игру, сбрасывает интерфейс,
     * очищает игровое поле.
     */
    void onStopGame();

    /**
     * @brief Слот для обработки завершения игры.
     *
     * Отображает окно с результатами,
     * обновляет интерфейс.
     *
     * @param score Итоговый счёт.
     */
    void onGameFinished(int score);

    /**
     * @brief Слот для открытия окна настроек игры.
     *
     * Позволяет пользователю изменить параметры игры:
     * • Длительность
     * • Скорость слов
     * • Язык
     * • Режим игры
     * • Размер поля
     * • Файл со словами
     */
    void onOpenSettings();

    /**
     * @brief Слот для открытия редактора слов.
     *
     * Позволяет пользователю редактировать,
     * загружать, сохранять и сбрасывать список слов.
     */
    void onOpenWordEditor();

    /**
     * @brief Слот для открытия окна статистики.
     *
     * Отображает статистику игры:
     * • Общее количество пропущенных слов
     * • Топ самых пропущенных слов
     */
    void onOpenStatistics();

    /**
     * @brief Слот для обновления отображения счёта.
     *
     * @param score Текущий счёт.
     */
    void updateScoreDisplay(int score);

    /**
     * @brief Слот для обновления отображения времени.
     *
     * @param seconds Оставшееся время.
     */
    void updateTimeDisplay(int seconds);

    /**
     * @brief Слот для обновления отображения жизней.
     *
     * @param lives Оставшиеся жизни.
     */
    void updateLivesDisplay(int lives);

    /**
     * @brief Слот для обновления статистики.
     *
     * Может использоваться для обновления меток статистики
     * в интерфейсе, если необходимо.
     */
    void updateStatisticsDisplay();
private:
    /**
     * @brief Настройка интерфейса главного окна.
     *
     * Создаёт и компонует все виджеты,
     * подключает сигналы и слоты.
     */
    void setupUI();

    /**
     * @brief Загрузка настроек приложения из файла.
     *
     * Если настройки не существуют, устанавливает значения по умолчанию.
     */
    void loadSettings();

    /**
     * @brief Сохранение настроек приложения в файл.
     */
    void saveSettings();

    // Основные виджеты
    GameWidget *m_gameWidget;          // Виджет игрового поля
    QSettings m_settings;              // Объект для работы с настройками
    bool m_gameActive;                 // Флаг активности игры

    // Текущие настройки игры
    QStringList m_currentWordList;     // Текущий список слов
    int m_fieldWidth;                  // Ширина игрового поля
    int m_fieldHeight;                 // Высота игрового поля

    // Виджеты интерфейса
    QLabel *m_scoreLabel;              // Метка для отображения счёта
    QLabel *m_timeLabel;               // Метка для отображения времени
    QLabel *m_livesLabel;              // Метка для отображения жизней
    QLineEdit *m_inputEdit;            // Поле для ввода слов
    QPushButton *m_startButton;        // Кнопка "Старт"
    QPushButton *m_stopButton;         // Кнопка "Стоп" (скрыта по умолчанию)
    QPushButton *m_settingsButton;     // Кнопка "Настройки"
    QPushButton *m_editWordsButton;    // Кнопка "Редактор слов"
    QPushButton *m_statisticsButton;   // Кнопка "Статистика"

    // Виджеты настроек
    QSpinBox *m_widthSpin;             // Настройка ширины поля
    QSpinBox *m_heightSpin;            // Настройка высоты поля
};

#endif // MAINWIDGET_H
