#include "gamewidget.h"
#include <QPainter>
#include <QRandomGenerator>
#include <QFontMetrics>
#include <QDebug>
#include <algorithm>

// Структура для хранения информации о движущемся слове
struct MovingWord {
    QString text;      // Текст слова
    int length;        // Длина слова
    double x, y;       // Координаты слова
    bool active;       // Активность слова
};

// Конструктор виджета игры
GameWidget::GameWidget(QWidget *parent) : QWidget(parent)
{
    setMinimumSize(800, 400);               // Устанавливаем минимальные размеры виджета
    setBackgroundRole(QPalette::Base);       // Устанавливаем цвет фона
    setAutoFillBackground(true);             // Включаем автоматическое заполнение фона

    // Инициализация всех переменных
    m_spawnTimerId = 0;                     // Инициализация идентификатора таймера спавна слов
    m_moveTimerId = 0;                      // Инициализация идентификатора таймера движения слов
    m_score = 0;                            // Начальный счёт
    m_gameDuration = 60;                    // Длительность игры по умолчанию (в секундах)
    m_wordSpeed = 3.5;                      // Скорость движения слов по умолчанию
    m_countdownTimer = nullptr;             // Таймер обратного отсчёта
    m_fieldWidth = 800;                      // Ширина игрового поля
    m_fieldHeight = 400;                     // Высота игрового поля
    m_gameMode = TIMED;                      // Режим игры по умолчанию
    m_lives = 3;                            // Количество жизней по умолчанию
    m_timeAttackCounter = 0;                // Счётчик для режима Time Attack
    m_timeAttackSpeedMultiplier = 1.0;       // Множитель скорости для режима Time Attack
    m_totalMissed = 0;                      // Общее количество пропущенных слов

    // Отладочное сообщение при инициализации
    qDebug() << "GameWidget initialized. Default speed:" << m_wordSpeed << "px/frame";
}

// Запуск игры
void GameWidget::startGame(int durationSeconds, const QStringList &wordList, const QString &lang, double wordSpeed, int fieldWidth, int fieldHeight, GameMode mode, int lives)
{
    stopGame();                             // Останавливаем текущую игру
    m_missedWords.clear();                  // Очищаем статистику пропущенных слов
    m_totalMissed = 0;                      // Сбрасываем счётчик пропущенных слов

    m_dictionary = wordList;                // Устанавливаем словарь для игры
    m_currentLanguage = lang;               // Устанавливаем текущий язык
    m_gameDuration = durationSeconds;       // Устанавливаем длительность игры
    m_wordSpeed = wordSpeed;                // Устанавливаем скорость движения слов
    m_score = 0;                            // Сбрасываем счёт
    m_words.clear();                        // Очищаем список активных слов
    m_fieldWidth = fieldWidth;              // Устанавливаем ширину поля
    m_fieldHeight = fieldHeight;            // Устанавливаем высоту поля
    m_gameMode = mode;                      // Устанавливаем режим игры
    m_lives = lives;                        // Устанавливаем количество жизней
    m_timeAttackCounter = 0;                // Сбрасываем счётчик режима Time Attack
    m_timeAttackSpeedMultiplier = 1.0;       // Сбрасываем множитель скорости

    // Оповещаем об изменении счёта, жизней и времени
    emit scoreChanged(0);
    emit livesChanged(m_lives);
    emit timeLeftChanged(m_gameDuration);
    emit statisticsUpdated();

    // Настройка таймеров в зависимости от режима игры
    if (m_gameMode == TIMED || m_gameMode == TIME_ATTACK) {
        m_spawnTimerId = startTimer(1800);  // Таймер для спавна слов (1.8 секунды)
        m_moveTimerId = startTimer(33);     // Таймер для движения слов (~30 FPS)

        m_gameTimer.start();                // Запускаем таймер игры

        // Таймер обратного отсчёта
        m_countdownTimer = new QTimer(this);
        connect(m_countdownTimer, &QTimer::timeout, [this]() {
            int elapsed = m_gameTimer.elapsed() / 1000;  // Прошедшее время в секундах
            int left = m_gameDuration - elapsed;         // Оставшееся время

            emit timeLeftChanged(left);                 // Оповещаем об изменении времени

            if (left <= 0) {                            // Если время вышло
                m_countdownTimer->stop();
                endGame();
            }

            if (m_gameMode == TIME_ATTACK) {           // Для режима Time Attack
                m_timeAttackCounter++;                  // Увеличиваем счётчик
                if (m_timeAttackCounter % 5 == 0) {     // Каждые 5 слов
                    m_timeAttackSpeedMultiplier += 0.2; // Увеличиваем скорость
                    qDebug() << "Time Attack speed increased to" << m_timeAttackSpeedMultiplier;
                }
            }
        });
        m_countdownTimer->start(1000);                // Запуск таймера каждую секунду
        qDebug() << "Timed/Time Attack mode started. Duration:" << m_gameDuration << "sec";
    } else if (m_gameMode == SURVIVAL || m_gameMode == INFINITE) {
        m_spawnTimerId = startTimer(1800);  // Таймер для спавна слов
        m_moveTimerId = startTimer(33);     // Таймер для движения слов
        qDebug() << "Survival/Infinite mode started. Lives:" << m_lives;
    }
}

// Остановка игры
void GameWidget::stopGame()
{
    // Останавливаем и удаляем таймеры
    if (m_spawnTimerId) {
        killTimer(m_spawnTimerId);
        m_spawnTimerId = 0;
    }
    if (m_moveTimerId) {
        killTimer(m_moveTimerId);
        m_moveTimerId = 0;
    }
    if (m_countdownTimer) {
        m_countdownTimer->stop();
        m_countdownTimer->deleteLater();
        m_countdownTimer = nullptr;
    }
    clearWords();  // Очищаем все слова

    // Отладочное сообщение при остановке игры
    qDebug() << "Game stopped. Final score:" << m_score << "Missed words:" << m_totalMissed;
}

// Установка скорости слов
void GameWidget::setWordSpeed(double speed)
{
    m_wordSpeed = qBound(1.0, speed, 15.0);  // Ограничиваем скорость в пределах 1.0 - 15.0
    qDebug() << "Word speed set to" << m_wordSpeed << "px/frame";
}

// Попытка удаления слова
bool GameWidget::tryRemoveWord(const QString &input)
{
    QString trimmed = input.trimmed();
    if (trimmed.isEmpty()) {
        qWarning() << "Empty input word!";
        return false;
    }

    // Поиск слова в списке активных слов
    for (int i = 0; i < m_words.size(); ++i) {
        if (m_words[i].active && m_words[i].text == trimmed) {
            int points = m_words[i].length;  // Очки равны длине слова
            m_score += points;
            emit scoreChanged(m_score);      // Оповещаем об изменении счёта
            qDebug() << "Word removed:" << m_words[i].text << "Points:" << points;
            m_words.removeAt(i);             // Удаляем слово
            return true;
        }
    }

    // Если слово не найдено
    qWarning() << "Word not found:" << trimmed;
    if (m_gameMode == SURVIVAL) {
        loseLife();  // Уменьшаем количество жизней
    }
    return false;
}

// Уменьшение количества жизней
void GameWidget::loseLife()
{
    m_lives--;
    emit livesChanged(m_lives);  // Оповещаем об изменении количества жизней
    qDebug() << "Life lost. Remaining lives:" << m_lives;
    if (m_lives <= 0) {
        qDebug() << "No lives left. Game over!";
        endGame();  // Если жизней не осталось, заканчиваем игру
    }
}

// Очистка списка слов
void GameWidget::clearWords()
{
    m_words.clear();  // Очищаем список активных слов
    update();         // Обновляем виджет
    qDebug() << "All words cleared.";
}

// Спавн нового слова
void GameWidget::spawnWord()
{
    if (m_dictionary.isEmpty()) {
        qWarning() << "Dictionary is empty! Cannot spawn word.";
        return;  // Если словарь пустой, не спавним новое слово
    }

    // Выбираем случайное слово из словаря
    int randomIndex = QRandomGenerator::global()->bounded(m_dictionary.size());
    QString word = m_dictionary[randomIndex];
    if (word.isEmpty()) {
        qWarning() << "Random word is empty!";
        return;
    }

    // Создаём новое движущееся слово
    MovingWord newWord;
    newWord.text = word;
    newWord.length = word.length();
    newWord.x = 0.0;  // Начинаем с левого края
    newWord.y = QRandomGenerator::global()->bounded(50, m_fieldHeight - 50);  // Случайная Y-координата
    newWord.active = true;
    m_words.append(newWord);  // Добавляем слово в список

    qDebug() << "Word spawned:" << word << "at pos (" << newWord.x << "," << newWord.y << ")";
}

// Обновление позиций слов
void GameWidget::updatePositions()
{
    bool anyMoved = false;
    for (int i = 0; i < m_words.size(); ++i) {
        if (!m_words[i].active) continue;

        // Обновляем позицию слова
        m_words[i].x += m_wordSpeed * m_timeAttackSpeedMultiplier;

        // Если слово вышло за пределы поля
        if (m_words[i].x > m_fieldWidth + 100) {
            if (m_gameMode == SURVIVAL) {
                loseLife();  // В режиме Survival уменьшаем жизни
            }
            addMissedWord(m_words[i].text);  // Добавляем слово в статистику пропущенных
            qDebug() << "Word missed:" << m_words[i].text;
            m_words.removeAt(i);  // Удаляем слово
            i--;  // Корректируем индекс
            anyMoved = true;
        } else {
            anyMoved = true;
        }
    }

    if (anyMoved) update();  // Обновляем виджет, если были изменения
}

// Отрисовка виджета
void GameWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);  // Включаем сглаживание

    painter.fillRect(rect(), QColor(240, 240, 245));  // Заливаем фон

    QFont font("Arial", 18, QFont::Bold);  // Настраиваем шрифт
    painter.setFont(font);

    // Рисуем каждое активное слово
    for (const MovingWord &word : m_words) {
        if (!word.active) continue;

        painter.setPen(QPen(QColor(50, 50, 180), 2));  // Цвет обводки
        painter.setBrush(QColor(100, 100, 220, 200)); // Цвет фона

        // Вычисляем прямоугольник для слова
        QRectF textRect(word.x, word.y - 18,
                      QFontMetrics(font).horizontalAdvance(word.text) + 20, 40);
        painter.drawRoundedRect(textRect, 15, 15);  // Рисуем закруглённый прямоугольник

        painter.setPen(Qt::white);  // Белый цвет текста
        painter.drawText(textRect, Qt::AlignCenter, word.text);  // Рисуем текст
    }

    // Рисуем линию на правой границе поля
    painter.setPen(QPen(Qt::red, 2, Qt::DashLine));
    painter.drawLine(m_fieldWidth - 10, 0, m_fieldWidth - 10, m_fieldHeight);

    qDebug() << "Paint event triggered. Active words:" << m_words.size();
}

// Обработка событий таймера
void GameWidget::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_spawnTimerId) {
        spawnWord();  // Спавним слово при срабатывании таймера спавна
    } else if (event->timerId() == m_moveTimerId) {
        updatePositions();  // Обновляем позиции слов при срабатывании таймера движения
    } else {
        QWidget::timerEvent(event);  // Обработка других таймеров
    }
}

// Завершение игры
void GameWidget::endGame()
{
    stopGame();  // Останавливаем игру
    emit gameFinished(m_score);  // Оповещаем об окончании игры
    emit statisticsUpdated();    // Оповещаем об обновлении статистики
    qDebug() << "Game ended. Final score:" << m_score;
}

// Добавление пропущенного слова в статистику
void GameWidget::addMissedWord(const QString &word)
{
    m_missedWords[word]++;  // Увеличиваем счётчик пропущенного слова
    m_totalMissed++;        // Увеличиваем общее количество пропущенных слов
    emit statisticsUpdated();  // Оповещаем об обновлении статистики
    qDebug() << "Missed word added to stats:" << word;
}

// Получение статистики пропущенных слов
QMap<QString, int> GameWidget::getMissedWords() const
{
    qDebug() << "Missed words requested. Total unique words:" << m_missedWords.size();
    return m_missedWords;  // Возвращаем карту пропущенных слов
}

// Получение общего количества пропущенных слов
int GameWidget::getTotalMissed() const
{
    qDebug() << "Total missed words requested:" << m_totalMissed;
    return m_totalMissed;  // Возвращаем общее количество пропущенных слов
}

// Получение топ пропущенных слов
QStringList GameWidget::getTopMissedWords(int count) const
{
    // Создаём список пар (слово, количество)
    QList<QPair<QString, int>> list;
    for (auto it = m_missedWords.constBegin(); it != m_missedWords.constEnd(); ++it) {
        list.append(qMakePair(it.key(), it.value()));
    }

    // Сортируем по количеству в убывающем порядке
    std::sort(list.begin(), list.end(), [](const QPair<QString, int> &a, const QPair<QString, int> &b) {
        return a.second > b.second;
    });

    // Формируем список топ слов
    QStringList topWords;
    for (int i = 0; i < qMin(count, list.size()); ++i) {
        topWords << QString("%1 (%2)").arg(list[i].first).arg(list[i].second);
    }

    qDebug() << "Top missed words requested. Top count:" << topWords.size();
    return topWords;
}
