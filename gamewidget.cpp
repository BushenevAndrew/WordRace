#include "gamewidget.h"
#include <QPainter>
#include <QRandomGenerator>
#include <QFontMetrics>
#include <QDebug>

// Конструктор виджета игры
GameWidget::GameWidget(QWidget *parent)
    : QWidget(parent),
      m_spawnTimerId(0),
      m_moveTimerId(0),
      m_score(0),
      m_gameDuration(60),
      m_wordSpeed(3.5),
      m_countdownTimer(nullptr),
      m_fieldWidth(800),
      m_fieldHeight(400),
      m_gameMode(TIMED),
      m_lives(3),
      m_timeAttackCounter(0),
      m_timeAttackSpeedMultiplier(1.0),
      m_totalMissed(0)
{
    setMinimumSize(800, 400);
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);
}

// Запуск игры
void GameWidget::startGame(
    int durationSeconds,
    const QStringList &wordList,
    const QString &lang,
    double wordSpeed,
    int fieldWidth,
    int fieldHeight,
    GameMode mode,
    int lives
) {
    stopGame();
    m_missedWords.clear();
    m_totalMissed = 0;

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
    m_timeAttackCounter = 0;
    m_timeAttackSpeedMultiplier = 1.0;

    emit scoreChanged(0);
    emit livesChanged(m_lives);
    emit timeLeftChanged(m_gameDuration);
    emit statisticsUpdated();

    if (m_gameMode == TIMED || m_gameMode == TIME_ATTACK) {
        m_spawnTimerId = startTimer(1800);
        m_moveTimerId = startTimer(33);

        m_gameTimer.start();

        m_countdownTimer = new QTimer(this);
        connect(m_countdownTimer, &QTimer::timeout, [this]() {
            int elapsed = m_gameTimer.elapsed() / 1000;
            int left = m_gameDuration - elapsed;

            emit timeLeftChanged(left);

            if (left <= 0) {
                m_countdownTimer->stop();
                endGame();
            }

            if (m_gameMode == TIME_ATTACK) {
                m_timeAttackCounter++;
                if (m_timeAttackCounter % 5 == 0) {
                    m_timeAttackSpeedMultiplier += 0.2;
                }
            }
        });
        m_countdownTimer->start(1000);
    } else if (m_gameMode == SURVIVAL || m_gameMode == INFINITE) {
        m_spawnTimerId = startTimer(1800);
        m_moveTimerId = startTimer(33);
    }
}

// Остановка игры
void GameWidget::stopGame()
{
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
    clearWords();
}

// Установка скорости слов
void GameWidget::setWordSpeed(double speed)
{
    m_wordSpeed = qBound(1.0, speed, 15.0);
}

// Попытка удалить слово
bool GameWidget::tryRemoveWord(const QString &input)
{
    QString trimmed = input.trimmed();
    if (trimmed.isEmpty()) return false;

    for (int i = 0; i < m_words.size(); ++i) {
        if (m_words[i].active && m_words[i].text == trimmed) {
            int points = m_words[i].length;
            m_score += points;
            emit scoreChanged(m_score);
            m_words.removeAt(i);
            return true;
        }
    }

    if (m_gameMode == SURVIVAL) {
        loseLife();
    }
    return false;
}

// Уменьшение количества жизней
void GameWidget::loseLife()
{
    m_lives--;
    emit livesChanged(m_lives);
    if (m_lives <= 0) {
        endGame();
    }
}

// Очистка всех слов
void GameWidget::clearWords()
{
    m_words.clear();
    update();
}

// Спавн нового слова
void GameWidget::spawnWord()
{
    if (m_dictionary.isEmpty()) return;

    int randomIndex = QRandomGenerator::global()->bounded(m_dictionary.size());
    QString word = m_dictionary[randomIndex];
    if (word.isEmpty()) return;

    MovingWord newWord;
    newWord.text = word;
    newWord.length = word.length();
    newWord.x = 0.0;
    newWord.y = QRandomGenerator::global()->bounded(50, m_fieldHeight - 50);
    newWord.active = true;
    m_words.append(newWord);
}

// Обновление позиций слов
void GameWidget::updatePositions()
{
    bool anyMoved = false;
    for (int i = 0; i < m_words.size(); ++i) {
        if (!m_words[i].active) continue;

        m_words[i].x += m_wordSpeed * m_timeAttackSpeedMultiplier;

        if (m_words[i].x > m_fieldWidth + 100) {
            if (m_gameMode == SURVIVAL) {
                loseLife();
            }
            addMissedWord(m_words[i].text);
            m_words.removeAt(i);
            i--;
            anyMoved = true;
        } else {
            anyMoved = true;
        }
    }

    if (anyMoved) update();
}

// Отрисовка виджета
void GameWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.fillRect(rect(), QColor(240, 240, 245));

    QFont font("Arial", 18, QFont::Bold);
    painter.setFont(font);

    for (const MovingWord &word : m_words) {
        if (!word.active) continue;

        painter.setPen(QPen(QColor(50, 50, 180), 2));
        painter.setBrush(QColor(100, 100, 220, 200));

        QRectF textRect(
            word.x,
            word.y - 18,
            QFontMetrics(font).horizontalAdvance(word.text) + 20,
            40
        );
        painter.drawRoundedRect(textRect, 15, 15);

        painter.setPen(Qt::white);
        painter.drawText(textRect, Qt::AlignCenter, word.text);
    }

    // Линия на правой границе поля
    painter.setPen(QPen(Qt::red, 2, Qt::DashLine));
    painter.drawLine(m_fieldWidth - 10, 0, m_fieldWidth - 10, m_fieldHeight);
}

// Обработка событий таймера
void GameWidget::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_spawnTimerId) {
        spawnWord();
    } else if (event->timerId() == m_moveTimerId) {
        updatePositions();
    } else {
        QWidget::timerEvent(event);
    }
}

// Завершение игры
void GameWidget::endGame()
{
    stopGame();
    emit gameFinished(m_score);
    emit statisticsUpdated();
}

// Добавление пропущенного слова в статистику
void GameWidget::addMissedWord(const QString &word)
{
    m_missedWords[word]++;
    m_totalMissed++;
    emit statisticsUpdated();
}

// Получение статистики пропущенных слов
QMap<QString, int> GameWidget::getMissedWords() const
{
    return m_missedWords;
}

// Получение общего количества пропущенных слов
int GameWidget::getTotalMissed() const
{
    return m_totalMissed;
}

// Получение топ N пропущенных слов
QStringList GameWidget::getTopMissedWords(int count) const
{
    QList<QPair<QString, int>> list;
    for (auto it = m_missedWords.constBegin(); it != m_missedWords.constEnd(); ++it) {
        list.append(qMakePair(it.key(), it.value()));
    }

    std::sort(list.begin(), list.end(), [](const QPair<QString, int> &a, const QPair<QString, int> &b) {
        return a.second > b.second;
    });

    QStringList topWords;
    for (int i = 0; i < qMin(count, list.size()); ++i) {
        topWords << QString("%1 (%2)").arg(list[i].first).arg(list[i].second);
    }
    return topWords;
}
