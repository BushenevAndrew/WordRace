#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QMainWindow>
#include <QSettings>

class GameWidget;
class QLineEdit;
class QLabel;
class QPushButton;
class QSpinBox;
class QComboBox;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    // Конструктор
    MainWindow(QWidget *parent = nullptr);
    // Деструктор
    ~MainWindow();

private slots:
    // Обработчики событий
    void onStartGame();          // Запуск игры
    void onStopGame();           // Остановка игры
    void onGameFinished(int score); // Обработка завершения игры
    void onOpenSettings();       // Открытие окна настроек
    void onOpenWordEditor();     // Открытие редактора слов

private:
    // Настройка интерфейса
    void setupUI();
    // Загрузка настроек
    void loadSettings();
    // Сохранение настроек
    void saveSettings();
    // Обновление отображения счёта
    void updateScoreDisplay(int score);
    // Обновление отображения времени
    void updateTimeDisplay(int seconds);
    // Обновление отображения жизней
    void updateLivesDisplay(int lives);

    GameWidget *m_gameWidget;     // Виджет игры
    QSettings m_settings;         // Настройки приложения
    bool m_gameActive;            // Флаг активности игры
    QStringList m_currentWordList;// Текущий список слов
    int m_fieldWidth;             // Ширина игрового поля
    int m_fieldHeight;            // Высота игрового поля

    // Виджеты интерфейса
    QLabel *m_scoreLabel;         // Метка счёта
    QLabel *m_timeLabel;          // Метка времени
    QLabel *m_livesLabel;         // Метка жизней
    QLineEdit *m_inputEdit;       // Поле для ввода слов
    QPushButton *m_startButton;   // Кнопка "Старт"
    QPushButton *m_stopButton;    // Кнопка "Стоп"
    QPushButton *m_settingsButton;// Кнопка "Настройки"
    QPushButton *m_editWordsButton;// Кнопка "Редактор слов"
    QSpinBox *m_widthSpin;        // Поле для выбора ширины поля
    QSpinBox *m_heightSpin;       // Поле для выбора высоты поля
};

#endif // MAINWIDGET_H
