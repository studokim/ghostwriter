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

#ifndef DOCUMENTHISTORY_H
#define DOCUMENTHISTORY_H

#include <QObject>
#include <QHash>
#include <QString>
#include <QStringList>

namespace ghostwriter
{

/**
 * This class stores and retrieves recent file history using QSettings.
 * It is reentrant, and different instances can be used from anywhere to
 * access the same file history.
 */
class Library
{
public:
    /*
     * Encapsulates the file path/cursor position as a pair.
     */
    class Bookmark
    {
    public:
        /**
         * Returns the file path of the document.
         */
        QString filePath;

        /**
         * The last-known cursor position for the given file path.  Value
         * will be set to 0 (beginning of the file) if the last cursor position
         * is unknown.
         */
        int position;

        /**
         * True if the document is new/untitled and therefore
         * cached to a temporary file location specified by filePath().
         */
        bool untitled;

        inline bool operator==(const Bookmark &other)
        {
            return (other.filePath == filePath);
        }
    };

    /**
     * Constructor.
     */
    Library(const QString& path = QString());

    /**
     * Destructor.
     */
    ~Library();

    /**
     * Returns the list of recent files, up to the maximum number specified.
     * Specify a value of -1 to get the entire history.
     */
    QList<Bookmark> recentFiles(int max = -1);

    /**
     * Adds the given file path and cursor position to the history.
     */
    void addRecent(const QString &filePath,
        int cursorPosition,
        bool untitled = false);

    void favorite(const QString &filePath,
        int cursorPosition);

    /**
     * Wipes the document history clean.
     */
    void clearRecent();

signals:
    /**
     * Emitted when a recent file is added/removed from the history.
     */
    void recentFilesChanged();
};
}

#endif // DOCUMENTHISTORY_H
