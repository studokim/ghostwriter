/***********************************************************************
 *
 * Copyright (C) 2014-2020 wereturtle
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

#ifndef MARKUPDOCUMENT_H
#define MARKUPDOCUMENT_H

#include <QDateTime>
#include <QObject>
#include <QScopedPointer>
#include <QString>
#include <QTextBlock>
#include <QTextDocument>

#include "markdownast.h"

namespace ghostwriter
{
/**
 * Text document that maintains timestamp, read-only state, draft vs.
 * saved status, and Markdown AST.
 */
class MarkdownDocumentPrivate;
class MarkdownDocument : public QTextDocument
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(MarkdownDocument)

public:
    static void setDraftDirectory(const QString &path);

    /**
     * Constructor.
     */
    MarkdownDocument(QObject *parent = 0);

    /**
     * Constructor.
     */
    MarkdownDocument(const QString &text, QObject *parent = 0);

    /**
     * Destructor.
     */
    virtual ~MarkdownDocument();

    /**
     * Gets display name for the document for displaying in a text
     * editor's containing window or tab.
     */
    QString displayName() const;

    /**
     * Gets the document file path.
     */
    QString filePath() const;

    /**
     * Sets the document file path.
     */
    void setFilePath(const QString &path);

    /**
     * Returns true if the document is a new draft.
     */
    bool isDraft() const;

    /**
     * Returns true if the document has read only permissions.
     */
    bool isReadOnly() const;

    /**
     * Sets whether the document has read only permissions.
     */
    void setReadOnly(bool readOnly);

    /**
     * Gets the timestamp of the document, which is useful when comparing
     * the last modified time of the file represented on disk.
     */
    QDateTime timestamp() const;

    /**
     * Sets a timestamp to the document, which is useful when comparing
     * the last modified time of the file represented on disk.
     */
    void setTimestamp(const QDateTime &timestamp);

    MarkdownAST *markdownAST() const;
    void setMarkdownAST(MarkdownAST *ast);

    /**
     * For internal use only with TextBlockData class.  Emits signals
     * to notify listeners that the given text block is about to be
     * removed from the document.
     */
    void notifyTextBlockRemoved(const QTextBlock &block);

signals:
    /**
     * Emitted when the file name changes.
     */
    void fileNameChanged();

    /**
     * Emitted when the QTextBlock at the given position in the document
     * is removed.
     */
    void textBlockRemoved(int position);

    /**
     * Emitted when a QTextBlock is removed from the document.  Parameter
     * is the QTextBlock that is being removed.
     */
    void textBlockRemoved(const QTextBlock &block);

    /**
     * Emitted when the document's display name changes, which is useful
     * for updating the editor's containing window or tab to have the new
     * document display name.
     */
    void documentDisplayNameChanged(const QString &displayName);

    /**
     * Emitted when the document's modification state changes.  The
     * modified parameter will be true if the document has been modified.
     */
    void documentModifiedChanged(bool modified);

    /**
     * Emitted when a document is loaded from disk.
     */
    void documentLoaded();

    /**
     * Emitted when the document is closed.
     */
    void documentClosed();

public slots:

    /**
     * Sets whether auto-saving of the file is enabled.
     */
    void setAutoSaveEnabled(bool enabled);

    /**
     * Sets whether a backup file is created (with a .backup extension)
     * on disk before the document is saved.
     */
    void setFileBackupEnabled(bool enabled);

    /**
     * Reloads document from disk contents.  This method does nothing if
     * the document is new and is not associated with a file on disk.
     * Note that if the document is modified, this method will discard
     * changes before reloading.  It is left to the caller to check for
     * modification and save any changes before calling this method.
     */
    void reload();

    /**
     * Renames file represented by this document to the given file path.
     * This method does nothing if the document is new and is not
     * associated with a file on disk.
     */
    void rename();

    /**
     * Savse document contents to disk.  This method does nothing if the
     * document is new and is not associated with a file on disk.
     */
    bool save();

    /**
     * Prompts the user for a file path location, and saves the document
     * contents to the selected file path. This method is
     * also called if the document is new and is now going to be saved to
     * a file path for the first time.  Future save operations to the same
     * file path can be achieved by calling save() instead.
     */
    bool saveAs();

    /**
     * Closes the current file, clearing the editor of text and leaving
     * only an untitled "new" document in its place.  Note that isNew()
     * will return true after this method is called.
     */
    bool close();

    /**
     * Exports the current file, prompting the user for the desired
     * export format.
     */
    void exportFile();

private:
    QScopedPointer<MarkdownDocumentPrivate *> d_ptr;
};
} // namespace ghostwriter

#endif // MARKUPDOCUMENT_H
