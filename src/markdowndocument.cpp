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

#include <QApplication>
#include <QString>
#include <QTextDocument>
#include <QPlainTextDocumentLayout>
#include <QFileInfo>
#include <QDir>

#include "markdowndocument.h"

namespace ghostwriter
{

class MarkdownDocumentPrivate
{
    Q_DECLARE_PUBLIC(MarkdownDocument)

public:

    static QString draftDirectory;

    MarkdownDocumentPrivate(MarkdownDocument *document)
        : q_ptr(document), ast(nullptr) { }
    ~MarkdownDocumentPrivate() { }

    /*
    * Initializes the class for an untitled document.
    */
    void initializeDraft();

    /*
    * Begins asynchronous save operation.  Called by save() and saveAs().
    */
    void saveFile();

    void onSaveCompleted();

    void onFileChangedExternally(const QString &path);

    /*
    * Loads the document with the file contents at the given path.
    */
    bool loadFile(const QString &filePath);

    /*
    * Sets the file path for the document, such that the file will be
    * monitored for external changes made to it, and the display name
    * for the document updated.
    */
    void setFilePath(const QString &filePath);

    /*
    * Checks if changes need to be saved before an operation
    * can continue.  The user will be prompted to save if
    * necessary, and this method will return true if the
    * operation can proceed.
    */
    bool checkSaveChanges();

    /*
    * Checks if file on the disk is read only.  This method will return
    * true if save operation can proceed.
    */
    bool checkPermissionsBeforeSave();

    /*
    * Saves the given text to the given file path, returning a null
    * string if successful, otherwise an error message.  Note that this
    * method is intended to be run in a separate thread from the main
    * Qt event loop, and should thus never interact with any widgets.
    */
    QString saveToDisk
    (
        const QString &filePath,
        const QString &text,
        bool createBackup
    ) const;

    /*
    * Creates a backup file with a ".backup" extension of the file having
    * the specified path.  Note that this method is intended to be run in
    * a separate thread from the main Qt event loop, and should thus never
    * interact with any widgets.
    */
    void backupFile(const QString &filePath) const;

    void autoSaveFile();

    MarkdownDocument *q_ptr;
    QString displayName;
    QString filePath;
    bool readOnlyFlag;
    QDateTime timestamp;
    MarkdownAST *ast;
    bool draft;

    /*
    * This flag is used to prevent notifying the user that the document
    * was modified when the user is the one who modified it by saving.
    * See code for onFileChangedExternally() for details.
    */
    bool saveInProgress;

    /*
    * This timer's timeout signal is connected to the autoSaveFile() slot,
    * which saves the document if it can be saved and has been modified.
    */
    QTimer *autoSaveTimer;
    bool autoSaveEnabled;

    /*
    * Boolean flag used to track if the prompt for the file having been
    * externally modified is already displayed and should not be displayed
    * again.
    */
    bool documentModifiedNotifVisible;
    
    QFutureWatcher<QString> *saveFutureWatcher;
    QFileSystemWatcher *fileWatcher;
    bool createBackupOnSave;
};

void MarkdownDocument::setDraftDirectory(const QString &path)
{
    QDir dir(path);

    if (!dir.exists(path)) {
        if (!dir.mkdir(path)) {
            MessageBoxHelper::critical(nullptr,
                QObject::tr("Invalid draft recovery directory"),
                QObject::tr("Failed to create draft recovery directory: %1\n"
                    "Drafts will be autosaved to %2.").arg(path).arg(QDir::tempPath()),
            );

            MarkdownDocumentPrivate::draftDirectory = QDir::tempPath();
        };
    }
    else {
        MarkdownDocumentPrivate::draftDirectory = path;
    }
}

MarkdownDocument::MarkdownDocument(QObject *parent)
    : QTextDocument(parent), d_ptr(new MarkdownDocumentPrivate(this))
{
    Q_D(MarkdownDocument);

    d->initializeDraft();
}

MarkdownDocument::MarkdownDocument(const QString &text, QObject *parent)
    : QTextDocument(text, parent), d_ptr(new MarkdownDocumentPrivate(this))
{
    Q_D(MarkdownDocument);

    d->initializeDraft();
}

MarkdownDocument::~MarkdownDocument()
{
    Q_D(MarkdownDocument);

    // QPlainTextDocumentLayout *documentLayout =
    //     new QPlainTextDocumentLayout(this);
    // this->setDocumentLayout(documentLayout);

    if (nullptr != ast) {
        delete ast;
        ast = nullptr;
    }
}

QString MarkdownDocument::displayName() const
{
    Q_D(const MarkdownDocument);
    
    return d->displayName;
}

QString MarkdownDocument::name() const
{
    Q_D(const MarkdownDocument);
    
    return d->filePath;
}

void MarkdownDocument::setName(const QString &path)
{
    Q_D(MarkdownDocument);
    
    if (!path.isNull() && !path.isEmpty()) {
        QFileInfo fileInfo(path);
        d->filePath = fileInfo.absoluteFilePath();
        d->displayName = fileInfo.fileName();
    } else {
        d->filePath = QString();
        d->draft = true;
        this->setReadOnly(false);
        this->setModified(false);
        d->displayName = tr("untitled");
    }

    emit filePathChanged();
}

bool MarkdownDocument::isDraft() const
{
    Q_D(const MarkdownDocument);

    return d->draft;
}

bool MarkdownDocument::isReadOnly() const
{
    Q_D(const MarkdownDocument);
    
    return d->readOnlyFlag;
}

void MarkdownDocument::setReadOnly(bool readOnly)
{
    Q_D(MarkdownDocument);
    
    d->readOnlyFlag = readOnly;
}

QDateTime MarkdownDocument::timestamp() const
{
    Q_D(const MarkdownDocument);
    
    return d->timestamp;
}

void MarkdownDocument::setTimestamp(const QDateTime &timestamp)
{
    Q_D(MarkdownDocument);
    
    this->d->timestamp = timestamp;
}


MarkdownAST *MarkdownDocument::markdownAST() const
{
    Q_D(const MarkdownDocument);
    
    return d->ast;
}

void MarkdownDocument::setMarkdownAST(MarkdownAST *ast)
{
    Q_D(MarkdownDocument);
    
    this->ast = ast;
}

void MarkdownDocument::notifyTextBlockRemoved(const QTextBlock &block)
{
    emit textBlockRemoved(block.position());
    emit textBlockRemoved(block);
}

void MarkdownDocumentPrivate::initializeDraft()
{
    Q_Q(MarkdownDocument);

    if (MarkdownDocumentPrivate::draftDirectory.isNull() ||
            MarkdownDocumentPrivate::draftDirectory.isEmpty() ||
            !QDir(MarkdownDocumentPrivate::draftDirectory).exists()) {
        MarkdownDocumentPrivate::draftDirectory = QDir::tempPath();
    }

    QPlainTextDocumentLayout *documentLayout =
        new QPlainTextDocumentLayout(this);
    q->setDocumentLayout(documentLayout);

    filePath = QString();
    draft = true;
    readOnlyFlag = false;
    displayName = MarkdownDocument::tr("untitled");
    timestamp = QDateTime::currentDateTime();

    if (autoSaveEnabled) {
        filePath = draftDirectory + "/" +
            MarkdownDocument::tr("UNTITLED %1").arg(timestamp.toString("YYYY-MM-DD HH.mm.ss"));
    }
}

void MarkdownDocumentPrivate::onSaveCompleted()
{
    Q_Q(MarkdownDocument);

    QString err = this->saveFutureWatcher->result();

    if (!err.isNull() && !err.isEmpty()) {
        MessageBoxHelper::critical
        (
            QApplication::activeWindow(),
            QObject::tr("Error saving %1").arg(q->filePath()),
            err
        );
    } else if (!this->fileWatcher->files().contains(q->filePath())) {
        fileWatcher->addPath(q->filePath());
    }

    q->setTimestamp(QDateTime::currentDateTime());
    this->saveInProgress = false;
}

void MarkdownDocumentPrivate::onFileChangedExternally(const QString &path)
{
    Q_Q(MarkdownDocument);

    QFileInfo fileInfo(path);

    if (!fileInfo.exists()) {
        emit q->documentModifiedChanged(true);

        // Make sure autosave knows the document is modified so it can
        // save it.
        //
        q->setModified(true);
    } else {
        if (fileInfo.isWritable() && q->isReadOnly()) {
            q->setReadOnly(false);

            if (autoSaveEnabled) {
                emit q->documentModifiedChanged(false);
            }
        } else if (!fileInfo.isWritable() && !q->isReadOnly()) {
            q->setReadOnly(true);

            if (q->isModified()) {
                emit q->documentModifiedChanged(true);
            }
        }

        // Need to guard against the QFileSystemWatcher from signalling a
        // file change when we're the one who changed the file by saving.
        // Thus, check the saveInProgress flag before prompting.
        //
        if
        (
            !saveInProgress &&
            (fileInfo.lastModified() > q->timestamp()) &&
            !documentModifiedNotifVisible
        ) {
            documentModifiedNotifVisible = true;

            int response =
                MessageBoxHelper::question
                (
                    editor,
                    QObject::tr("The document has been modified by another program."),
                    QObject::tr("Would you like to reload the document?"),
                    QMessageBox::Yes | QMessageBox::No,
                    QMessageBox::Yes
                );

            documentModifiedNotifVisible = false;

            if (QMessageBox::Yes == response) {
                q->reload();
            }
        }
    }
}

void MarkdownDocumentPrivate::autoSaveFile()
{
    Q_Q(MarkdownDocument);

    if (this->autoSaveEnabled &&
            !q->isReadOnly() &&
            q->isModified()) {
        q->save();
    }
}

void MarkdownDocumentPrivate::saveFile()
{
    Q_Q(MarkdownDocument);

    q->setModified(false);
    emit q->documentModifiedChanged(false);

    if (this->saveFutureWatcher->isRunning() ||
            this->saveFutureWatcher->isStarted()) {
        this->saveFutureWatcher->waitForFinished();
    }

    saveInProgress = true;

    if (fileWatcher->files().contains(document->name())) {
        this->fileWatcher->removePath(document->name());
    }

    document->setTimestamp(QDateTime::currentDateTime());

    QFuture<QString> future =
        QtConcurrent::run
        (
            this,
            &MarkdownDocumentPrivate::saveToDisk,
            document->name(),
            document->toPlainText(),
            createBackupOnSave
        );

    this->saveFutureWatcher->setFuture(future);
}

bool MarkdownDocumentPrivate::loadFile(const QString &filePath)
{
    Q_Q(MarkdownDocument);

    QFileInfo fileInfo(filePath);
    QFile inputFile(filePath);

    if (!inputFile.open(QIODevice::ReadOnly)) {
        MessageBoxHelper::critical
        (
            editor,
            QObject::tr("Could not read %1").arg(filePath),
            inputFile.errorString()
        );
        return false;
    }

    // NOTE: Must set editor's text cursor to the beginning
    // of the document before clearing the document/editor
    // of text to prevent a crash in Qt 5.10 on opening or
    // reloading a file if a file has already been previously
    // opened in the editor.
    //
    QTextCursor cursor(document);
    cursor.setPosition(0);
    editor->setTextCursor(cursor);

    document->clearUndoRedoStacks();
    document->setUndoRedoEnabled(false);
    document->setPlainText("");

    QApplication::setOverrideCursor(Qt::WaitCursor);
    emit q->operationStarted(QObject::tr("opening %1").arg(filePath));
    QTextStream inStream(&inputFile);

    // Markdown files need to be in UTF-8 format, so assume that is
    // what the user is opening by default.  Enable autodection
    // of of UTF-16 or UTF-32 BOM in case the file isn't UTF-8 encoded.
    //
    inStream.setCodec("UTF-8");
    inStream.setAutoDetectUnicode(true);

    QString text = inStream.readAll();

    editor->setPlainText(text);
    editor->navigateDocument(0);
    emit q->operationUpdate();

    document->setUndoRedoEnabled(true);

    if (QFile::NoError != inputFile.error()) {
        MessageBoxHelper::critical
        (
            editor,
            QObject::tr("Could not read %1").arg(filePath),
            inputFile.errorString()
        );

        inputFile.close();
        return false;
    }

    inputFile.close();

    if (fileHistoryEnabled) {
        Library library;
        Bookmark bookmark = library.search(filePath);
        editor->navigateDocument(bookmark.position());
    } else {
        editor->navigateDocument(0);
    }

    setFilePath(filePath);
    editor->setReadOnly(false);

    if (!fileInfo.isWritable()) {
        document->setReadOnly(true);
    } else {
        document->setReadOnly(false);
    }

    document->setModified(false);
    document->setTimestamp(fileInfo.lastModified());

    QString watchedFile;

    foreach (watchedFile, fileWatcher->files()) {
        fileWatcher->removePath(watchedFile);
    }

    fileWatcher->addPath(filePath);
    emit q->operationFinished();
    emit q->documentModifiedChanged(false);
    QApplication::restoreOverrideCursor();

    editor->centerCursor();
    emit q->documentLoaded();

    return true;
}

void MarkdownDocumentPrivate::setFilePath(const QString &filePath)
{
    Q_Q(MarkdownDocument);

    if (!document->isDraft() || autoSaveEnabled) {
        fileWatcher->removePath(document->name());
    }

    document->setName(filePath);

    if (!filePath.isNull() && !filePath.isEmpty()) {
        QFileInfo fileInfo(filePath);

        if (fileInfo.exists()) {
            document->setReadOnly(!fileInfo.isWritable());
        } else {
            document->setReadOnly(false);
        }
    } else {
        document->setReadOnly(false);
    }

    emit q->documentDisplayNameChanged(document->displayName());
}

bool MarkdownDocumentPrivate::checkSaveChanges()
{
    Q_Q(MarkdownDocument);

    if (document->isModified()) {
        if (autoSaveEnabled && !document->isReadOnly()) {
            return q->save();
        } else {
            // Prompt user if he wants to save changes.
            QString text;

            if (document->isDraft()) {
                text = QObject::tr("File has been modified.");
            } else {
                text = (QObject::tr("%1 has been modified.")
                        .arg(document->displayName()));
            }

            int response =
                MessageBoxHelper::question
                (
                    editor,
                    text,
                    QObject::tr("Would you like to save your changes?"),
                    QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
                    QMessageBox::Save
                );

            switch (response) {
            case QMessageBox::Save:
                if (document->isDraft()) {
                    return q->saveAs();
                } else {
                    return q->save();
                }
                break;
            case QMessageBox::Cancel:
                return false;
            default:
                break;
            }
        }
    }

    return true;
}

bool MarkdownDocumentPrivate::checkPermissionsBeforeSave()
{
    if (document->isReadOnly()) {
        int response =
            MessageBoxHelper::question
            (
                editor,
                QObject::tr("%1 is read only.").arg(document->name()),
                QObject::tr("Overwrite protected file?"),
                QMessageBox::Yes | QMessageBox::No,
                QMessageBox::Yes
            );

        if (QMessageBox::No == response) {
            return false;
        } else {
            QFile file(document->name());
            fileWatcher->removePath(document->name());

            if (!file.remove()) {
                if (file.setPermissions(QFile::WriteUser | QFile::ReadUser) && file.remove()) {
                    document->setReadOnly(false);
                    return true;
                } else {
                    MessageBoxHelper::critical
                    (
                        editor,
                        QObject::tr("Overwrite failed."),
                        QObject::tr("Please save file to another location.")
                    );

                    fileWatcher->addPath(document->name());
                    return false;
                }
            } else {
                document->setReadOnly(false);
            }
        }
    }

    return true;
}

QString MarkdownDocumentPrivate::saveToDisk
(
    const QString &filePath,
    const QString &text,
    bool createBackup
) const
{
    QString err;

    if (filePath.isNull() || filePath.isEmpty()) {
        return QObject::tr("Null or empty file path provided for writing.");
    }

    QFile outputFile(filePath);

    if (createBackup && outputFile.exists()) {
        backupFile(filePath);
    }

    if (!outputFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        return outputFile.errorString();
    }

    // Write contents to disk.
    QTextStream outStream(&outputFile);

    // Markdown files need to be in UTF-8, since most Markdown processors
    // (i.e., Pandoc, et. al.) can only read UTF-8 encoded text files.
    //
    outStream.setCodec("UTF-8");
    outStream << text;

    if (QFile::NoError != outputFile.error()) {
        err = outputFile.errorString();
    }

    // Close the file.  All done!
    outputFile.close();
    return err;
}

void MarkdownDocumentPrivate::backupFile(const QString &filePath) const
{
    QString backupFilePath = filePath + ".backup";
    QFile backupFile(backupFilePath);

    if (backupFile.exists()) {
        if (!backupFile.remove()) {
            qCritical("Could not remove backup file %s before saving: %s",
                      backupFilePath.toLatin1().data(),
                      backupFile.errorString().toLatin1().data());
            return;
        }
    }

    QFile file(filePath);

    if (!file.copy(backupFilePath)) {
        qCritical("Failed to backup file to %s: %s",
                  backupFilePath.toLatin1().data(),
                  file.errorString().toLatin1().data());
    }
}
} // namespace ghostwriter
