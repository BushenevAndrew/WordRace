#include "gamewidget.h"
#include <QPainter>
#include <QRandomGenerator>
#include <QFontMetrics>
#include <QDebug>

GameWidget::GameWidget(QWidget *parent) : QWidget(parent)
{
    setMinimumSize(800, 400);
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);

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
    stopGame();

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

    emit scoreChanged(0);
    emit livesChanged(m_lives);

    if (m_gameMode == TIMED) {
        m_spawnTimerId = startTimer(1800);
        m_moveTimerId = startTimer(33);

        m_gameTimer.start();

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
        m_spawnTimerId = startTimer(1800);
        m_moveTimerId = startTimer(33);
    }
}

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

void GameWidget::setWordSpeed(double speed)
{
    m_wordSpeed = qBound(1.0, speed, 15.0);
}

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

void GameWidget::loseLife()
{
    m_lives--;
    emit livesChanged(m_lives);
    if (m_lives <= 0) {
        endGame();
    }
}

void GameWidget::clearWords()
{
    m_words.clear();
    update();
}

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
    newWord.y = QRandomGenerator::global()->bounded(m_fieldHeight);
    newWord.active = true;
    m_words.append(newWord);
}

void GameWidget::updatePositions()
{
    bool anyMoved = false;
    for (int i = 0; i < m_words.size(); ++i) {
        if (!m_words[i].active) continue;

        m_words[i].x += m_wordSpeed;

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

    if (anyMoved) update();
}

void GameWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.fillRect(rect(), QColor(240, 240, 245));

    QFont font("Arial", 18, QFont::Bold);
    painter.setFont(font);

    for (const MovingWord &word : m_words) {
        painter.setPen(QPen(QColor(50, 50, 180), 2));
        painter.setBrush(QColor(100, 100, 220, 200));

        QRectF textRect(word.x, word.y - 18,
                      QFontMetrics(font).horizontalAdvance(word.text) + 20, 40);
        painter.drawRoundedRect(textRect, 15, 15);

        painter.setPen(Qt::white);
        painter.drawText(textRect, Qt::AlignCenter, word.text);
    }

    painter.setPen(QPen(Qt::red, 2, Qt::DashLine));
    painter.drawLine(m_fieldWidth - 10, 0, m_fieldWidth - 10, m_fieldHeight);
}

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

void GameWidget::endGame()
{
    stopGame();
    emit gameFinished(m_score);
}
