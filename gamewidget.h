#ifndef GAMEWIDGET_H
#define GAMEWIDGET_H

#include <QWidget>
#include <QList>
#include <QString>
#include <QTimer>
#include <QElapsedTimer>

// Режимы игры
enum GameMode {
    TIMED,      // Игра с ограничением по времени
    SURVIVAL,   // Игра с ограничением по жизням
    INFINITE    // Бесконечная игра
};

// Структура для хранения информации о движущемся слове
struct MovingWord {
    QString text;   // Текст слова
    qreal x;        // Координата X
    qreal y;        // Координата Y
    int length;     // Длина слова (используется для подсчёта очков)
    bool active;    // Активность слова (true — слово на экране)
};

class GameWidget : public QWidget
{
    Q_OBJECT

public:
    // Конструктор
    explicit GameWidget(QWidget *parent = nullptr);

    // Запуск игры
    void startGame(int durationSeconds, const QStringList &wordList, const QString &lang,
                   double wordSpeed = 3.5, int fieldWidth = 800, int fieldHeight = 400,
                   GameMode mode = TIMED, int lives = 3);

    // Остановка игры
    void stopGame();

    // Проверка введённого слова и удаление, если совпадает
    bool tryRemoveWord(const QString &input);

    // Установка скорости движения слов
    void setWordSpeed(double speed);

signals:
    // Сигналы для обновления интерфейса
    void scoreChanged(int newScore);      // Изменение счёта
    void gameFinished(int finalScore);    // Игра завершена
    void timeLeftChanged(int secondsLeft); // Оставшееся время
    void livesChanged(int livesLeft);     // Оставшиеся жизни

protected:
    // Отрисовка виджета
    void paintEvent(QPaintEvent *event) override;
    // Обработка таймеров
    void timerEvent(QTimerEvent *event) override;

private:
    // Спавн нового слова
    void spawnWord();
    // Обновление позиций всех слов
    void updatePositions();
    // Завершение игры
    void endGame();
    // Очистка списка слов
    void clearWords();
    // Уменьшение количества жизней
    void loseLife();

    QList<MovingWord> m_words;            // Список активных слов
    int m_spawnTimerId;                   // ID таймера спавна
    int m_moveTimerId;                    // ID таймера движения
    QElapsedTimer m_gameTimer;            // Таймер для измерения времени игры
    int m_gameDuration;                   // Длительность игры (в секундах)
    int m_score;                          // Текущий счёт игрока
    QStringList m_dictionary;             // Список слов для игры
    QString m_currentLanguage;            // Текущий язык
    int m_fieldWidth;                     // Ширина игрового поля
    int m_fieldHeight;                    // Высота игрового поля
    double m_wordSpeed;                   // Скорость движения слов
    QTimer *m_countdownTimer;             // Таймер обратного отсчёта (для режима TIMED)
    GameMode m_gameMode;                  // Режим игры
    int m_lives;                          // Количество жизней (для режима SURVIVAL)
};

#endif // GAMEWIDGET_H
