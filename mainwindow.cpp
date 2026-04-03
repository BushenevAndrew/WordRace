#include "mainwindow.h"
#include "gamewidget.h"
#include "wordlistloader.h"

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QMessageBox>
#include <QDialog>
#include <QFormLayout>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QGroupBox>
#include <QFileDialog>
#include <QTextEdit>
#include <QTimer>
#include <QDialogButtonBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_settings("MyCompany", "WordRaceGame")
    , m_gameActive(false)
{
    setupUI();
    loadSettings();
    setWindowTitle("Word Race Game");
    resize(900, 600);
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUI()
{
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    // Верхняя панель
    QHBoxLayout *topLayout = new QHBoxLayout;

    m_scoreLabel = new QLabel("Score: 0");
    m_scoreLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #2c3e50;");

    m_timeLabel = new QLabel("Time: 0 sec");
    m_timeLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #e74c3c;");

    m_startButton = new QPushButton("▶ Start Game");
    m_startButton->setStyleSheet("QPushButton { background-color: #27ae60; color: white; padding: 8px 16px; border-radius: 5px; font-weight: bold; }"
                                 "QPushButton:hover { background-color: #229954; }");

    m_settingsButton = new QPushButton("⚙ Settings");
    m_settingsButton->setStyleSheet("QPushButton { background-color: #3498db; color: white; padding: 8px 16px; border-radius: 5px; font-weight: bold; }"
                                    "QPushButton:hover { background-color: #2980b9; }");

    topLayout->addWidget(m_scoreLabel);
    topLayout->addStretch();
    topLayout->addWidget(m_timeLabel);
    topLayout->addStretch();
    topLayout->addWidget(m_startButton);
    topLayout->addWidget(m_settingsButton);

    // Игровое поле
    m_gameWidget = new GameWidget(this);
    m_gameWidget->setMinimumHeight(400);

    // Поле ввода
    m_inputEdit = new QLineEdit;
    m_inputEdit->setPlaceholderText("✏ Type the word here and press Enter...");
    m_inputEdit->setStyleSheet("QLineEdit { padding: 12px; font-size: 14px; border: 2px solid #bdc3c7; border-radius: 5px; }"
                               "QLineEdit:focus { border-color: #3498db; }");
    m_inputEdit->setEnabled(false);

    mainLayout->addLayout(topLayout);
    mainLayout->addWidget(m_gameWidget, 1);
    mainLayout->addWidget(m_inputEdit);

    setCentralWidget(centralWidget);

    // Подключаем сигналы
    connect(m_startButton, &QPushButton::clicked, this, &MainWindow::onStartGame);
    connect(m_settingsButton, &QPushButton::clicked, this, &MainWindow::onOpenSettings);
    connect(m_gameWidget, &GameWidget::scoreChanged, this, &MainWindow::updateScoreDisplay);
    connect(m_gameWidget, &GameWidget::timeLeftChanged, this, &MainWindow::updateTimeDisplay);
    connect(m_gameWidget, &GameWidget::gameFinished, this, &MainWindow::onGameFinished);

    connect(m_inputEdit, &QLineEdit::returnPressed, [this]() {
        if (m_gameActive && m_gameWidget->tryRemoveWord(m_inputEdit->text())) {
            m_inputEdit->clear();
        } else if (m_gameActive) {
            m_inputEdit->setStyleSheet("QLineEdit { padding: 12px; font-size: 14px; border: 2px solid #e74c3c; border-radius: 5px; }");
            QTimer::singleShot(200, [this]() {
                m_inputEdit->setStyleSheet("QLineEdit { padding: 12px; font-size: 14px; border: 2px solid #bdc3c7; border-radius: 5px; }"
                                           "QLineEdit:focus { border-color: #3498db; }");
            });
        }
    });
}

void MainWindow::onStartGame()
{
    if (m_gameActive) return;

    int duration = m_settings.value("duration", 60).toInt();
    double speed = m_settings.value("speed", 3.5).toDouble();
    QString lang = m_settings.value("language", "en").toString();
    QString wordFilePath = m_settings.value("wordfile", "").toString();

    // Загружаем слова
    if (!wordFilePath.isEmpty() && QFile::exists(wordFilePath)) {
        m_currentWordList = WordListLoader::loadFromFile(wordFilePath);
    } else {
        if (lang == "ru") {
            m_currentWordList = WordListLoader::getRussianDefaultWords();
        } else {
            m_currentWordList = WordListLoader::getDefaultWords();
        }
    }

    if (m_currentWordList.isEmpty()) {
        QMessageBox::warning(this, "Error", "No words to play!\nPlease check your word file or select a different one in settings.");
        return;
    }

    m_gameActive = true;
    m_inputEdit->setEnabled(true);
    m_inputEdit->clear();
    m_inputEdit->setFocus();
    m_startButton->setEnabled(false);
    m_settingsButton->setEnabled(false);

    m_gameWidget->startGame(duration, m_currentWordList, lang, speed);
}

void MainWindow::onGameFinished(int score)
{
    m_gameActive = false;
    m_inputEdit->setEnabled(false);
    m_startButton->setEnabled(true);
    m_settingsButton->setEnabled(true);

    int duration = m_settings.value("duration", 60).toInt();
    double wpm = (score / (duration / 60.0));

    QMessageBox::information(this, "Game Over",
        QString("<h2>Game Finished!</h2>"
                "<b>Your score:</b> %1<br>"
                "<b>Words per minute:</b> %2<br>"
                "<b>Total words typed:</b> %3")
        .arg(score)
        .arg(wpm, 0, 'f', 1)
        .arg(score / 3));
}

void MainWindow::onOpenSettings()
{
    QDialog dialog(this);
    dialog.setWindowTitle("Game Settings");
    dialog.setMinimumWidth(450);
    dialog.setStyleSheet("QDialog { background-color: #ecf0f1; }");

    QVBoxLayout *layout = new QVBoxLayout(&dialog);

    // Группа игровых параметров
    QGroupBox *gameGroup = new QGroupBox("🎮 Game Parameters");
    gameGroup->setStyleSheet("QGroupBox { font-weight: bold; margin-top: 10px; }");
    QFormLayout *gameForm = new QFormLayout(gameGroup);

    QSpinBox *durationSpin = new QSpinBox;
    durationSpin->setRange(10, 300);
    durationSpin->setValue(m_settings.value("duration", 60).toInt());
    durationSpin->setSuffix(" seconds");
    durationSpin->setStyleSheet("padding: 5px;");

    QDoubleSpinBox *speedSpin = new QDoubleSpinBox;
    speedSpin->setRange(1.0, 15.0);
    speedSpin->setSingleStep(0.5);
    speedSpin->setValue(m_settings.value("speed", 3.5).toDouble());
    speedSpin->setSuffix(" px/frame");
    speedSpin->setToolTip("Higher speed = harder game");

    QComboBox *langCombo = new QComboBox;
    langCombo->addItems({"English", "Русский"});
    langCombo->setCurrentText(m_settings.value("language") == "ru" ? "Русский" : "English");

    gameForm->addRow("⏱ Game duration:", durationSpin);
    gameForm->addRow("⚡ Word speed:", speedSpin);
    gameForm->addRow("🌐 Language:", langCombo);

    // Группа слов
    QGroupBox *wordsGroup = new QGroupBox("📚 Words Settings");
    wordsGroup->setStyleSheet("QGroupBox { font-weight: bold; margin-top: 10px; }");
    QVBoxLayout *wordsLayout = new QVBoxLayout(wordsGroup);

    QLabel *fileLabel = new QLabel("Word list file:");
    QHBoxLayout *fileLayout = new QHBoxLayout;
    QLineEdit *fileEdit = new QLineEdit;
    fileEdit->setText(m_settings.value("wordfile", "").toString());
    fileEdit->setPlaceholderText("Select a text file with words...");
    QPushButton *browseBtn = new QPushButton("📂 Browse...");
    QPushButton *previewBtn = new QPushButton("👁 Preview");
    previewBtn->setCheckable(true);

    fileLayout->addWidget(fileEdit);
    fileLayout->addWidget(browseBtn);
    fileLayout->addWidget(previewBtn);

    QTextEdit *previewEdit = new QTextEdit;
    previewEdit->setReadOnly(true);
    previewEdit->setMaximumHeight(150);
    previewEdit->setVisible(false);

    wordsLayout->addWidget(fileLabel);
    wordsLayout->addLayout(fileLayout);
    wordsLayout->addWidget(previewEdit);

    // Информация
    QLabel *formatInfo = new QLabel(
        "📖 <b>File format:</b><br>"
        "• One word per line<br>"
        "• Or comma-separated: cat,dog,sun<br>"
        "• Lines starting with # are ignored"
    );
    formatInfo->setWordWrap(true);
    formatInfo->setStyleSheet("color: #7f8c8d; font-size: 11px; margin-top: 10px;");
    wordsLayout->addWidget(formatInfo);

    // Кнопки
    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    buttons->setStyleSheet("QPushButton { padding: 8px 16px; }");

    layout->addWidget(gameGroup);
    layout->addWidget(wordsGroup);
    layout->addWidget(buttons);

    // Подключаем логику предпросмотра
    connect(browseBtn, &QPushButton::clicked, [&dialog, fileEdit, previewEdit, previewBtn]() {
        QString filePath = QFileDialog::getOpenFileName(&dialog, "Select Word File",
                                                         "", "Text Files (*.txt);;All Files (*)");
        if (!filePath.isEmpty()) {
            fileEdit->setText(filePath);
            if (previewBtn->isChecked()) {
                QStringList words = WordListLoader::loadFromFile(filePath);
                previewEdit->setText(words.join(", "));
                previewEdit->setVisible(true);
            }
        }
    });

    connect(previewBtn, &QPushButton::toggled, [fileEdit, previewEdit](bool checked) {
        if (checked && !fileEdit->text().isEmpty()) {
            QStringList words = WordListLoader::loadFromFile(fileEdit->text());
            previewEdit->setText(words.join(", "));
            previewEdit->setVisible(true);
        } else {
            previewEdit->setVisible(false);
        }
    });

    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        m_settings.setValue("duration", durationSpin->value());
        m_settings.setValue("speed", speedSpin->value());
        m_settings.setValue("language", langCombo->currentText() == "Русский" ? "ru" : "en");
        m_settings.setValue("wordfile", fileEdit->text());

        QMessageBox::information(this, "Settings Saved",
            "✅ Settings will be applied on next game start.\n\n"
            "Current speed: " + QString::number(speedSpin->value()) + " px/frame\n"
            "Game duration: " + QString::number(durationSpin->value()) + " seconds");
    }
}

void MainWindow::updateScoreDisplay(int score)
{
    m_scoreLabel->setText(QString("⭐ Score: %1").arg(score));
}

void MainWindow::updateTimeDisplay(int seconds)
{
    m_timeLabel->setText(QString("⏱ Time: %1 sec").arg(seconds));
}

void MainWindow::loadSettings()
{
    if (!m_settings.contains("duration")) m_settings.setValue("duration", 60);
    if (!m_settings.contains("speed")) m_settings.setValue("speed", 3.5);
    if (!m_settings.contains("language")) m_settings.setValue("language", "en");
}

void MainWindow::saveSettings()
{
    m_settings.sync();
}
