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
    TIMED,      // Игра с ограничением по времени
    SURVIVAL,   // Игра с ограничением по жизням
    INFINITE,   // Бесконечная игра
    TIME_ATTACK // Режим "Time Attack" с увеличивающейся скоростью
};

/**
 * @brief Структура для хранения информации о движущемся слове.
 *
 * Используется для отрисовки и обработки слов на игровом поле.
 */
struct MovingWord {
    QString text;   // Текст слова
    qreal x;        // Координата X (положение на поле)
    qreal y;        // Координата Y (положение на поле)
    int length;     // Длина слова (используется для подсчёта очков)
    bool active;    // Флаг активности слова (true — слово на экране)
};

class GameWidget : public QWidget
{
    Q_OBJECT  // Макрос для поддержки сигналов и слотов

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
     * @param durationSeconds Длительность игры в секундах.
     * @param wordList Список слов для игры.
     * @param lang Текущий язык (не используется напрямую, но может пригодиться для будущих расширений).
     * @param wordSpeed Скорость движения слов (в пикселях за кадр).
     * @param fieldWidth Ширина игрового поля.
     * @param fieldHeight Высота игрового поля.
     * @param mode Режим игры.
     * @param lives Количество жизней (для режима SURVIVAL).
     */
    void startGame(int durationSeconds, const QStringList &wordList, const QString &lang,
                   double wordSpeed = 3.5, int fieldWidth = 800, int fieldHeight = 400,
                   GameMode mode = TIMED, int lives = 3);

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
     * @return true Если слово найдено и удалено.
     * @return false Если слово не найдено или игра не активна.
     */
    bool tryRemoveWord(const QString &input);

    /**
     * @brief Установка скорости движения слов.
     *
     * @param speed Новая скорость (ограничена в пределах 1.0–15.0).
     */
    void setWordSpeed(double speed);

    /**
     * @brief Получение статистики по пропущенным словам.
     *
     * @return QMap<QString, int> Карта: слово → количество пропусков.
     */
    QMap<QString, int> getMissedWords() const;

    /**
     * @brief Получение общего количества пропущенных слов.
     *
     * @return int Общее количество пропусков.
     */
    int getTotalMissed() const;

    /**
     * @brief Получение топ N пропущенных слов.
     *
     * @param count Количество топ слов для отображения.
     * @return QStringList Список строк формата "слово (количество)".
     */
    QStringList getTopMissedWords(int count = 5) const;

signals:
    /**
     * @brief Сигнал об изменении счёта.
     *
     * Излучается при изменении счёта игрока.
     *
     * @param newScore Новый счёт.
     */
    void scoreChanged(int newScore);

    /**
     * @brief Сигнал о завершении игры.
     *
     * Излучается при завершении игры.
     *
     * @param finalScore Итоговый счёт.
     */
    void gameFinished(int finalScore);

    /**
     * @brief Сигнал об изменении оставшегося времени.
     *
     * Излучается каждую секунду в режимах TIMED и TIME_ATTACK.
     *
     * @param secondsLeft Оставшееся время в секундах.
     */
    void timeLeftChanged(int secondsLeft);

    /**
     * @brief Сигнал об изменении количества жизней.
     *
     * Излучается при изменении количества жизней.
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
     *
     * @param event Событие отрисовки.
     */
    void paintEvent(QPaintEvent *event) override;

    /**
     * @brief Обработчик таймерных событий.
     *
     * Используется для обработки таймеров спавна и движения слов.
     *
     * @param event Событие таймера.
     */
    void timerEvent(QTimerEvent *event) override;

private:
    /**
     * @brief Спавн нового слова на поле.
     *
     * Выбирает случайное слово из словаря,
     * задаёт ему случайную Y-координату,
     * добавляет в список активных слов.
     */
    void spawnWord();

    /**
     * @brief Обновление позиций всех активных слов.
     *
     * Сдвигает слова на заданную скорость,
     * проверяет выход за пределы поля,
     * удаляет слова, которые вышли за поле,
     * обновляет виджет.
     */
    void updatePositions();

    /**
     * @brief Завершение игры.
     *
     * Останавливает игру, излучает сигнал о завершении,
     * обновляет статистику.
     */
    void endGame();

    /**
     * @brief Очистка игрового поля.
     *
     * Удаляет все активные слова.
     */
    void clearWords();

    /**
     * @brief Уменьшение количества жизней.
     *
     * Уменьшает количество жизней на единицу,
     * излучает сигнал об изменении жизней,
     * завершает игру, если жизней не осталось.
     */
    void loseLife();

    /**
     * @brief Добавление слова в статистику пропусков.
     *
     * Увеличивает счётчик пропусков для данного слова,
     * обновляет общее количество пропусков,
     * излучает сигнал для обновления статистики.
     *
     * @param word Пропущенное слово.
     */
    void addMissedWord(const QString &word);

    // Список активных слов на экране
    QList<MovingWord> m_words;

    // Идентификаторы таймеров
    int m_spawnTimerId;    // Таймер для спавна новых слов
    int m_moveTimerId;      // Таймер для движения слов

    // Таймер игры для режимов с ограничением по времени
    QElapsedTimer m_gameTimer;

    // Настройки игры
    int m_gameDuration;     // Длительность игры в секундах
    int m_score;            // Текущий счёт игрока
    QStringList m_dictionary; // Словарь слов для игры
    QString m_currentLanguage; // Текущий язык (для будущих расширений)
    int m_fieldWidth;       // Ширина игрового поля
    int m_fieldHeight;      // Высота игрового поля
    double m_wordSpeed;     // Скорость движения слов
    double m_timeAttackSpeedMultiplier; // Множитель скорости для режима TIME_ATTACK

    // Таймер обратного отсчёта для режимов с ограничением по времени
    QTimer *m_countdownTimer;

    // Настройки режима игры
    GameMode m_gameMode;    // Текущий режим игры
    int m_lives;            // Количество жизней (для режима SURVIVAL)
    int m_timeAttackCounter; // Счётчик для режима TIME_ATTACK

    // Статистика пропущенных слов
    QMap<QString, int> m_missedWords; // Карта: слово → количество пропусков
    int m_totalMissed;                 // Общее количество пропусков
};

#endif // GAMEWIDGET_H
