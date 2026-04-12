#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QMainWindow>
#include <QSettings>

class GameWidget;
class QLineEdit;
class QLabel;
class QPushButton;
class QSpinBox;
class QComboBox;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onStartGame();
    void onStopGame();
    void onGameFinished(int score);
    void onOpenSettings();
    void onOpenWordEditor();

private:
    void setupUI();
    void loadSettings();
    void saveSettings();
    void updateScoreDisplay(int score);
    void updateTimeDisplay(int seconds);
    void updateLivesDisplay(int lives);

    GameWidget *m_gameWidget;
    QSettings m_settings;
    bool m_gameActive;
    QStringList m_currentWordList;
    int m_fieldWidth;
    int m_fieldHeight;

    QLabel *m_scoreLabel;
    QLabel *m_timeLabel;
    QLabel *m_livesLabel;
    QLineEdit *m_inputEdit;
    QPushButton *m_startButton;
    QPushButton *m_stopButton;
    QPushButton *m_settingsButton;
    QPushButton *m_editWordsButton;
    QSpinBox *m_widthSpin;
    QSpinBox *m_heightSpin;
};

#endif // MAINWIDGET_H
