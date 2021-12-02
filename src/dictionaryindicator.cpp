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

#include <QListView>
#include <iostream>

#include "dictionary_manager.h"
#include "dictionaryindicator.h"

namespace ghostwriter
{
    DictionaryIndicator::DictionaryIndicator(QWidget *parent)
        : QComboBox(parent)
    {
        QStringList languages = DictionaryManager::instance().availableDictionaries();
        languages.sort();
        appSettings = AppSettings::instance();

        this->setView(new QListView());
        this->view()->setTextElideMode(Qt::ElideNone);
        this->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLength);

        int currentDictionaryIndex = 0;
        int maxLangLength = 0;

        for (int i = 0; i < languages.length(); i++)
        {
            QString language = languages[i];
            if (appSettings->dictionaryFavoriteLocales().contains(language))
            {
                QString langName = trimAfterFirstWhitespace(languageName(language));
                this->addItem(langName, language);
                this->setItemData(currentDictionaryIndex, Qt::AlignCenter, Qt::TextAlignmentRole);
                // this->setForegroundRole(QPalette::NoRole);
                // this->setBackgroundRole(QPalette::NoRole);

                if (appSettings->dictionaryLanguage() == language)
                {
                    this->setCurrentIndex(currentDictionaryIndex);
                }
                if (langName.length() > maxLangLength)
                    maxLangLength = langName.length();
                currentDictionaryIndex++;
            }
        }
        this->setMinimumContentsLength(maxLangLength);

        connect(
            this,
            static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            [this](int index)
            {
                QString language = this->itemData(index).toString();
                DictionaryManager::instance().setDefaultLanguage(language);
                appSettings->setDictionaryLanguage(language);
            });
    }

    DictionaryIndicator::~DictionaryIndicator()
    {
        ;
    }

    void DictionaryIndicator::showPopup()
    {
        int max = 0;

        for (int i = 0; i < this->count(); i++)
        {
            int itemWidth = this->fontMetrics().horizontalAdvance(this->itemText(i));

            if (itemWidth > max)
            {
                max = itemWidth;
            }
        }

        this->view()->setMinimumWidth(max + 20);
        QComboBox::showPopup();
    }

    QString DictionaryIndicator::trimAfterFirstWhitespace(QString languageName)
    {
        return languageName.split(' ')[0];
    }

    // Copied from preferencesdialog.cpp
    QString DictionaryIndicator::languageName(const QString &language)
    {
        QString lang_code = language.left(5);
        QLocale locale(lang_code);
        QString name;

        if (lang_code.length() > 2)
        {
            if (locale.name() == lang_code)
            {
                name = locale.nativeLanguageName() + " (" + locale.nativeCountryName() + ")";
            }
            else
            {
                name = locale.nativeLanguageName() + " (" + language + ")";
            }
        }
        else
        {
            name = locale.nativeLanguageName();
        }
        if (locale.textDirection() == Qt::RightToLeft)
        {
            name.prepend(QChar(0x202b));
        }

        return name;
    }
}
