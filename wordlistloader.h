#ifndef WORDLISTLOADER_H
#define WORDLISTLOADER_H

#include <QString>
#include <QStringList>
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QDebug>

class WordListLoader
{
public:
    static QStringList loadFromFile(const QString &filePath)
    {
        QStringList words;

        if (filePath.isEmpty() || !QFile::exists(filePath)) {
            qDebug() << "File not found or empty path:" << filePath;
            return getDefaultWords();
        }

        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qDebug() << "Cannot open file:" << filePath;
            return getDefaultWords();
        }

        QTextStream stream(&file);
        while (!stream.atEnd()) {
            QString line = stream.readLine().trimmed();

            // Пропускаем пустые строки и комментарии
            if (line.isEmpty() || line.startsWith('#')) {
                continue;
            }

            // Пробуем разделить по запятым
            if (line.contains(',')) {
                QStringList lineWords = line.split(',', QString::SkipEmptyParts);
                for (QString &word : lineWords) {
                    word = word.trimmed();
                    if (!word.isEmpty()) {
                        words.append(word);
                    }
                }
            } else {
                // Одно слово на строку
                words.append(line);
            }
        }
        file.close();

        if (words.isEmpty()) {
            qDebug() << "No words loaded, using defaults";
            return getDefaultWords();
        }

        return words;
    }

    static QStringList getDefaultWords()
    {
        return QStringList()
            << "cat" << "dog" << "sun" << "moon" << "star"
            << "tree" << "fish" << "bird" << "frog" << "bear"
            << "apple" << "house" << "happy" << "friend" << "computer"
            << "keyboard" << "mouse" << "screen" << "developer" << "program";
    }

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
