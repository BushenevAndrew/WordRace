#ifndef GAMEWIDGET_H
#define GAMEWIDGET_H

#include <QWidget>
#include <QList>
#include <QString>
#include <QTimer>
#include <QElapsedTimer>

struct MovingWord {
    QString text;
    qreal x;
    qreal y;
    int length;
    bool active;
};

class GameWidget : public QWidget
{
    Q_OBJECT

public:
    explicit GameWidget(QWidget *parent = nullptr);
    void startGame(int durationSeconds, const QStringList &wordList, const QString &lang, double wordSpeed = 3.5);
    void stopGame();
    bool tryRemoveWord(const QString &input);
    void setWordSpeed(double speed);

signals:
    void scoreChanged(int newScore);
    void gameFinished(int finalScore);
    void timeLeftChanged(int secondsLeft);

protected:
    void paintEvent(QPaintEvent *event) override;
    void timerEvent(QTimerEvent *event) override;

private:
    void spawnWord();
    void updatePositions();
    void endGame();

    QList<MovingWord> m_words;
    int m_spawnTimerId;
    int m_moveTimerId;
    QElapsedTimer m_gameTimer;
    int m_gameDuration;
    int m_score;
    QStringList m_dictionary;
    QString m_currentLanguage;
    int m_linesCount;
    QList<qreal> m_lineYPositions;
    double m_wordSpeed;
    QTimer *m_countdownTimer;
};

#endif // GAMEWIDGET_H
