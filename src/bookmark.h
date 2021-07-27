/***********************************************************************
 *
 * Copyright (C) 2021 wereturtle
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

#ifndef BOOKMARK_H
#define BOOKMARK_H

#include <QObject>
#include <QString>
#include <QScopedPointer>

namespace ghostwriter
{
/**
 * Encapsulates the file path/cursor position as a pair.
 */
class BookmarkPrivate;
class Bookmark : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Bookmark)
    
public:
    /**
     * Constructor.
     */
    Bookmark(const QString &path, int position = 0);

    /**
     * Copy constructor.
     */
    Bookmark(const Bookmark &copy);

    /**
     * Destructor.
     */
    ~Bookmark();

    /**
     * Returns the file path of the document.
     */
    QString filePath() const;

    /**
     * Sets the file path of the document.
     */
    void setFilePath(const QString &path);

    /**
     * Returns the last-known cursor position for the given file path.  Value
     * will be set to 0 (beginning of the file) if the last cursor position
     * is unknown.
     */
    int position() const;

    /**
     * Sets the last-known cursor position for the given file path.
     */
    void setPosition(int value);

    /**
     * Comparison operator that compares two bookmark's file paths.
     */
    bool operator==(const Bookmark &other) const;

private:
    QScopedPointer<BookmarkPrivate> d_ptr;
};
}