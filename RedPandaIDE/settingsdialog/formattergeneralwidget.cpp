/*
 * Copyright (C) 2020-2022 Roy Qu (royqh1979@gmail.com)
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
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "formattergeneralwidget.h"
#include "ui_formattergeneralwidget.h"
#include "../settings.h"

FormatterGeneralWidget::FormatterGeneralWidget(const QString& name, const QString& group, QWidget *parent):
    SettingsWidget(name,group,parent),
    ui(new Ui::FormatterGeneralWidget)
{
    ui->setupUi(this);
    ui->cbBraceStyle->setModel(&mStylesModel);
    connect(ui->cbBraceStyle, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &FormatterGeneralWidget::onBraceStyleChanged);
    ui->editDemo->setReadOnly(true);
    connect(this, &SettingsWidget::settingsChanged,
               this, &FormatterGeneralWidget::updateDemo);

    ui->cbMinConditionalIndent->addItem(tr("No minimal indent"),0);
    ui->cbMinConditionalIndent->addItem(tr("Indent at least one additional indent"),1);
    ui->cbMinConditionalIndent->addItem(tr("Indent at least two additional indents"),2);
    ui->cbMinConditionalIndent->addItem(tr("Indent at least one-half an additional indent."),3);
}

FormatterGeneralWidget::~FormatterGeneralWidget()
{
    delete ui;
}

void FormatterGeneralWidget::onBraceStyleChanged()
{
    PFormatterStyleItem item = mStylesModel.getStyle(ui->cbBraceStyle->currentIndex());
    if (item) {
        ui->lblBraceStyle->setText(item->description);
    }
}

void FormatterGeneralWidget::doLoad()
{
    Settings::CodeFormatter& format = pSettings->codeFormatter();
    for (int i=0;i<mStylesModel.rowCount(QModelIndex());i++) {
        PFormatterStyleItem item = mStylesModel.getStyle(i);
        if (item->name == format.baseStyle()) {
            ui->cbBraceStyle->setCurrentIndex(i);
            break;
        }
    }
    ui->spinTabSize->setValue(format.tabWidth());
    updateDemo();
}

void FormatterGeneralWidget::doSave()
{
    Settings::CodeFormatter& format = pSettings->codeFormatter();
    updateCodeFormatter(format);
    format.save();
}

FormatterStyleModel::FormatterStyleModel(QObject *parent):QAbstractListModel(parent)
{
    mStyles.append(
                std::make_shared<FormatterStyleItem>(
                    tr("LLVM"),
                    tr("A style complying with the LLVM coding standards."))
                );
    mStyles.append(
                std::make_shared<FormatterStyleItem>(
                    tr("Google"),
                    tr("A style complying with Google’s C++ style guide."))
                );
    mStyles.append(
                std::make_shared<FormatterStyleItem>(
                    tr("Chromium"),
                    tr("A style complying with Chromium’s style guide."))
                );
    mStyles.append(
                std::make_shared<FormatterStyleItem>(
                    tr("Mozilla"),
                    tr("A style complying with Mozilla’s style guide."))
                );
    mStyles.append(
                std::make_shared<FormatterStyleItem>(
                    tr("WebKit"),
                    tr("A style complying with WebKit’s style guide."))
                );
    mStyles.append(
                std::make_shared<FormatterStyleItem>(
                    tr("Microsoft"),
                    tr("A style complying with Microsoft’s style guide."))
                );
    mStyles.append(
                std::make_shared<FormatterStyleItem>(
                    tr("GNU"),
                    tr("A style complying with the GNU coding standards."))
                );
}

int FormatterStyleModel::rowCount(const QModelIndex &) const
{
    return mStyles.count();
}

QVariant FormatterStyleModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    int row = index.row();
    if (row<0 || row>=mStyles.count())
        return QVariant();
    PFormatterStyleItem item = mStyles[row];
    switch (role) {
    case Qt::DisplayRole:
        return item->name;
    case Qt::ToolTipRole:
        return item->description;
    }
    return QVariant();
}

PFormatterStyleItem FormatterStyleModel::getStyle(const QModelIndex &index)
{
    if (index.isValid()) {
        return getStyle(index.row());
    } else {
        return PFormatterStyleItem();
    }
}

PFormatterStyleItem FormatterStyleModel::getStyle(int index)
{
    if (index<0 || index>=mStyles.count())
        return PFormatterStyleItem();
    return mStyles[index];
}

FormatterStyleItem::FormatterStyleItem(const QString &name, const QString &description)
{
    this->name = name;
    this->description = description;
}

void FormatterGeneralWidget::on_chkBreakMaxCodeLength_stateChanged(int)
{
    ui->spinMaxCodeLength->setEnabled(ui->chkBreakMaxCodeLength->isChecked());
    ui->chkBreakAfterLogical->setEnabled(ui->chkBreakMaxCodeLength->isChecked());
}

void FormatterGeneralWidget::updateDemo()
{
    QFile file(":/codes/formatdemo.cpp");
    if (!file.open(QFile::ReadOnly))
        return;
    QByteArray content = file.readAll();

    Settings::CodeFormatter formatter(nullptr);
    updateCodeFormatter(formatter);

#ifdef Q_OS_WIN
    QByteArray newContent = runAndGetOutput("clang-format.exe",
                                            pSettings->dirs().appDir(),
                                            formatter.getArguments(),
                                            content);
#else
    QByteArray newContent = runAndGetOutput(pSettings->environment().formatterPath(),
                                            extractFileDir(pSettings->environment().AStylePath()),
                                            formatter.getArguments(),
                                            content);
#endif
    ui->editDemo->document()->setText(newContent);
}

void FormatterGeneralWidget::updateCodeFormatter(Settings::CodeFormatter &format)
{
    PFormatterStyleItem item = mStylesModel.getStyle(ui->cbBraceStyle->currentIndex());
    if (item)
        format.setBaseStyle(item->name);
    format.setTabWidth(ui->spinTabSize->value());
}

