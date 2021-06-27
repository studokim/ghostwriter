/***********************************************************************
 *
 * Copyright (C) 2014-2021 wereturtle
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 ***********************************************************************/

#include <QString>
#include <QStringList>
#include <QList>
#include <QFileInfo>
#include <QSettings>

#include "documenthistory.h"

#define MAX_FILE_HISTORY_SIZE 20
#define FILE_HISTORY_KEY "FileHistory"
#define FILE_PATH_KEY "filePath"
#define CURSOR_POSITION_KEY "cursorPosition"
#define UNTITLED_KEY "untitled"

namespace ghostwriter
{

static void
storeToSettings(const QList<Library::Bookmark> &recentFiles);

static void
cleanUpHistory(QList<Library::Bookmark> &recentFiles);

Library::Library()
{
    ;
}

Library::~Library()
{
    ;
}

QList<Library::Bookmark>
Library::recentFiles(int max)
{
    QSettings settings;
    int size = settings.beginReadArray(FILE_HISTORY_KEY);

    RecentFilesList recentFiles;

    if (max < 0) {
        max = size;
    }

    int filesAdded = 0;

    for (int i = 0; (i < size) && (filesAdded < max); i++) {
        settings.setArrayIndex(i);

        QString filePath = settings.value(FILE_PATH_KEY).toString();
        int position = settings.value(CURSOR_POSITION_KEY, 0).toInt();
        bool untitled = settings.value(UNTITLED_KEY, false).toBool();

        if (!filePath.isNull() && !filePath.isEmpty() && QFileInfo(filePath).exists()) {
            RecentFile recentFile;
            recentFile.filePath = filePath;
            recentFile.position = position;
            recentFile.untitled = untitled;
            recentFiles.append(recentFile);
            filesAdded++;
        }
    }

    settings.endArray();

    return recentFiles;
}

void Library::add
(
    const QString &filePath,
    int cursorPosition
)
{
    QFileInfo fileInfo(filePath);

    if (fileInfo.exists()) {
        QString sanitizedPath = fileInfo.canonicalFilePath();
        RecentFilesList recentFiles = recentFiles();
        RecentFile lastFile;

        lastFile.filePath = sanitizedPath;
        lastFile.position = cursorPosition;
        recentFiles.removeAll(lastFile);
        recentFiles.prepend(lastFile);
        cleanUpHistory(recentFiles);
        storeToSettings(recentFiles);
    }
}

// int Library::cursorPosition(const QString &filePath)
// {
//     QString sanitizedPath = QFileInfo(filePath).canonicalFilePath();
//     int position = 0;

//     RecentFilesList recentFiles = recentFiles();
//     cleanUpHistory(recentFiles);

//     foreach (RecentFile file, recentFiles) {
//         if (sanitizedPath == file.filePath) {
//             position = file.position;
//             break;
//         }
//     }

//     if (position < 0) {
//         position = 0;
//     }

//     return position;
// }

void Library::clearRecent()
{
    QSettings settings;
    settings.remove(FILE_HISTORY_KEY);
}

static void
storeToSettings(const QList<Library::Bookmark> &recentFiles)
{
    QSettings settings;

    if (recentFiles.isEmpty()) {
        settings.remove(FILE_HISTORY_KEY);
        return;
    }

    settings.beginWriteArray(FILE_HISTORY_KEY, recentFiles.size());

    for (int i = 0; i < recentFiles.size(); i++) {
        RecentFile recentFile = recentFiles.at(i);

        settings.setArrayIndex(i);
        settings.setValue(FILE_PATH_KEY, recentFile.filePath);
        settings.setValue
        (
            CURSOR_POSITION_KEY,
            recentFile.position
        );
    }

    settings.endArray();
}

static void cleanUpHistory(QList<Library::Bookmark> &recentFiles)
{
    while (recentFiles.size() > MAX_FILE_HISTORY_SIZE) {
        recentFiles.removeLast();
    }
}
}