#ifndef GAMEWIDGET_H
#define GAMEWIDGET_H

#include <QWidget>
#include <QList>
#include <QString>
#include <QTimer>
#include <QElapsedTimer>

enum GameMode {
    TIMED,
    SURVIVAL,
    INFINITE
};

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
    void startGame(int durationSeconds, const QStringList &wordList, const QString &lang, double wordSpeed = 3.5, int fieldWidth = 800, int fieldHeight = 400, GameMode mode = TIMED, int lives = 3);
    void stopGame();
    bool tryRemoveWord(const QString &input);
    void setWordSpeed(double speed);

signals:
    void scoreChanged(int newScore);
    void gameFinished(int finalScore);
    void timeLeftChanged(int secondsLeft);
    void livesChanged(int livesLeft);

protected:
    void paintEvent(QPaintEvent *event) override;
    void timerEvent(QTimerEvent *event) override;

private:
    void spawnWord();
    void updatePositions();
    void endGame();
    void clearWords();
    void loseLife();

    QList<MovingWord> m_words;
    int m_spawnTimerId;
    int m_moveTimerId;
    QElapsedTimer m_gameTimer;
    int m_gameDuration;
    int m_score;
    QStringList m_dictionary;
    QString m_currentLanguage;
    int m_fieldWidth;
    int m_fieldHeight;
    double m_wordSpeed;
    QTimer *m_countdownTimer;
    GameMode m_gameMode;
    int m_lives;
};

#endif // GAMEWIDGET_H
