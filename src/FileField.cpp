// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright © 2009-2018 ANSSI. All Rights Reserved.
/**
 * @file FileField.cpp
 * cryptclt generic file selector implementation.
 * @author Olivier Levillain <clipos@ssi.gouv.fr>
 * @author Vincent Strubel <clipos@ssi.gouv.fr>
 *
 * Copyright (C) 2009 SGDN
 * Copyright (C) 2010-2011 ANSSI
 * @n
 * All rights reserved.
 */

#include "FileField.h"
#include "SingleFileField.h"
#include "SinglePrivateKeyFileField.h"
#include "MultipleFileField.h"
#include "PubKeysFileField.h"
#include "common.h"

#include <QLabel>
#include <QPushButton>
#include <QFileDialog>
#include <QLayout>
#include <QTreeWidget>


FileField::FileField (QWidget* parent, const char* label, const char* desc,
          const char* dialogTitle, const char* filter, file_field_t t)
  : QFrame (parent)
{
  QLabel* lbl;
  QPushButton* btnAddFiles;
  QPushButton* btnDelFiles;

  QHBoxLayout* lh = new QHBoxLayout();
  lh->setSpacing(space);
  lh->addWidget(lbl = new QLabel(label, this));
  lh->addStretch(2);
  lh->addWidget(btnAddFiles = new QPushButton("+", this));
  lh->addWidget(btnDelFiles = new QPushButton("-", this));

  QVBoxLayout* lv = new QVBoxLayout (this);
  lv->setSpacing(space);
  lv->addLayout(lh);
  if (t == MULTIPLE_EXISTING_PUBLIC_KEYS || t == MULTIPLE_EXISTING_FILES) {
    singleFile = NULL;

    if (t == MULTIPLE_EXISTING_PUBLIC_KEYS)
      multipleFiles = new PubKeysFileField(this, dialogTitle, filter, t);
    else 
      multipleFiles = new MultipleFileField(this, desc, dialogTitle, filter, t);

    multipleFiles->setMinimumSize(QSize(viewMinWidth, viewMinHeight));
    lv->addWidget(multipleFiles);

    connect(btnAddFiles, SIGNAL(clicked()), 
                          multipleFiles, SLOT(askUserForFiles()));
    connect(btnDelFiles, SIGNAL(clicked()), 
                          multipleFiles, SLOT(deleteSelectedFiles()));
    connect(this, SIGNAL(cleanSignal()), multipleFiles, SLOT(clean()));
    connect(this, SIGNAL(insertSignal(const QString&)), 
                          multipleFiles, SLOT(insertFile (const QString&)));

    connect(multipleFiles, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)),
                      this, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)));
  } else {
    multipleFiles = NULL;

    if (t == SINGLE_EXISTING_PRIVATE_KEY)
      singleFile = new SinglePrivateKeyFileField(this, desc, 
                                                  dialogTitle, filter, t);
    else
      singleFile = new SingleFileField(this, desc, dialogTitle, filter, t);

    singleFile->setMinimumSize(QSize(lineMinWidth, lineMinHeight));
    lv->addWidget(singleFile);

    connect(singleFile, SIGNAL(textChanged(const QString&)), 
                                this, SIGNAL(valueChanged(const QString&)));
    connect(singleFile, SIGNAL(idChanged(const QString&, const QString &)),
                      this, SIGNAL(idChanged(const QString &, const QString &)));
    connect(btnAddFiles, SIGNAL(clicked()), 
                                singleFile, SLOT(askUserForAFile()));
    connect(btnDelFiles, SIGNAL(clicked()), singleFile, SLOT(clean()));
    connect(this, SIGNAL(cleanSignal()), singleFile, SLOT(clean()));
    connect(this, SIGNAL(insertSignal(const QString&)), 
                                singleFile, SLOT(insertFile (const QString&)));
  }
}

const 
QStringList FileField::returnFilenames() 
{
  if (multipleFiles) {
    return multipleFiles->returnFilenames();
  } else if (singleFile) {
    return singleFile->returnFilename();
  }

  return QStringList();
}

const 
QStringList FileField::returnSelectedFilenames() 
{
  if (multipleFiles) {
    return multipleFiles->returnSelectedFilenames();
  } else if (singleFile) {
    return singleFile->returnFilename();
  }

  return QStringList();
}

void 
FileField::deleteSelectedFiles()
{
  if (multipleFiles) {
    multipleFiles->deleteSelectedFiles();
  } else if (singleFile) {
    clean();
  }
}

void 
FileField::clean() 
{
  emit cleanSignal();
}

void 
FileField::insertFile(const QString& s) 
{
  emit insertSignal(s);
}

QString
FileField::defaultPath() const
{
  if (multipleFiles)
    return multipleFiles->defaultPath();
  else 
    return singleFile->defaultPath();
}

void
FileField::setDefaultPath(const QString &path)
{
  if (multipleFiles)
    multipleFiles->setDefaultPath(path);
  else
    singleFile->setDefaultPath(path);
}

void 
FileField::askUser() 
{
  if (multipleFiles)
    multipleFiles->askUserForFiles();
  else if (singleFile)
    singleFile->askUserForAFile();
}

void 
FileField::updateVirtualId(const QString &id, const QString &path)
{
  if (multipleFiles)
    multipleFiles->updateVirtualId(id, path);
}

void
FileField::setIcon(const QIcon &icon)
{
  if (multipleFiles)
    multipleFiles->setIcon(icon);
}

void
FileField::setVirtualIcon(const QIcon &icon)
{
  if (multipleFiles)
    multipleFiles->setVirtualIcon(icon);
}
// vi:sw=2:ts=2:et:co=80:
