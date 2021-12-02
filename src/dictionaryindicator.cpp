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

#include "dictionaryindicator.h"

namespace ghostwriter
{
    static QString wpmText(int value) { return QString("%Ln wpm"); }

    DictionaryIndicator::DictionaryIndicator(DocumentStatistics *documentStats,
                                             SessionStatistics *sessionStats, QWidget *parent)
        : QComboBox(parent)
    {
        this->setView(new QListView());
        this->view()->setTextElideMode(Qt::ElideNone);
        this->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLength);

        connect(this,
                &QComboBox::currentTextChanged,
                [this](QString text)
                {
                    int max = this->fontMetrics().averageCharWidth() * (text.length() + 2);
                    this->setMaximumWidth(max + 20);
                    this->setMinimumContentsLength(text.length());
                });

        int index = 0;

        this->addItem(wpmText(0));
        this->setItemData(index, Qt::AlignCenter, Qt::TextAlignmentRole);
        this->connect(documentStats,
                      &DocumentStatistics::wordCountChanged,
                      this,
                      [this, index](int value)
                      {
                          this->setItemText(index,
                                            wpmText(value));

                          if (index == this->currentIndex())
                          {
                              this->setMinimumContentsLength(this->itemText(index).length());
                          }
                      });
        // index++;
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

}
