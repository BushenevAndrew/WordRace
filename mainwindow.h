#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>

class GameWidget;
class QLineEdit;
class QLabel;
class QPushButton;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onStartGame();
    void onGameFinished(int score);
    void onOpenSettings();
    void onOpenWordEditor();

private:
    void setupUI();
    void loadSettings();
    void saveSettings();
    void updateScoreDisplay(int score);
    void updateTimeDisplay(int seconds);

    GameWidget *m_gameWidget;
    QSettings m_settings;
    bool m_gameActive;
    QStringList m_currentWordList;

    // UI элементы
    QLabel *m_scoreLabel;
    QLabel *m_timeLabel;
    QLineEdit *m_inputEdit;
    QPushButton *m_startButton;
    QPushButton *m_settingsButton;
    QPushButton *m_editWordsButton;
};

#endif // MAINWINDOW_H
