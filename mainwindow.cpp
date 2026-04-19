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
#include <QListWidget>

// Конструктор главного окна
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),
    m_settings("MyCompany", "WordRaceGame"),  // Настройки приложения
    m_gameActive(false),                      // Флаг активности игры
    m_fieldWidth(800),                         // Ширина игрового поля по умолчанию
    m_fieldHeight(400)                        // Высота игрового поля по умолчанию
{
    // Настройка интерфейса и загрузка настроек
    setupUI();
    loadSettings();
    setWindowTitle("Word Race Game");
    resize(m_settings.value("windowWidth", 900).toInt(),
           m_settings.value("windowHeight", 600).toInt());

    qDebug() << "MainWindow initialized. Current language:"
             << m_settings.value("language").toString();
}

// Деструктор
MainWindow::~MainWindow()
{
    // Сохранение размеров окна при закрытии
    m_settings.setValue("windowWidth", width());
    m_settings.setValue("windowHeight", height());
    m_settings.sync();
    qDebug() << "Settings saved. Window size:"
             << width() << "x" << height();
}

// Настройка интерфейса главного окна
void MainWindow::setupUI()
{
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    // Верхняя панель с метками и кнопками
    QHBoxLayout *topLayout = new QHBoxLayout;

    // Метка счёта
    m_scoreLabel = new QLabel("Score: 0");
    m_scoreLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #2c3e50;");

    // Метка времени
    m_timeLabel = new QLabel("Time: 0 sec");
    m_timeLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #e74c3c;");

    // Метка жизней (скрыта по умолчанию)
    m_livesLabel = new QLabel("Lives: 3");
    m_livesLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #27ae60;");
    m_livesLabel->setVisible(false);

    // Кнопка "Старт"
    m_startButton = new QPushButton("▶ Start Game");
    m_startButton->setStyleSheet("QPushButton { background-color: #27ae60; color: white; padding: 8px 16px; border-radius: 5px; font-weight: bold; }"
                                 "QPushButton:hover { background-color: #229954; }");

    // Кнопка "Стоп" (скрыта по умолчанию)
    m_stopButton = new QPushButton("⏹ Stop Game");
    m_stopButton->setStyleSheet("QPushButton { background-color: #e74c3c; color: white; padding: 8px 16px; border-radius: 5px; font-weight: bold; }"
                                "QPushButton:hover { background-color: #c0392b; }");
    m_stopButton->setVisible(false);

    // Кнопка "Настройки"
    m_settingsButton = new QPushButton("⚙ Settings");
    m_settingsButton->setStyleSheet("QPushButton { background-color: #3498db; color: white; padding: 8px 16px; border-radius: 5px; font-weight: bold; }"
                                    "QPushButton:hover { background-color: #2980b9; }");

    // Кнопка "Редактор слов"
    m_editWordsButton = new QPushButton("✏ Edit Words");
    m_editWordsButton->setStyleSheet("QPushButton { background-color: #e67e22; color: white; padding: 8px 16px; border-radius: 5px; font-weight: bold; }"
                                     "QPushButton:hover { background-color: #d35400; }");

    // Кнопка "Статистика"
    m_statisticsButton = new QPushButton("📊 Stats");
    m_statisticsButton->setStyleSheet("QPushButton { background-color: #34495e; color: white; padding: 8px 16px; border-radius: 5px; font-weight: bold; }"
                                     "QPushButton:hover { background-color: #2c3e50; }");

    // Добавление виджетов в верхнюю панель
    topLayout->addWidget(m_scoreLabel);
    topLayout->addWidget(m_livesLabel);
    topLayout->addStretch();
    topLayout->addWidget(m_timeLabel);
    topLayout->addStretch();
    topLayout->addWidget(m_startButton);
    topLayout->addWidget(m_stopButton);
    topLayout->addWidget(m_settingsButton);
    topLayout->addWidget(m_editWordsButton);
    topLayout->addWidget(m_statisticsButton);

    // Создание и настройка виджета игры
    m_gameWidget = new GameWidget(this);
    m_gameWidget->setMinimumSize(m_fieldWidth, m_fieldHeight);

    // Поле для ввода слов
    m_inputEdit = new QLineEdit;
    m_inputEdit->setPlaceholderText("✏ Type the word here and press Enter...");
    m_inputEdit->setStyleSheet("QLineEdit { padding: 12px; font-size: 14px; border: 2px solid #bdc3c7; border-radius: 5px; }"
                               "QLineEdit:focus { border-color: #3498db; }");
    m_inputEdit->setEnabled(false);

    // Добавление виджетов в основной макет
    mainLayout->addLayout(topLayout);
    mainLayout->addWidget(m_gameWidget, 1);
    mainLayout->addWidget(m_inputEdit);

    setCentralWidget(centralWidget);

    // Подключение сигналов и слотов
    connect(m_startButton, &QPushButton::clicked, this, &MainWindow::onStartGame);
    connect(m_stopButton, &QPushButton::clicked, this, &MainWindow::onStopGame);
    connect(m_settingsButton, &QPushButton::clicked, this, &MainWindow::onOpenSettings);
    connect(m_editWordsButton, &QPushButton::clicked, this, &MainWindow::onOpenWordEditor);
    connect(m_statisticsButton, &QPushButton::clicked, this, &MainWindow::onOpenStatistics);
    connect(m_gameWidget, &GameWidget::scoreChanged, this, &MainWindow::updateScoreDisplay);
    connect(m_gameWidget, &GameWidget::timeLeftChanged, this, &MainWindow::updateTimeDisplay);
    connect(m_gameWidget, &GameWidget::livesChanged, this, &MainWindow::updateLivesDisplay);
    connect(m_gameWidget, &GameWidget::gameFinished, this, &MainWindow::onGameFinished);
    connect(m_gameWidget, &GameWidget::statisticsUpdated, this, &MainWindow::updateStatisticsDisplay);

    // Обработка ввода слова
    connect(m_inputEdit, &QLineEdit::returnPressed, [this]() {
        if (m_gameActive && m_gameWidget->tryRemoveWord(m_inputEdit->text())) {
            m_inputEdit->clear();
        } else if (m_gameActive) {
            // Подсветка ошибки ввода
            m_inputEdit->setStyleSheet("QLineEdit { padding: 12px; font-size: 14px; border: 2px solid #e74c3c; border-radius: 5px; }");
            QTimer::singleShot(200, [this]() {
                m_inputEdit->setStyleSheet("QLineEdit { padding: 12px; font-size: 14px; border: 2px solid #bdc3c7; border-radius: 5px; }"
                                           "QLineEdit:focus { border-color: #3498db; }");
            });
        }
    });

    // Инициализация таймера для обновления интерфейса
    m_updateTimer = new QTimer(this);
    connect(m_updateTimer, &QTimer::timeout, this, &MainWindow::updateInterface);
    m_updateTimer->start(500);
}

// Запуск игры
void MainWindow::onStartGame()
{
    if (m_gameActive) {
        qWarning() << "Game is already running!";
        return;
    }

    // Загрузка настроек игры
    int duration = m_settings.value("duration", 60).toInt();
    double speed = m_settings.value("speed", 3.5).toDouble();
    QString lang = m_settings.value("language", "en").toString();
    QString wordFilePath = m_settings.value("wordfile", WordListLoader::getDefaultWordFilePath()).toString();
    GameMode mode = static_cast<GameMode>(m_settings.value("mode", TIMED).toInt());
    int lives = m_settings.value("lives", 3).toInt();

    // Загрузка слов из файла или использование слов по умолчанию
    if (!wordFilePath.isEmpty() && QFile::exists(wordFilePath)) {
        m_currentWordList = WordListLoader::loadFromFile(wordFilePath);
    } else {
        if (lang == "ru") {
            m_currentWordList = WordListLoader::getRussianDefaultWords();
        } else {
            m_currentWordList = WordListLoader::getDefaultWords();
        }
        WordListLoader::saveToFile(wordFilePath, m_currentWordList);
    }

    // Проверка наличия слов
    if (m_currentWordList.isEmpty()) {
        QMessageBox::warning(this, "Error", "No words to play!\nPlease add some words using the 'Edit Words' button.");
        return;
    }

    // Настройка интерфейса для игры
    m_gameActive = true;
    m_inputEdit->setEnabled(true);
    m_inputEdit->clear();
    m_inputEdit->setFocus();
    m_startButton->setEnabled(false);
    m_stopButton->setVisible(true);
    m_settingsButton->setEnabled(false);
    m_editWordsButton->setEnabled(false);

    // Настройка отображения меток в зависимости от режима игры
    if (mode == SURVIVAL) {
        m_timeLabel->setVisible(false);
        m_livesLabel->setVisible(true);
    } else if (mode == TIMED || mode == TIME_ATTACK) {
        m_timeLabel->setVisible(true);
        m_livesLabel->setVisible(false);
    } else if (mode == INFINITE) {
        m_timeLabel->setVisible(false);
        m_livesLabel->setVisible(false);
    }

    // Запуск игры
    m_gameWidget->startGame(duration, m_currentWordList, lang, speed, m_fieldWidth, m_fieldHeight, mode, lives);
    qDebug() << "Game started. Mode:" << mode << "Duration:" << duration << "sec";
}

// Остановка игры
void MainWindow::onStopGame()
{
    if (!m_gameActive) {
        qWarning() << "No active game to stop!";
        return;
    }

    m_gameWidget->stopGame();
    m_gameActive = false;
    m_inputEdit->setEnabled(false);
    m_startButton->setEnabled(true);
    m_stopButton->setVisible(false);
    m_settingsButton->setEnabled(true);
    m_editWordsButton->setEnabled(true);
    m_statisticsButton->setEnabled(true);
    m_scoreLabel->setText("Score: 0");
    m_timeLabel->setText("Time: 0 sec");
    m_livesLabel->setText("Lives: 3");

    qDebug() << "Game stopped.";
}

// Обработка завершения игры
void MainWindow::onGameFinished(int score)
{
    m_gameActive = false;
    m_inputEdit->setEnabled(false);
    m_startButton->setEnabled(true);
    m_stopButton->setVisible(false);
    m_settingsButton->setEnabled(true);
    m_editWordsButton->setEnabled(true);
    m_statisticsButton->setEnabled(true);

    QString missedWords = QString::number(m_gameWidget->getTotalMissed());
    QMessageBox::information(this, "Game Over",
        QString("<h2>Game Finished!</h2>"
                "<b>Your score:</b> %1<br>"
                "<b>Missed words:</b> %2")
        .arg(score)
        .arg(missedWords));

    qDebug() << "Game finished. Final score:" << score << "Missed words:" << missedWords;
}

// Открытие окна настроек
void MainWindow::onOpenSettings()
{
    QDialog dialog(this);
    dialog.setWindowTitle("Game Settings");
    dialog.setMinimumWidth(500);
    dialog.setStyleSheet("QDialog { background-color: #ecf0f1; }");

    QVBoxLayout *layout = new QVBoxLayout(&dialog);

    // Группа параметров игры
    QGroupBox *gameGroup = new QGroupBox("🎮 Game Parameters");
    gameGroup->setStyleSheet("QGroupBox { font-weight: bold; margin-top: 10px; }");
    QFormLayout *gameForm = new QFormLayout(gameGroup);

    // Настройка длительности игры
    QSpinBox *durationSpin = new QSpinBox;
    durationSpin->setRange(10, 300);
    durationSpin->setValue(m_settings.value("duration", 60).toInt());
    durationSpin->setSuffix(" seconds");
    durationSpin->setStyleSheet("padding: 5px;");

    // Настройка скорости слов
    QDoubleSpinBox *speedSpin = new QDoubleSpinBox;
    speedSpin->setRange(1.0, 15.0);
    speedSpin->setSingleStep(0.5);
    speedSpin->setValue(m_settings.value("speed", 3.5).toDouble());
    speedSpin->setSuffix(" px/frame");
    speedSpin->setToolTip("Higher speed = harder game");

    // Выбор языка
    QComboBox *langCombo = new QComboBox;
    langCombo->addItems({"English", "Русский"});
    langCombo->setCurrentText(m_settings.value("language") == "ru" ? "Русский" : "English");

    // Выбор режима игры
    QComboBox *modeCombo = new QComboBox;
    modeCombo->addItems({"Timed", "Survival", "Infinite", "Time Attack"});
    modeCombo->setCurrentIndex(m_settings.value("mode", TIMED).toInt());

    // Настройка количества жизней
    QSpinBox *livesSpin = new QSpinBox;
    livesSpin->setRange(1, 10);
    livesSpin->setValue(m_settings.value("lives", 3).toInt());
    livesSpin->setSuffix(" lives");

    // Группа параметров игрового поля
    QGroupBox *fieldGroup = new QGroupBox("📏 Field Size");
    fieldGroup->setStyleSheet("QGroupBox { font-weight: bold; margin-top: 10px; }");
    QFormLayout *fieldForm = new QFormLayout(fieldGroup);

    // Настройка ширины поля
    m_widthSpin = new QSpinBox;
    m_widthSpin->setRange(400, 1600);
    m_widthSpin->setValue(m_settings.value("fieldWidth", 800).toInt());
    m_widthSpin->setSuffix(" px");

    // Настройка высоты поля
    m_heightSpin = new QSpinBox;
    m_heightSpin->setRange(300, 900);
    m_heightSpin->setValue(m_settings.value("fieldHeight", 400).toInt());
    m_heightSpin->setSuffix(" px");

    fieldForm->addRow("Width:", m_widthSpin);
    fieldForm->addRow("Height:", m_heightSpin);

    // Добавление виджетов в формы
    gameForm->addRow("⏱ Game duration:", durationSpin);
    gameForm->addRow("⚡ Word speed:", speedSpin);
    gameForm->addRow("🌐 Language:", langCombo);
    gameForm->addRow("🎯 Mode:", modeCombo);
    gameForm->addRow("❤️ Lives:", livesSpin);

    // Группа настроек слов
    QGroupBox *wordsGroup = new QGroupBox("📚 Words Settings");
    wordsGroup->setStyleSheet("QGroupBox { font-weight: bold; margin-top: 10px; }");
    QVBoxLayout *wordsLayout = new QVBoxLayout(wordsGroup);

    // Поле для выбора файла со словами
    QLabel *fileLabel = new QLabel("Word list file:");
    QHBoxLayout *fileLayout = new QHBoxLayout;
    QLineEdit *fileEdit = new QLineEdit;
    fileEdit->setText(m_settings.value("wordfile", WordListLoader::getDefaultWordFilePath()).toString());
    fileEdit->setPlaceholderText("Select a text file with words...");
    QPushButton *browseBtn = new QPushButton("📂 Browse...");
    QPushButton *previewBtn = new QPushButton("👁 Preview");
    previewBtn->setCheckable(true);

    fileLayout->addWidget(fileEdit);
    fileLayout->addWidget(browseBtn);
    fileLayout->addWidget(previewBtn);

    // Поле для предпросмотра слов
    QTextEdit *previewEdit = new QTextEdit;
    previewEdit->setReadOnly(true);
    previewEdit->setMaximumHeight(150);
    previewEdit->setVisible(false);

    wordsLayout->addWidget(fileLabel);
    wordsLayout->addLayout(fileLayout);
    wordsLayout->addWidget(previewEdit);

    // Информация о формате файла
    QLabel *formatInfo = new QLabel(
        "📖 <b>File format:</b><br>"
        "• One word per line<br>"
        "• Or comma-separated: cat,dog,sun<br>"
        "• Lines starting with # are ignored"
    );
    formatInfo->setWordWrap(true);
    formatInfo->setStyleSheet("color: #7f8c8d; font-size: 11px; margin-top: 10px;");

    wordsLayout->addWidget(formatInfo);

    // Кнопки "OK" и "Cancel"
    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    buttons->setStyleSheet("QPushButton { padding: 8px 16px; }");

    // Добавление групп в основной макет
    layout->addWidget(gameGroup);
    layout->addWidget(fieldGroup);
    layout->addWidget(wordsGroup);
    layout->addWidget(buttons);

    // Блокировка настроек жизней, если режим не "Survival"
    connect(modeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index) {
        livesSpin->setEnabled(index == SURVIVAL);
    });
    livesSpin->setEnabled(modeCombo->currentIndex() == SURVIVAL);

    // Обработка кнопки "Browse..."
    connect(browseBtn, &QPushButton::clicked, [&dialog, fileEdit, previewEdit, previewBtn]() {
        QString filePath = QFileDialog::getOpenFileName(&dialog, "Select Word File", "", "Text Files (*.txt);;All Files (*)");
        if (!filePath.isEmpty()) {
            fileEdit->setText(filePath);
            if (previewBtn->isChecked()) {
                QStringList words = WordListLoader::loadFromFile(filePath);
                previewEdit->setText(words.join(", "));
                previewEdit->setVisible(true);
            }
        }
    });

    // Обработка кнопки предпросмотра
    connect(previewBtn, &QPushButton::toggled, [fileEdit, previewEdit](bool checked) {
        if (checked && !fileEdit->text().isEmpty()) {
            QStringList words = WordListLoader::loadFromFile(fileEdit->text());
            previewEdit->setText(words.join(", "));
            previewEdit->setVisible(true);
        } else {
            previewEdit->setVisible(false);
        }
    });

    // Обработка кнопки "OK"
    connect(buttons, &QDialogButtonBox::accepted, [&]() {
        // Сохранение настроек
        m_settings.setValue("duration", durationSpin->value());
        m_settings.setValue("speed", speedSpin->value());
        m_settings.setValue("language", langCombo->currentText() == "Русский" ? "ru" : "en");
        m_settings.setValue("mode", modeCombo->currentIndex());
        m_settings.setValue("lives", livesSpin->value());
        m_settings.setValue("wordfile", fileEdit->text());
        m_settings.setValue("fieldWidth", m_widthSpin->value());
        m_settings.setValue("fieldHeight", m_heightSpin->value());

        // Обновление размеров игрового поля
        m_fieldWidth = m_widthSpin->value();
        m_fieldHeight = m_heightSpin->value();
        m_gameWidget->setMinimumSize(m_fieldWidth, m_fieldHeight);

        QMessageBox::information(this, "Settings Saved",
            "✅ Settings will be applied on next game start.\n\n"
            "Current field size: " + QString::number(m_fieldWidth) + "x" + QString::number(m_fieldHeight) + " px");

        qDebug() << "Settings saved. Field size:"
                 << m_fieldWidth << "x" << m_fieldHeight;

        dialog.accept();
    });

    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    dialog.exec();
}

// Открытие редактора слов
void MainWindow::onOpenWordEditor()
{
    QDialog dialog(this);
    dialog.setWindowTitle("✏ Word List Editor");
    dialog.setMinimumSize(500, 600);
    dialog.setStyleSheet("QDialog { background-color: #ecf0f1; }");

    QVBoxLayout *layout = new QVBoxLayout(&dialog);

    QLabel *infoLabel = new QLabel(
        "<h3>📝 Edit Your Word List</h3>"
        "<p>Add, remove, or modify words for the game.<br>"
        "<b>Tip:</b> Each word should be on a new line.</p>"
    );
    infoLabel->setWordWrap(true);
    layout->addWidget(infoLabel);

    QTextEdit *wordEditor = new QTextEdit;
    wordEditor->setFont(QFont("Consolas", 11));
    wordEditor->setStyleSheet("QTextEdit { background-color: white; border: 2px solid #bdc3c7; border-radius: 5px; padding: 10px; }");

    // Загрузка текущего списка слов
    QString wordFilePath = m_settings.value("wordfile", WordListLoader::getDefaultWordFilePath()).toString();
    QStringList currentWords;

    if (!wordFilePath.isEmpty() && QFile::exists(wordFilePath)) {
        currentWords = WordListLoader::loadFromFile(wordFilePath);
    } else {
        QString lang = m_settings.value("language", "en").toString();
        if (lang == "ru") {
            currentWords = WordListLoader::getRussianDefaultWords();
        } else {
            currentWords = WordListLoader::getDefaultWords();
        }
    }

    wordEditor->setPlainText(currentWords.join("\n"));
    layout->addWidget(wordEditor);

    QHBoxLayout *buttonLayout = new QHBoxLayout;

    // Кнопка загрузки из файла
    QPushButton *loadFileBtn = new QPushButton("📂 Load from File");
    loadFileBtn->setStyleSheet("QPushButton { padding: 8px 16px; background-color: #3498db; color: white; border-radius: 5px; }"
                               "QPushButton:hover { background-color: #2980b9; }");

    // Кнопка сохранения в файл
    QPushButton *saveToFileBtn = new QPushButton("💾 Save to File");
    saveToFileBtn->setStyleSheet("QPushButton { padding: 8px 16px; background-color: #27ae60; color: white; border-radius: 5px; }"
                                 "QPushButton:hover { background-color: #229954; }");

    // Кнопка сброса на стандартные слова
    QPushButton *defaultWordsBtn = new QPushButton("🔄 Reset to Default");
    defaultWordsBtn->setStyleSheet("QPushButton { padding: 8px 16px; background-color: #e67e22; color: white; border-radius: 5px; }"
                                   "QPushButton:hover { background-color: #d35400; }");

    // Кнопка очистки всех слов
    QPushButton *clearBtn = new QPushButton("🗑 Clear All");
    clearBtn->setStyleSheet("QPushButton { padding: 8px 16px; background-color: #e74c3c; color: white; border-radius: 5px; }"
                            "QPushButton:hover { background-color: #c0392b; }");

    buttonLayout->addWidget(loadFileBtn);
    buttonLayout->addWidget(saveToFileBtn);
    buttonLayout->addWidget(defaultWordsBtn);
    buttonLayout->addWidget(clearBtn);
    layout->addLayout(buttonLayout);

    // Статистика слов
    QLabel *statsLabel = new QLabel;
    statsLabel->setStyleSheet("color: #7f8c8d; font-size: 12px; margin-top: 10px;");

    auto updateStats = [statsLabel, wordEditor]() {
        QString text = wordEditor->toPlainText();
        QStringList lines = text.split("\n", QString::SkipEmptyParts);
        int wordCount = 0;
        for (const QString &line : lines) {
            if (!line.trimmed().isEmpty()) {
                wordCount++;
            }
        }
        statsLabel->setText(QString("📊 Statistics: %1 words loaded | %2 lines total")
                           .arg(wordCount).arg(lines.size()));
    };

    updateStats();
    connect(wordEditor, &QTextEdit::textChanged, updateStats);
    layout->addWidget(statsLabel);

    // Кнопки "OK" и "Cancel"
    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    buttons->setStyleSheet("QPushButton { padding: 8px 16px; }");
    layout->addWidget(buttons);

    // Функция сохранения слов в файл
    auto saveWordsToFile = [&](const QStringList &words) -> bool {
        QString savePath = m_settings.value("wordfile", WordListLoader::getDefaultWordFilePath()).toString();
        if (savePath.isEmpty()) {
            savePath = WordListLoader::getDefaultWordFilePath();
            m_settings.setValue("wordfile", savePath);
        }
        return WordListLoader::saveToFile(savePath, words);
    };

    // Обработка кнопки загрузки из файла
    connect(loadFileBtn, &QPushButton::clicked, [&dialog, wordEditor]() {
        QString filePath = QFileDialog::getOpenFileName(&dialog, "Load Word List", "", "Text Files (*.txt);;All Files (*)");
        if (!filePath.isEmpty()) {
            QStringList words = WordListLoader::loadFromFile(filePath);
            wordEditor->setPlainText(words.join("\n"));
            QMessageBox::information(&dialog, "Success", QString("Loaded %1 words from file.").arg(words.size()));
        }
    });

    // Обработка кнопки сохранения в файл
    connect(saveToFileBtn, &QPushButton::clicked, [&dialog, wordEditor, saveWordsToFile]() {
        QString text = wordEditor->toPlainText();
        QStringList words;
        for (const QString &line : text.split("\n", QString::SkipEmptyParts)) {
            QString trimmed = line.trimmed();
            if (!trimmed.isEmpty()) {
                words.append(trimmed);
            }
        }
        if (words.isEmpty()) {
            QMessageBox::warning(&dialog, "Error", "Cannot save empty word list!");
            return;
        }
        if (saveWordsToFile(words)) {
            QMessageBox::information(&dialog, "Success", QString("✅ Saved %1 words to file!\n\nYou can use them in the next game.").arg(words.size()));
        } else {
            QMessageBox::critical(&dialog, "Error", "Failed to save words to file!");
        }
    });

    // Обработка кнопки сброса на стандартные слова
    connect(defaultWordsBtn, &QPushButton::clicked, [wordEditor]() {
        QStringList defaultWords = WordListLoader::getDefaultWords();
        wordEditor->setPlainText(defaultWords.join("\n"));
        QMessageBox::information(nullptr, "Reset", "Default word list loaded.");
    });

    // Обработка кнопки очистки
    connect(clearBtn, &QPushButton::clicked, [wordEditor]() {
        if (QMessageBox::question(nullptr, "Clear All", "Are you sure you want to clear all words?", QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
            wordEditor->clear();
        }
    });

    // Обработка кнопки "OK"
    connect(buttons, &QDialogButtonBox::accepted, [&, saveWordsToFile]() {
        QString text = wordEditor->toPlainText();
        QStringList words;
        for (const QString &line : text.split("\n", QString::SkipEmptyParts)) {
            QString trimmed = line.trimmed();
            if (!trimmed.isEmpty()) {
                words.append(trimmed);
            }
        }
        if (!words.isEmpty()) {
            if (saveWordsToFile(words)) {
                dialog.accept();
            } else {
                QMessageBox::critical(&dialog, "Error", "Failed to save changes!");
            }
        } else {
            QMessageBox::warning(&dialog, "Error", "Word list cannot be empty!");
        }
    });

    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    dialog.exec();
}

// Открытие статистики
void MainWindow::onOpenStatistics()
{
    QDialog dialog(this);
    dialog.setWindowTitle("📊 Game Statistics");
    dialog.setMinimumSize(500, 400);
    dialog.setStyleSheet("QDialog { background-color: #ecf0f1; }");

    QVBoxLayout *layout = new QVBoxLayout(&dialog);

    // Общая статистика
    QLabel *totalLabel = new QLabel(QString("<b>Total missed words:</b> %1")
                                  .arg(m_gameWidget->getTotalMissed()));
    totalLabel->setStyleSheet("font-size: 16px; margin: 10px;");

    // Топ пропущенных слов
    QLabel *topLabel = new QLabel("<b>Top missed words:</b>");
    topLabel->setStyleSheet("font-size: 16px; margin: 10px;");

    QListWidget *topWordsList = new QListWidget;
    QStringList topWords = m_gameWidget->getTopMissedWords(10);
    for (const QString &word : topWords) {
        topWordsList->addItem(word);
    }

    // Кнопка "OK"
    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok);
    buttons->setStyleSheet("QPushButton { padding: 8px 16px; }");

    layout->addWidget(totalLabel);
    layout->addWidget(topLabel);
    layout->addWidget(topWordsList);
    layout->addWidget(buttons);

    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);

    dialog.exec();
}

// Обновление отображения счёта
void MainWindow::updateScoreDisplay(int score)
{
    m_scoreLabel->setText(QString("⭐ Score: %1").arg(score));
}

// Обновление отображения времени
void MainWindow::updateTimeDisplay(int seconds)
{
    m_timeLabel->setText(QString("⏱ Time: %1 sec").arg(seconds));
}

// Обновление отображения жизней
void MainWindow::updateLivesDisplay(int lives)
{
    m_livesLabel->setText(QString("❤️ Lives: %1").arg(lives));
}

// Обновление статистики
void MainWindow::updateStatisticsDisplay()
{
    // Можно добавить обновление меток статистики в интерфейсе, если нужно
}

// Загрузка настроек из файла
void MainWindow::loadSettings()
{
    // Установка значений по умолчанию, если настройки не существуют
    if (!m_settings.contains("duration")) m_settings.setValue("duration", 60);
    if (!m_settings.contains("speed")) m_settings.setValue("speed", 3.5);
    if (!m_settings.contains("language")) m_settings.setValue("language", "en");
    if (!m_settings.contains("mode")) m_settings.setValue("mode", TIMED);
    if (!m_settings.contains("lives")) m_settings.setValue("lives", 3);
    if (!m_settings.contains("wordfile")) m_settings.setValue("wordfile", WordListLoader::getDefaultWordFilePath());
    if (!m_settings.contains("fieldWidth")) m_settings.setValue("fieldWidth", 800);
    if (!m_settings.contains("fieldHeight")) m_settings.setValue("fieldHeight", 400);

    // Обновление размеров игрового поля
    m_fieldWidth = m_settings.value("fieldWidth").toInt();
    m_fieldHeight = m_settings.value("fieldHeight").toInt();
    qDebug() << "Settings loaded. Current field size:"
             << m_fieldWidth << "x" << m_fieldHeight;
}

// Сохранение настроек в файл
void MainWindow::saveSettings()
{
    m_settings.sync();
    qDebug() << "Settings saved to file.";
}

// Обновление интерфейса
void MainWindow::updateInterface()
{
    if (m_gameActive && m_gameWidget) {
        // Можно добавить дополнительную логику обновления интерфейса
    }
}
