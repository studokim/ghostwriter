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

#include <QObject>
#include <QHash>
#include <QString>
#include <QStringList>
#include <QDir>
#include <QFileInfo>

#include "bookmark.h"

#include "library.h"

namespace ghostwriter
{

class BookmarkPrivate
{
public:
    BookmarkPrivate() { }
    ~BookmarkPrivate() { }

    QString filePath;
    int position;
};

Bookmark::Bookmark(const QString &path, int position)
    : d_ptr(new BookmarkPrivate())
{
    setFilePath(path);
    setPosition(position);
}

Bookmark::Bookmark(const Bookmark &copy)
    : d_ptr(new BookmarkPrivate())
{
    Q_D(Bookmark);

    d->filePath = copy.filePath();
    d->position = copy.position();
}

Bookmark::~Bookmark()
{
    ;
}

QString Bookmark::filePath() const
{
    Q_D(const Bookmark);

    return d->filePath;
}

void Bookmark::setFilePath(const QString &path)
{
    Q_D(Bookmark);

    QFileInfo info(path);

    if (info.exists()) {
        d->filePath = info.canonicalFilePath();
    }
    else {
        d->filePath = path;
    }

    d->filePath = path;
}

int Bookmark::position() const
{
    Q_D(const Bookmark);

    return d->position;
}

void Bookmark::setPosition(int value)
{
    Q_D(Bookmark);

    d->position = value;
}

bool Bookmark::operator==(const Bookmark &other) const
{
    return (QFileInfo(other.filePath()) == QFileInfo(this->filePath()));
}
}
