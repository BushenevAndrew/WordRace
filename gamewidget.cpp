#include "gamewidget.h"
#include <QPainter>
#include <QRandomGenerator>
#include <QFontMetrics>
#include <QDebug>

GameWidget::GameWidget(QWidget *parent) : QWidget(parent)
{
    // Установка минимального размера виджета
    setMinimumSize(800, 400);
    // Установка фонового цвета
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);

    // Инициализация переменных
    m_spawnTimerId = 0;
    m_moveTimerId = 0;
    m_score = 0;
    m_gameDuration = 60;
    m_wordSpeed = 3.5;
    m_countdownTimer = nullptr;
    m_fieldWidth = 800;
    m_fieldHeight = 400;
    m_gameMode = TIMED;
    m_lives = 3;
}

void GameWidget::startGame(int durationSeconds, const QStringList &wordList, const QString &lang, double wordSpeed, int fieldWidth, int fieldHeight, GameMode mode, int lives)
{
    // Остановка предыдущей игры
    stopGame();

    // Установка параметров игры
    m_dictionary = wordList;
    m_currentLanguage = lang;
    m_gameDuration = durationSeconds;
    m_wordSpeed = wordSpeed;
    m_score = 0;
    m_words.clear();
    m_fieldWidth = fieldWidth;
    m_fieldHeight = fieldHeight;
    m_gameMode = mode;
    m_lives = lives;

    // Уведомление интерфейса об обновлении счёта
    emit scoreChanged(0);
    emit livesChanged(m_lives);

    // Настройка таймеров в зависимости от режима игры
    if (m_gameMode == TIMED) {
        // Таймер спавна слов (каждые 1.8 секунды)
        m_spawnTimerId = startTimer(1800);
        // Таймер движения слов (каждые 33 мс)
        m_moveTimerId = startTimer(33);

        // Запуск таймера игры
        m_gameTimer.start();

        // Таймер обратного отсчёта (секунда)
        m_countdownTimer = new QTimer(this);
        connect(m_countdownTimer, &QTimer::timeout, [this]() {
            int elapsed = m_gameTimer.elapsed() / 1000;
            int left = m_gameDuration - elapsed;
            if (left <= 0) {
                m_countdownTimer->stop();
                endGame();
            } else {
                emit timeLeftChanged(left);
            }
        });
        m_countdownTimer->start(1000);
    } else if (m_gameMode == SURVIVAL || m_gameMode == INFINITE) {
        // Таймер спавна слов (каждые 1.8 секунды)
        m_spawnTimerId = startTimer(1800);
        // Таймер движения слов (каждые 33 мс)
        m_moveTimerId = startTimer(33);
    }
}

void GameWidget::stopGame()
{
    // Остановка таймеров, если они активны
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
    // Очистка списка слов
    clearWords();
}

void GameWidget::setWordSpeed(double speed)
{
    // Ограничение скорости в допустимом диапазоне
    m_wordSpeed = qBound(1.0, speed, 15.0);
}

bool GameWidget::tryRemoveWord(const QString &input)
{
    QString trimmed = input.trimmed();
    if (trimmed.isEmpty()) return false;

    // Поиск и удаление слова
    for (int i = 0; i < m_words.size(); ++i) {
        if (m_words[i].active && m_words[i].text == trimmed) {
            int points = m_words[i].length;
            m_score += points;
            emit scoreChanged(m_score);
            m_words.removeAt(i);
            return true;
        }
    }

    // Уменьшение жизни, если слово не найдено (только для режима SURVIVAL)
    if (m_gameMode == SURVIVAL) {
        loseLife();
    }
    return false;
}

void GameWidget::loseLife()
{
    // Уменьшение количества жизней
    m_lives--;
    emit livesChanged(m_lives);
    // Завершение игры, если жизней не осталось
    if (m_lives <= 0) {
        endGame();
    }
}

void GameWidget::clearWords()
{
    // Очистка списка слов
    m_words.clear();
    update();
}

void GameWidget::spawnWord()
{
    // Проверка наличия словаря
    if (m_dictionary.isEmpty()) return;

    // Генерация случайного индекса
    int randomIndex = QRandomGenerator::global()->bounded(m_dictionary.size());
    QString word = m_dictionary[randomIndex];
    if (word.isEmpty()) return;

    // Создание нового слова
    MovingWord newWord;
    newWord.text = word;
    newWord.length = word.length();
    newWord.x = 0.0;
    newWord.y = QRandomGenerator::global()->bounded(m_fieldHeight);
    newWord.active = true;
    m_words.append(newWord);
}

void GameWidget::updatePositions()
{
    bool anyMoved = false;
    for (int i = 0; i < m_words.size(); ++i) {
        if (!m_words[i].active) continue;

        // Обновление координаты X слова
        m_words[i].x += m_wordSpeed;

        // Удаление слова, вышедшего за пределы экрана
        if (m_words[i].x > m_fieldWidth + 100) {
            if (m_gameMode == SURVIVAL) {
                loseLife();
            }
            m_words.removeAt(i);
            i--;
            anyMoved = true;
        } else {
            anyMoved = true;
        }
    }

    // Обновление отрисовки, если слова перемещались
    if (anyMoved) update();
}

void GameWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Отрисовка фона
    painter.fillRect(rect(), QColor(240, 240, 245));

    // Настройка шрифта для слов
    QFont font("Arial", 18, QFont::Bold);
    painter.setFont(font);

    // Отрисовка всех активных слов
    for (const MovingWord &word : m_words) {
        painter.setPen(QPen(QColor(50, 50, 180), 2));
        painter.setBrush(QColor(100, 100, 220, 200));

        // Отрисовка прямоугольника вокруг слова
        QRectF textRect(word.x, word.y - 18,
                      QFontMetrics(font).horizontalAdvance(word.text) + 20, 40);
        painter.drawRoundedRect(textRect, 15, 15);

        // Отрисовка текста слова
        painter.setPen(Qt::white);
        painter.drawText(textRect, Qt::AlignCenter, word.text);
    }

    // Отрисовка линии, обозначающей правую границу поля
    painter.setPen(QPen(Qt::red, 2, Qt::DashLine));
    painter.drawLine(m_fieldWidth - 10, 0, m_fieldWidth - 10, m_fieldHeight);
}

void GameWidget::timerEvent(QTimerEvent *event)
{
    // Обработка событий таймеров
    if (event->timerId() == m_spawnTimerId) {
        spawnWord();
    } else if (event->timerId() == m_moveTimerId) {
        updatePositions();
    } else {
        QWidget::timerEvent(event);
    }
}

void GameWidget::endGame()
{
    // Остановка игры и уведомление интерфейса
    stopGame();
    emit gameFinished(m_score);
}
