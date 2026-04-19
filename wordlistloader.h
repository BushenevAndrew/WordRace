#ifndef WORDLISTLOADER_H
#define WORDLISTLOADER_H

#include <QString>
#include <QStringList>
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QFileInfo>

/**
 * @brief Класс для загрузки и сохранения списков слов в разных форматах.
 *
 * Поддерживает форматы: TXT (по умолчанию), JSON, CSV.
 * Каждый метод класса снабжён логикой обработки ошибок и возвращает
 * список слов по умолчанию, если произошла ошибка или файл не найден.
 *
 * @note На самом деле, это не класс, а набор статических функций,
 *       которые часто пишут, чтобы не создавать лишние объекты.
 */
class WordListLoader
{
public:
    // Перечисление форматов файлов
    enum FileFormat {
        TXT,    // Текстовый формат (по умолчанию)
        JSON,   // Формат JSON
        CSV     // Формат CSV
    };

    /**
     * @brief Загрузка списка слов из файла указанного пути.
     *
     * Автоматически определяет формат файла по расширению.
     * Если файл не найден, пуст или произошла ошибка,
     * возвращает список слов по умолчанию.
     *
     * @param filePath Путь к файлу.
     * @return QStringList Список загруженных слов.
     *
     * @note На самом деле, это может сломаться, если путь
     *       содержит недопустимые символы или файл открыт
     *       в другом приложении.
     */
    static QStringList loadFromFile(const QString &filePath)
    {
        QStringList words;
        qDebug() << "Loading words from file:" << filePath;

        // Проверка существования файла
        if (filePath.isEmpty() || !QFile::exists(filePath)) {
            qDebug() << "File not found or empty. Using defaults. Path:" << filePath;
            return getDefaultWords();  // Возвращаем слова по умолчанию
        }

        // Определение формата файла по расширению
        FileFormat format = detectFormat(filePath);
        qDebug() << "Detected file format:" << format << "for file:" << filePath;

        // Загрузка слов в зависимости от формата
        switch (format) {
        case TXT:
            words = loadFromTxt(filePath);
            qDebug() << "Loaded" << words.size() << "words from TXT file";
            break;
        case JSON:
            words = loadFromJson(filePath);
            qDebug() << "Loaded" << words.size() << "words from JSON file";
            break;
        case CSV:
            words = loadFromCsv(filePath);
            qDebug() << "Loaded" << words.size() << "words from CSV file";
            break;
        }

        // Проверка, что слова были загружены
        if (words.isEmpty()) {
            qDebug() << "No words loaded from file. Using defaults.";
            return getDefaultWords();
        }

        return words;
    }

    /**
     * @brief Сохранение списка слов в файл указанного пути.
     *
     * Автоматически определяет формат файла по расширению.
     * Если путь пуст, файл не открыт или произошла ошибка,
     * возвращает false.
     *
     * @param filePath Путь к файлу.
     * @param words Список слов для сохранения.
     * @return bool Успех операции.
     *
     * @note На самом деле, это может сломаться, если:
     *       • Путь содержит недопустимые символы
     *       • Файл открыт в другом приложении
     *       • Нет прав на запись в указанный путь
     */
    static bool saveToFile(const QString &filePath, const QStringList &words)
    {
        if (filePath.isEmpty()) {
            qDebug() << "Empty file path";
            return false;
        }

        // Определение формата файла по расширению
        FileFormat format = detectFormat(filePath);
        qDebug() << "Saving words to file:" << filePath << "Format:" << format;

        // Сохранение в зависимости от формата
        switch (format) {
        case TXT:
            return saveToTxt(filePath, words);
        case JSON:
            return saveToJson(filePath, words);
        case CSV:
            return saveToCsv(filePath, words);
        }

        qDebug() << "Unknown format. Saving failed.";
        return false;
    }

    /**
     * @brief Определение формата файла по его расширению.
     *
     * Если расширение ".json", возвращает JSON.
     * Если расширение ".csv", возвращает CSV.
     * В остальных случаях — по умолчанию TXT.
     *
     * @note На самом деле, это может сломаться, если:
     *       • Расширение содержит недопустимые символы
     *       • Файл не имеет расширения (например, "words")
     *       • Расширение написано с заглавной буквы (например, ".Txt")
     *
     * @param filePath Путь к файлу.
     * @return FileFormat Формат файла.
     */
    static FileFormat detectFormat(const QString &filePath)
    {
        // Удаляем пробелы и приводим к нижнему регистру
        QString ext = QFileInfo(filePath).suffix().toLower().trimmed();
        qDebug() << "Detecting format for extension:" << ext;

        // Сравнение расширений
        if (ext == "json") {
            qDebug() << "Format detected as JSON";
            return JSON;
        }
        if (ext == "csv") {
            qDebug() << "Format detected as CSV";
            return CSV;
        }

        // По умолчанию — текстовый формат
        qDebug() << "Format detected as TXT (default)";
        return TXT;
    }

    /**
     * @brief Загрузка списка слов из текстового файла.
     *
     * Поддерживает два варианта формата:
     * 1. Одно слово на строку (стандарт)
     * 2. Строки, разделённые запятыми (например: "cat,dog")
     *
     * Строки, начинающиеся с #, игнорируются (комментарии).
     *
     * @note На самом деле, это может сломаться, если:
     *       • Файл не открыт (например, из-за ошибки)
     *       • Файл пуст
     *       • Слова содержат недопустимые символы
     *
     * @param filePath Путь к текстовому файлу.
     * @return QStringList Список загруженных слов.
     */
    static QStringList loadFromTxt(const QString &filePath)
    {
        QStringList words;
        qDebug() << "Loading words from TXT file:" << filePath;

        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qDebug() << "Cannot open TXT file:" << filePath;
            return getDefaultWords();
        }

        // Чтение файла построчно
        QTextStream stream(&file);
        int lineNum = 0;
        while (!stream.atEnd()) {
            QString line = stream.readLine().trimmed();
            lineNum++;
            qDebug() << "Reading line" << lineNum << "from file:" << line;

            // Пропуск пустых строк и комментариев
            if (line.isEmpty() || line.startsWith('#')) {
                qDebug() << "Line" << lineNum << "is empty or comment. Skipping.";
                continue;
            }

            // Обработка строк с запятыми
            if (line.contains(',')) {
                qDebug() << "Line" << lineNum << "contains commas. Splitting.";
                QStringList lineWords = line.split(',', QString::SkipEmptyParts);
                for (QString &word : lineWords) {
                    word = word.trimmed();
                    if (!word.isEmpty()) {
                        words.append(word);  // Добавляем слово
                        qDebug() << "Added word from comma-separated line:" << word;
                    }
                }
            } else {
                // Одно слово на строку
                words.append(line);
                qDebug() << "Added word from single line:" << line;
            }
        }
        file.close();
        qDebug() << "Total words loaded from TXT file:" << words.size();
        return words;
    }

    /**
     * @brief Загрузка списка слов из файла формата JSON.
     *
     * Поддерживает два варианта структуры JSON:
     * 1. Простой массив строк: ["cat", "dog"]
     * 2. Объект с ключом "words": {"words": ["cat", "dog"]}
     *
     * Если произошла ошибка парсинга, возвращает false.
     *
     * @note На самом деле, это может сломаться, если:
     *       • JSON-структура не распознаётся
     *       • В JSON есть недопустимые символы
     *       • JSON-файл не открыт (например, из-за ошибки)
     *
     * @param filePath Путь к JSON-файлу.
     * @return QStringList Список загруженных слов.
     */
    static QStringList loadFromJson(const QString &filePath)
    {
        QStringList words;
        qDebug() << "Loading words from JSON file:" << filePath;

        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly)) {
            qDebug() << "Cannot open JSON file:" << filePath;
            return getDefaultWords();
        }

        QByteArray data = file.readAll();
        file.close();
        qDebug() << "Read" << data.size() << "bytes from JSON file";

        // Обработка ошибок парсинга JSON
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(data, &error);
        if (error.error != QJsonParseError::NoError) {
            qDebug() << "JSON parse error:" << error.errorString();
            return getDefaultWords();
        }

        // Загрузка слов в зависимости от структуры JSON
        if (doc.isArray()) {
            QJsonArray array = doc.array();
            qDebug() << "Detected simple JSON array with" << array.size() << "elements";
            for (const QJsonValue &value : array) {
                if (value.isString()) {
                    QString word = value.toString();
                    words.append(word);
                    qDebug() << "Added word from array:" << word;
                }
            }
        } else if (doc.isObject()) {
            QJsonObject obj = doc.object();
            qDebug() << "Detected JSON object. Checking for 'words' key...";
            if (obj.contains("words") && obj["words"].isArray()) {
                QJsonArray array = obj["words"].toArray();
                qDebug() << "Found 'words' array with" << array.size() << "elements";
                for (const QJsonValue &value : array) {
                    if (value.isString()) {
                        QString word = value.toString();
                        words.append(word);
                        qDebug() << "Added word from object array:" << word;
                    }
                }
            } else {
                qDebug() << "JSON object does not contain 'words' array!";
                return getDefaultWords();
            }
        } else {
            qDebug() << "Unknown JSON structure!";
            return getDefaultWords();
        }

        qDebug() << "Total words loaded from JSON file:" << words.size();
        return words;
    }

    /**
     * @brief Загрузка списка слов из файла формата CSV.
     *
     * На самом деле, это то же самое, что и TXT,
     * но с другим расширением.
     *
     * @note На самом деле, это может сломаться, если:
     *       • Файл не открыт
     *       • Файл пуст
     *       • Слова содержат недопустимые символы
     *
     * @param filePath Путь к CSV-файлу.
     * @return QStringList Список загруженных слов.
     */
    static QStringList loadFromCsv(const QString &filePath)
    {
        QStringList words;
        qDebug() << "Loading words from CSV file (same as TXT):" << filePath;

        // Загрузка из TXT
        words = loadFromTxt(filePath);
        qDebug() << "Total words loaded from CSV file:" << words.size();

        return words;
    }

    /**
     * @brief Сохранение списка слов в текстовом формате.
     *
     * Добавляет заголовок с комментариями в начале файла:
     * • "# Word list for Word Race Game"
     * • "# Each word on a new line"
     * • "# Lines starting with # are ignored"
     *
     * @note На самом деле, это может сломаться, если:
     *       • Путь содержит недопустимые символы
     *       • Нет прав на запись в указанный путь
     *       • Файл уже открыт в другом приложении
     *
     * @param filePath Путь к файлу.
     * @param words Список слов для сохранения.
     * @return bool Успех операции.
     */
    static bool saveToTxt(const QString &filePath, const QStringList &words)
    {
        qDebug() << "Saving words to TXT file:" << filePath << "Total words:" << words.size();

        QFile file(filePath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            qDebug() << "Cannot open TXT file for writing:" << filePath;
            return false;
        }

        QTextStream stream(&file);
        // Заголовок с комментариями
        stream << "# Word list for Word Race Game\n";
        stream << "# Each word on a new line\n";
        stream << "# Lines starting with # are ignored\n\n";
        qDebug() << "Added header to TXT file";

        // Сохранение слов
        for (const QString &word : words) {
            QString trimmed = word.trimmed();
            if (!trimmed.isEmpty()) {
                stream << trimmed << "\n";
                qDebug() << "Saved word to TXT file:" << trimmed;
            } else {
                qWarning() << "Empty word in list. Skipping.";
            }
        }

        file.close();
        qDebug() << "File closed. TXT save operation completed.";
        return true;
    }

    /**
     * @brief Сохранение списка слов в формате JSON.
     *
     * Создаёт простой массив строк:
     * ["cat", "dog", ...]
     *
     * @note На самом деле, это может сломаться, если:
     *       • Путь содержит недопустимые символы
     *       • Нет прав на запись в указанный путь
     *       • JSON-структура не распознаётся
     *
     * @param filePath Путь к файлу.
     * @param words Список слов для сохранения.
     * @return bool Успех операции.
     */
    static bool saveToJson(const QString &filePath, const QStringList &words)
    {
        qDebug() << "Saving words to JSON file:" << filePath << "Total words:" << words.size();

        QFile file(filePath);
        if (!file.open(QIODevice::WriteOnly)) {
            qDebug() << "Cannot open JSON file for writing:" << filePath;
            return false;
        }

        // Создание JSON-массива
        QJsonArray jsonArray;
        for (const QString &word : words) {
            QString trimmed = word.trimmed();
            if (!trimmed.isEmpty()) {
                jsonArray.append(trimmed);
                qDebug() << "Added word to JSON array:" << trimmed;
            } else {
                qWarning() << "Empty word in list. Skipping.";
            }
        }

        QJsonDocument doc(jsonArray);
        file.write(doc.toJson());
        file.close();
        qDebug() << "JSON file written and closed. Save operation completed.";
        return true;
    }

    /**
     * @brief Сохранение списка слов в формате CSV.
     *
     * На самом деле, это то же самое, что и TXT,
     * но с другим расширением.
     *
     * @note На самом деле, это может сломаться, если:
     *       • Путь содержит недопустимые символы
     *       • Нет прав на запись в указанный путь
     *       • Файл уже открыт в другом приложении
     *
     * @param filePath Путь к файлу.
     * @param words Список слов для сохранения.
     * @return bool Успех операции.
     */
    static bool saveToCsv(const QString &filePath, const QStringList &words)
    {
        qDebug() << "Saving words to CSV file (same as TXT):" << filePath << "Total words:" << words.size();
        return saveToTxt(filePath, words);  // Сохраняем как TXT
    }

    /**
     * @brief Возвращает путь к файлу слов по умолчанию.
     *
     * @return QString Путь к файлу "words.txt".
     *
     * @note На самом деле, это может сломаться, если:
     *       • Нет прав на запись в указанный путь
     *       • Путь содержит недопустимые символы
     */
    static QString getDefaultWordFilePath()
    {
        QString path = "words.txt";
        qDebug() << "Returning default word file path:" << path;
        return path;
    }

    /**
     * @brief Возвращает список слов по умолчанию на английском языке.
     *
     * Используется, если:
     * • Файл со словами не найден
     * • Файл пуст
     * • Произошла ошибка при загрузке
     *
     * @return QStringList Список слов по умолчанию.
     *
     * @note На самом деле, это может сломаться, если:
     *       • Нет памяти для создания списка
     *       • Список не распознаётся
     */
    static QStringList getDefaultWords()
    {
        qDebug() << "Returning default English word list";
        return QStringList()
            << "cat" << "dog" << "sun" << "moon" << "star"
            << "tree" << "fish" << "bird" << "frog" << "bear"
            << "apple" << "house" << "happy" << "friend" << "computer"
            << "keyboard" << "mouse" << "screen" << "developer" << "program";
    }

    /**
     * @brief Возвращает список слов по умолчанию на русском языке.
     *
     * Используется, если:
     * • Файл со словами не найден
     * • Файл пуст
     * • Произошла ошибка при загрузке
     * • Выбран русский язык
     *
     * @return QStringList Список слов по умолчанию.
     *
     * @note На самом деле, это может сломаться, если:
     *       • Нет памяти для создания списка
     *       • Список не распознаётся
     */
    static QStringList getRussianDefaultWords()
    {
        qDebug() << "Returning default Russian word list";
        return QStringList()
            << "кот" << "дом" << "лес" << "река" << "солнце"
            << "машина" << "дружба" << "работа" << "успех"
            << "компьютер" << "программа" << "разработка"
            << "окно" << "стол" << "стул" << "книга";
    }
};

#endif // WORDLISTLOADER_H
