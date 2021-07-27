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

#ifndef LIBRARY_H
#define LIBRARY_H

#include <QList>
#include <QObject>
#include <QScopedPointer>
#include <QString>

#include "bookmark.h"

namespace ghostwriter
{
/**
 * This class stores and retrieves recent file history using QSettings,
 * as well as tracks the list of favorite documents with their last
 * known cursor positions.
 *
 * It is reentrant, and different instances can be used from anywhere to
 * access the same data.
 */
class LibraryPrivate;
class Library
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Library)

public:
    /**
     * Constructor.
     */
    Library();

    /**
     * Destructor.
     */
    ~Library();

    /**
     * Returns the list of recent files.
     */
    QList<Bookmark> recentFiles();

    /**
     * Adds the given file path and cursor position to top of the history.
     */
    void addRecent(const QString &filePath,
        int cursorPosition);

    /**
     * Adds the given bookmark to the top of the history.
     */
    void addRecent(const Bookmark &bookmark);

    /**
     * Returns the list of favorite files as bookmarks.
     */
    QList<Bookmark> favorites() const;

    /**
     * Adds the given file path and cursor position to the favorites list.
     */
    void addFavorite(const QString &filePath,
        int cursorPosition);

    /**
     * Adds the given bookmark to the favorites list.
     */
    void addFavorite(const Bookmark &bookmark);

    /**
     * Removes the given file path from the favorites list.
     */
    void removeFavorite(const QString &filePath);

    /**
     * Removes the given bookmark from the favorites list.
     */
    void removeFavorite(const Bookmark &bookmark);

    /**
     * Wipes the history clean.
     */
    void clearRecent();

signals:
    /**
     * Emitted when a recent file is added/removed from the history.
     */
    void recentFilesChanged();

    /**
     * Emitted when a file is added/removed from the favorites list.
     */
    void favoritesChanged();

private:
    QScopedPointer<LibraryPrivate> d_ptr;
};
}

#endif // LIBRARY_H
