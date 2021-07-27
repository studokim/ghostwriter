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

#include "library.h"
#include "bookmark.h"

#define MAX_FILE_HISTORY_SIZE 20
#define FILE_HISTORY_KEY "FileHistory"
#define FAVORITES_KEY "Favorites"
#define FILE_PATH_KEY "filePath"
#define CURSOR_POSITION_KEY "cursorPosition"

namespace ghostwriter
{

class LibraryPrivate
{
public:
    LibraryPrivate() { }
    ~LibraryPrivate() { }

    QList<Bookmark> fetchLibraryList(const QString &key);

    void storeToSettings(const QString &key,
        const QList<Bookmark> &bookmarks);
    
    void addToLibraryList(const QString &key,
        const Bookmark &bookmark);
    
    void cleanUpHistory(QList<Bookmark> &recentFiles);
};

Library::Library()
    : d_ptr(new LibraryPrivate())
{
    ;
}

Library::~Library()
{
    ;
}

QList<Bookmark> Library::recentFiles() const
{
    Q_D(const Library);

    return d->fetchLibraryList(FILE_HISTORY_KEY);
}

void Library::addRecent
(
    const QString &filePath,
    int cursorPosition
)
{
    addRecent(Bookmark(filePath, cursorPosition));
}

void Library::addRecent(const Bookmark &bookmark)
{
    Q_D(Library);

    QList<Bookmark> recents =
        d->fetchLibraryList(FILE_HISTORY_KEY);

    recents.removeAll(bookmark);
    recents.prepend(bookmark);
    d->cleanUpHistory(recents);
    d->storeToSettings(FILE_HISTORY_KEY, recents);
}

QList<Bookmark> Library::favorites() const
{
    Q_D(const Library);

    return d->fetchLibraryList(FAVORITES_KEY);
}

void Library::addFavorite(const QString &filePath,
    int cursorPosition)
{
    addFavorite(Bookmark(filePath, cursorPosition));
}

void Library::addFavorite(const Bookmark &bookmark)
{
    Q_D(Library);

    QList<Bookmark> bookmarks = d->fetchLibraryList(FAVORITES_KEY);
    bookmarks.removeAll(bookmark);
    bookmarks.append(bookmark);
    d->storeToSettings(FAVORITES_KEY, bookmarks);
}

void Library::removeFavorite(const QString &filePath)
{
    removeFavorite(Bookmark(filePath));
}

void Library::removeFavorite(const Bookmark &bookmark)
{
    Q_D(Library);

    QList<Bookmark> bookmarks = this->favorites();
    bookmarks.removeAll(Bookmark(sanitizedPath));
    d->storeToSettings(FAVORITES_KEY, bookmarks);
}

void Library::clearRecent()
{
    QSettings settings;
    settings.remove(FILE_HISTORY_KEY);
}

void LibraryPrivate::storeToSettings(const QString &key,
    const QList<Bookmark> &bookmarks)
{
    QSettings settings;

    if (bookmarks.isEmpty()) {
        settings.remove(key);
        return;
    }

    settings.beginWriteArray(key, bookmarks.size());

    for (int i = 0; i < bookmarks.size(); i++) {
        Bookmark recentFile = bookmarks.at(i);

        settings.setArrayIndex(i);
        settings.setValue(FILE_PATH_KEY, recentFile.filePath());
        settings.setValue
        (
            CURSOR_POSITION_KEY,
            recentFile.position()
        );
    }

    settings.endArray();
}

QList<Bookmark> LibraryPrivate::fetchLibraryList(const QString &key)
{
    QSettings settings;
    int size = settings.beginReadArray(key);

    QList<Bookmark> bookmarks;

    for (int i = 0; i < size; i++) {
        settings.setArrayIndex(i);

        QString filePath = settings.value(FILE_PATH_KEY).toString();
        int position = settings.value(CURSOR_POSITION_KEY, 0).toInt();

        if (!filePath.isNull() && !filePath.isEmpty() && QFileInfo(filePath).exists()) {
            Bookmark bookmark(filePath, position);
            bookmarks.append(bookmark);
        }
    }

    settings.endArray();

    return bookmarks;
}

void LibraryPrivate::cleanUpHistory(QList<Bookmark> &recentFiles)
{
    while (recentFiles.size() > MAX_FILE_HISTORY_SIZE) {
        recentFiles.removeLast();
    }
}
}
