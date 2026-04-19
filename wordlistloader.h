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
 * список слов по умолчанию, если произошла ошибка.
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
     * Если файл не найден или пуст, возвращает список слов по умолчанию.
     *
     * @param filePath Путь к файлу.
     * @return QStringList Список загруженных слов.
     */
    static QStringList loadFromFile(const QString &filePath)
    {
        QStringList words;

        // Проверка существования файла
        if (filePath.isEmpty() || !QFile::exists(filePath)) {
            qDebug() << "File not found or empty path:" << filePath;
            return getDefaultWords();  // Возвращаем слова по умолчанию
        }

        // Определение формата файла по расширению
        FileFormat format = detectFormat(filePath);
        qDebug() << "Detected format:" << format;

        // Загрузка слов в зависимости от формата
        switch (format) {
        case TXT:
            words = loadFromTxt(filePath);
            break;
        case JSON:
            words = loadFromJson(filePath);
            break;
        case CSV:
            words = loadFromCsv(filePath);
            break;
        }

        // Проверка, что слова были загружены
        if (words.isEmpty()) {
            qDebug() << "No words loaded, using defaults";
            return getDefaultWords();
        }

        return words;
    }

    /**
     * @brief Сохранение списка слов в файл указанного пути.
     *
     * Автоматически определяет формат файла по расширению.
     * Если путь пуст, возвращает false.
     *
     * @param filePath Путь к файлу.
     * @param words Список слов для сохранения.
     * @return bool Успех операции.
     */
    static bool saveToFile(const QString &filePath, const QStringList &words)
    {
        if (filePath.isEmpty()) {
            qDebug() << "Empty file path";
            return false;
        }

        // Определение формата файла по расширению
        FileFormat format = detectFormat(filePath);

        // Сохранение в зависимости от формата
        switch (format) {
        case TXT:
            return saveToTxt(filePath, words);
        case JSON:
            return saveToJson(filePath, words);
        case CSV:
            return saveToCsv(filePath, words);
        }

        return false;
    }

    /**
     * @brief Определение формата файла по его расширению.
     *
     * Если расширение ".json", возвращает JSON.
     * Если расширение ".csv", возвращает CSV.
     * В остальных случаях — по умолчанию TXT.
     *
     * @param filePath Путь к файлу.
     * @return FileFormat Формат файла.
     */
    static FileFormat detectFormat(const QString &filePath)
    {
        QString ext = QFileInfo(filePath).suffix().toLower();
        if (ext == "json") return JSON;
        if (ext == "csv") return CSV;
        return TXT; // По умолчанию — текстовый формат
    }

    /**
     * @brief Загрузка списка слов из текстового файла.
     *
     * Поддерживает формат:
     * • Одно слово на строке
     * • Строки, разделённые запятыми (например: "cat,dog")
     * • Строки, начинающиеся с #, игнорируются (комментарии)
     *
     * @param filePath Путь к текстовому файлу.
     * @return QStringList Список загруженных слов.
     */
    static QStringList loadFromTxt(const QString &filePath)
    {
        QStringList words;
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qDebug() << "Cannot open file:" << filePath;
            return getDefaultWords();
        }

        QTextStream stream(&file);
        while (!stream.atEnd()) {
            QString line = stream.readLine().trimmed();
            if (line.isEmpty() || line.startsWith('#')) {
                continue;  // Игнорируем пустые строки и комментарии
            }
            if (line.contains(',')) {
                // Если строка содержит запятые, разбиваем на слова
                QStringList lineWords = line.split(',', QString::SkipEmptyParts);
                for (QString &word : lineWords) {
                    word = word.trimmed();
                    if (!word.isEmpty()) {
                        words.append(word);  // Добавляем слово
                    }
                }
            } else {
                words.append(line);  // Одно слово на строку
            }
        }
        file.close();
        return words;
    }

    /**
     * @brief Загрузка списка слов из файла формата JSON.
     *
     * Поддерживает два варианта структуры JSON:
     * 1. Простой массив строк: ["cat", "dog"]
     * 2. Объект с ключом "words": {"words": ["cat", "dog"]}
     *
     * @param filePath Путь к JSON-файлу.
     * @return QStringList Список загруженных слов.
     */
    static QStringList loadFromJson(const QString &filePath)
    {
        QStringList words;
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly)) {
            qDebug() << "Cannot open file:" << filePath;
            return getDefaultWords();
        }

        QByteArray data = file.readAll();
        file.close();

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
            for (const QJsonValue &value : array) {
                if (value.isString()) {
                    words.append(value.toString());  // Добавляем строку
                }
            }
        } else if (doc.isObject()) {
            QJsonObject obj = doc.object();
            if (obj.contains("words") && obj["words"].isArray()) {
                QJsonArray array = obj["words"].toArray();
                for (const QJsonValue &value : array) {
                    if (value.isString()) {
                        words.append(value.toString());
                    }
                }
            }
        }

        return words;
    }

    /**
     * @brief Загрузка списка слов из файла формата CSV.
     *
     * Поддерживает формат:
     * • Одно слово на строку
     * • Строки, начинающиеся с #, игнорируются (комментарии)
     *
     * @param filePath Путь к CSV-файлу.
     * @return QStringList Список загруженных слов.
     */
    static QStringList loadFromCsv(const QString &filePath)
    {
        QStringList words;
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qDebug() << "Cannot open file:" << filePath;
            return getDefaultWords();
        }

        QTextStream stream(&file);
        while (!stream.atEnd()) {
            QString line = stream.readLine().trimmed();
            if (line.isEmpty()) continue;

            // Пропуск комментариев (строки, начинающиеся с #)
            if (line.startsWith('#')) continue;

            // Разделение по запятой
            QStringList lineWords = line.split(',', QString::SkipEmptyParts);
            for (QString &word : lineWords) {
                word = word.trimmed();
                if (!word.isEmpty()) {
                    words.append(word);  // Добавляем слово
                }
            }
        }
        file.close();
        return words;
    }

    /**
     * @brief Сохранение списка слов в текстовом формате.
     *
     * Добавляет комментарии в начале файла:
     * • "# Word list for Word Race Game"
     * • "# Each word on a new line"
     * • "# Lines starting with # are ignored"
     *
     * @param filePath Путь к файлу.
     * @param words Список слов для сохранения.
     * @return bool Успех операции.
     */
    static bool saveToTxt(const QString &filePath, const QStringList &words)
    {
        QFile file(filePath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            qDebug() << "Cannot open file for writing:" << filePath;
            return false;
        }

        QTextStream stream(&file);
        // Заголовок с комментариями
        stream << "# Word list for Word Race Game\n";
        stream << "# Each word on a new line\n";
        stream << "# Lines starting with # are ignored\n\n";

        // Сохранение слов
        for (const QString &word : words) {
            if (!word.trimmed().isEmpty()) {
                stream << word.trimmed() << "\n";
            }
        }

        file.close();
        return true;
    }

    /**
     * @brief Сохранение списка слов в формате JSON.
     *
     * Создаёт простой массив строк:
     * ["cat", "dog", ...]
     *
     * @param filePath Путь к файлу.
     * @param words Список слов для сохранения.
     * @return bool Успех операции.
     */
    static bool saveToJson(const QString &filePath, const QStringList &words)
    {
        QFile file(filePath);
        if (!file.open(QIODevice::WriteOnly)) {
            qDebug() << "Cannot open file for writing:" << filePath;
            return false;
        }

        // Создание JSON-массива
        QJsonArray jsonArray;
        for (const QString &word : words) {
            jsonArray.append(word.trimmed());
        }

        QJsonDocument doc(jsonArray);
        file.write(doc.toJson());
        file.close();
        return true;
    }

    /**
     * @brief Сохранение списка слов в формате CSV.
     *
     * На самом деле, сохраняет в том же формате, что и TXT,
     * но с расширением .csv (для единообразия).
     *
     * @param filePath Путь к файлу.
     * @param words Список слов для сохранения.
     * @return bool Успех операции.
     */
    static bool saveToCsv(const QString &filePath, const QStringList &words)
    {
        QFile file(filePath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            qDebug() << "Cannot open file for writing:" << filePath;
            return false;
        }

        QTextStream stream(&file);
        // Заголовок с комментариями
        stream << "# Word list for Word Race Game\n";
        stream << "# Each word on a new line\n";
        stream << "# Lines starting with # are ignored\n\n";

        // Сохранение слов
        for (const QString &word : words) {
            if (!word.trimmed().isEmpty()) {
                stream << word.trimmed() << "\n";
            }
        }

        file.close();
        return true;
    }

    /**
     * @brief Возвращает путь к файлу слов по умолчанию.
     *
     * @return QString Путь к файлу "words.txt".
     */
    static QString getDefaultWordFilePath()
    {
        return "words.txt";
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
     */
    static QStringList getDefaultWords()
    {
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
     */
    static QStringList getRussianDefaultWords()
    {
        return QStringList()
            << "кот" << "дом" << "лес" << "река" << "солнце"
            << "машина" << "дружба" << "работа" << "успех"
            << "компьютер" << "программа" << "разработка"
            << "окно" << "стол" << "стул" << "книга";
    }
};

#endif // WORDLISTLOADER_H
