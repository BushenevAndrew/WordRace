#ifndef GAMEWIDGET_H
#define GAMEWIDGET_H

#include <QWidget>
#include <QList>
#include <QString>
#include <QTimer>
#include <QElapsedTimer>
#include <QMap>

// Режимы игры
enum GameMode {
    TIMED,          // Игра с ограничением по времени
    SURVIVAL,       // Игра с ограничением по жизням
    INFINITE,       // Бесконечная игра
    TIME_ATTACK     // Режим "Time Attack" с увеличивающейся скоростью
};

/**
 * @brief Структура для хранения информации о движущемся слове.
 *
 * Используется для отрисовки и обработки слов на игровом поле.
 */
struct MovingWord {
    QString text;   // Текст слова
    qreal x;        // Координата X
    qreal y;        // Координата Y
    int length;     // Длина слова (для подсчёта очков)
    bool active;    // Флаг активности
};

class GameWidget : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief Конструктор игрового виджета.
     *
     * @param parent Родительский виджет.
     */
    explicit GameWidget(QWidget *parent = nullptr);

    /**
     * @brief Запуск игры с заданными параметрами.
     *
     * @param durationSeconds Длительность игры (в секундах).
     * @param wordList Список слов для игры.
     * @param lang Текущий язык.
     * @param wordSpeed Скорость движения слов (по умолчанию: 3.5).
     * @param fieldWidth Ширина игрового поля (по умолчанию: 800).
     * @param fieldHeight Высота игрового поля (по умолчанию: 400).
     * @param mode Режим игры (по умолчанию: TIMED).
     * @param lives Количество жизней (для режима SURVIVAL, по умолчанию: 3).
     */
    void startGame(
        int durationSeconds,
        const QStringList &wordList,
        const QString &lang,
        double wordSpeed = 3.5,
        int fieldWidth = 800,
        int fieldHeight = 400,
        GameMode mode = TIMED,
        int lives = 3
    );

    /**
     * @brief Остановка игры.
     *
     * Останавливает все таймеры, очищает игровое поле,
     * сбрасывает счётчики и статистику.
     */
    void stopGame();

    /**
     * @brief Проверка ввода пользователя.
     *
     * @param input Введённое пользователем слово.
     * @return true, если слово найдено и удалено.
     * @return false, если слово не найдено или игра не активна.
     */
    bool tryRemoveWord(const QString &input);

    /**
     * @brief Установка скорости движения слов.
     *
     * Скорость ограничена в пределах 1.0–15.0.
     *
     * @param speed Новая скорость.
     */
    void setWordSpeed(double speed);

    // Методы для получения статистики
    QMap<QString, int> getMissedWords() const;
    int getTotalMissed() const;
    QStringList getTopMissedWords(int count = 5) const;

signals:
    /**
     * @brief Сигнал об изменении счёта.
     *
     * @param newScore Новый счёт.
     */
    void scoreChanged(int newScore);

    /**
     * @brief Сигнал о завершении игры.
     *
     * @param finalScore Итоговый счёт.
     */
    void gameFinished(int finalScore);

    /**
     * @brief Сигнал об изменении оставшегося времени.
     *
     * @param secondsLeft Оставшееся время (в секундах).
     */
    void timeLeftChanged(int secondsLeft);

    /**
     * @brief Сигнал об изменении количества жизней.
     *
     * @param livesLeft Оставшиеся жизни.
     */
    void livesChanged(int livesLeft);

    /**
     * @brief Сигнал для обновления статистики.
     *
     * Излучается при изменении статистики пропущенных слов.
     */
    void statisticsUpdated();

protected:
    /**
     * @brief Обработчик события отрисовки.
     *
     * Используется для отрисовки игрового поля и слов.
     */
    void paintEvent(QPaintEvent *event) override;

    /**
     * @brief Обработчик таймерных событий.
     *
     * Используется для обработки таймеров спавна и движения слов.
     */
    void timerEvent(QTimerEvent *event) override;

private slots:
    /**
     * @brief Спавн нового слова на поле.
     */
    void spawnWord();

    /**
     * @brief Обновление позиций всех активных слов.
     */
    void updatePositions();

    /**
     * @brief Завершение игры.
     */
    void endGame();

    /**
     * @brief Очистка игрового поля.
     */
    void clearWords();

    /**
     * @brief Уменьшение количества жизней.
     */
    void loseLife();

    /**
     * @brief Добавление слова в статистику пропусков.
     *
     * @param word Пропущенное слово.
     */
    void addMissedWord(const QString &word);

private:
    // Список активных слов
    QList<MovingWord> m_words;

    // Идентификаторы таймеров
    int m_spawnTimerId;
    int m_moveTimerId;

    // Таймер игры для режимов с ограничением по времени
    QElapsedTimer m_gameTimer;

    // Таймер обратного отсчёта для режимов с ограничением по времени
    QTimer *m_countdownTimer;

    // Настройки игры
    int m_gameDuration;
    int m_score;
    int m_fieldWidth;
    int m_fieldHeight;
    double m_wordSpeed;
    double m_timeAttackSpeedMultiplier;

    // Настройки режима игры
    GameMode m_gameMode;
    int m_lives;
    int m_timeAttackCounter;

    // Текущие настройки
    QStringList m_dictionary;
    QString m_currentLanguage;

    // Статистика пропущенных слов
    QMap<QString, int> m_missedWords;
    int m_totalMissed;
};
#endif // GAMEWIDGET_H
