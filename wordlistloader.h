#ifndef WORDLISTLOADER_H
#define WORDLISTLOADER_H

#include <QString>
#include <QStringList>
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QDebug>

/**
 * @brief Утилитарный класс для загрузки и сохранения списков слов
 * Содержит только статические методы, не требует создания экземпляра
 */
class WordListLoader
{
public:
    /**
     * @brief Загрузка слов из текстового файла
     * @param filePath - путь к файлу
     * @return QStringList - список загруженных слов
     *
     * Поддерживаемые форматы:
     * - Одно слово на строку
     * - Слова через запятую на одной строке (cat,dog,fish)
     * - Строки, начинающиеся с '#' - комментарии (игнорируются)
     *
     * При ошибке возвращает стандартный список слов
     */
    static QStringList loadFromFile(const QString &filePath)
    {
        QStringList words;

        // Проверка существования файла
        if (filePath.isEmpty() || !QFile::exists(filePath)) {
            qDebug() << "File not found or empty path:" << filePath;
            return getDefaultWords();
        }

        // Открытие файла
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qDebug() << "Cannot open file:" << filePath;
            return getDefaultWords();
        }

        // Чтение файла построчно
        QTextStream stream(&file);
        while (!stream.atEnd()) {
            QString line = stream.readLine().trimmed();

            // Пропуск пустых строк и комментариев
            if (line.isEmpty() || line.startsWith('#')) {
                continue;
            }

            // Обработка строки с запятыми
            if (line.contains(',')) {
                QStringList lineWords = line.split(',', QString::SkipEmptyParts);
                for (QString &word : lineWords) {
                    word = word.trimmed();
                    if (!word.isEmpty()) {
                        words.append(word);
                    }
                }
            } else {
                // Одиночное слово
                words.append(line);
            }
        }
        file.close();

        // Проверка, что слова были загружены
        if (words.isEmpty()) {
            qDebug() << "No words loaded, using defaults";
            return getDefaultWords();
        }

        return words;
    }

    /**
     * @brief Сохранение списка слов в файл
     * @param filePath - путь для сохранения
     * @param words - список слов
     * @return true - успешно, false - ошибка
     *
     * Сохраняет слова в формате "одно слово на строку"
     * Добавляет заголовок с комментариями
     */
    static bool saveToFile(const QString &filePath, const QStringList &words)
    {
        if (filePath.isEmpty()) {
            qDebug() << "Empty file path";
            return false;
        }

        // Открытие файла для записи
        QFile file(filePath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            qDebug() << "Cannot open file for writing:" << filePath;
            return false;
        }

        // Запись слов в файл
        QTextStream stream(&file);
        stream << "# Word list for Word Race Game\n";
        stream << "# Each word on a new line\n";
        stream << "# Lines starting with # are ignored\n\n";

        for (const QString &word : words) {
            if (!word.trimmed().isEmpty()) {
                stream << word.trimmed() << "\n";
            }
        }

        file.close();
        return true;
    }

    /**
     * @brief Получение пути к файлу со словами по умолчанию
     * @return "words.txt" в текущей директории
     */
    static QString getDefaultWordFilePath()
    {
        return "words.txt";
    }

    /**
     * @brief Стандартный английский словарь
     * @return список из 20 базовых английских слов
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
     * @brief Стандартный русский словарь
     * @return список базовых русских слов
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
