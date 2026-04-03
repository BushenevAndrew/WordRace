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
    m_linesCount = 4;
    m_wordSpeed = 3.5;
    m_countdownTimer = nullptr;
}

void GameWidget::startGame(int durationSeconds, const QStringList &wordList, const QString &lang, double wordSpeed)
{
    stopGame();

    m_dictionary = wordList;
    m_currentLanguage = lang;
    m_gameDuration = durationSeconds;
    m_wordSpeed = wordSpeed;
    m_score = 0;
    m_words.clear();

    emit scoreChanged(0);
    emit timeLeftChanged(durationSeconds);

    m_lineYPositions.clear();
    int startY = 70;
    for (int i = 0; i < m_linesCount; ++i) {
        m_lineYPositions.append(startY + i * 70);
    }

    // Таймеры
    m_spawnTimerId = startTimer(1800);
    m_moveTimerId = startTimer(33);

    m_gameTimer.start();

    // Таймер обратного отсчета
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
    return false;
}

void GameWidget::spawnWord()
{
    if (m_dictionary.isEmpty()) return;

    int randomIndex = QRandomGenerator::global()->bounded(m_dictionary.size());
    QString word = m_dictionary[randomIndex];
    if (word.isEmpty()) return;

    // Выбираем наименее загруженную строку
    QList<int> wordsPerLine;
    // Инициализируем список нулями
    for (int i = 0; i < m_linesCount; ++i) {
        wordsPerLine.append(0);
    }

    for (const MovingWord &w : m_words) {
        int lineIdx = m_lineYPositions.indexOf(w.y);
        if (lineIdx >= 0 && lineIdx < wordsPerLine.size()) {
            wordsPerLine[lineIdx]++;
        }
    }

    int bestLine = 0;
    for (int i = 1; i < m_linesCount; ++i) {
        if (wordsPerLine[i] < wordsPerLine[bestLine]) bestLine = i;
    }

    MovingWord newWord;
    newWord.text = word;
    newWord.length = word.length();
    newWord.x = 0.0;
    newWord.y = m_lineYPositions[bestLine];
    newWord.active = true;
    m_words.append(newWord);
}

void GameWidget::updatePositions()
{
    bool anyMoved = false;
    for (int i = 0; i < m_words.size(); ++i) {
        if (!m_words[i].active) continue;

        m_words[i].x += m_wordSpeed;

        if (m_words[i].x > width() + 100) {
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

    // Отрисовка фона
    painter.fillRect(rect(), QColor(240, 240, 245));

    // Линии-разделители дорожек
    painter.setPen(QPen(QColor(200, 200, 200), 1, Qt::DotLine));
    for (qreal y : m_lineYPositions) {
        painter.drawLine(0, y - 25, width(), y - 25);
        painter.drawLine(0, y + 20, width(), y + 20);
    }

    // Рисуем слова
    QFont font("Arial", 18, QFont::Bold);
    painter.setFont(font);

    for (const MovingWord &word : m_words) {
        painter.setPen(QPen(QColor(50, 50, 180), 2));
        painter.setBrush(QColor(100, 100, 220, 200));

        QRect textRect(word.x, word.y - 18,
                      QFontMetrics(font).horizontalAdvance(word.text) + 20, 40);
        painter.drawRoundedRect(textRect, 5, 5);

        painter.setPen(Qt::white);
        painter.drawText(textRect, Qt::AlignCenter, word.text);
    }

    // Красная граница справа
    painter.setPen(QPen(Qt::red, 2, Qt::DashLine));
    painter.drawLine(width() - 10, 0, width() - 10, height());
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
