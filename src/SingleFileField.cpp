// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright © 2009-2018 ANSSI. All Rights Reserved.
/**
 * @file SingleFileField.cpp
 * cryptclt single file selector implementation.
 * @author Olivier Levillain <clipos@ssi.gouv.fr>
 * @author Vincent Strubel <clipos@ssi.gouv.fr>
 *
 * Copyright (C) 2009 SGDN
 * Copyright (C) 2010 ANSSI
 * @n
 * All rights reserved.
 */

#include "SingleFileField.h"

#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFileDialog>
#include <QLineEdit>
#include <QStringList>
#include <QFileInfo>

#include <exception>


SingleFileField::SingleFileField(QWidget* parent, const char* desc, 
                  const char* dialogTitle, const char* f, file_field_t t)
  : QLineEdit(parent), filterSuffix(f), title(dialogTitle), 
    defPath(QDir::homePath()), type(t)
{
  if (t == MULTIPLE_EXISTING_FILES)
    throw std::exception();

  if (filterSuffix.length() > 0) {
    if (desc)
      QTFilter = QString(desc) + " ";
    QTFilter += "(*" + filterSuffix + ")";
  }

  setReadOnly(true);
  setAcceptDrops(true);
}

void 
SingleFileField::dragEnterEvent(QDragEnterEvent* e) 
{
  if (e->mimeData()->hasText() && !e->mimeData()->text().isEmpty())
    e->acceptProposedAction();
}

void 
SingleFileField::dropEvent (QDropEvent* e) 
{
  QStringList l = filenamesFromDnD(e->mimeData(), filterSuffix, type);
  if (l.size() == 1)
    insertFile (l.first());
}

void 
SingleFileField::askUserForAFile() 
{
  QString s;

  switch (type) {
    case SINGLE_EXISTING_FILE:
      s = QFileDialog::getOpenFileName(NULL, title, defaultPath(), QTFilter);
      break;
    case SINGLE_DESTINATION_FILE:
      if (text().isEmpty())
        s = QFileDialog::getSaveFileName(NULL, title, defaultPath(), QTFilter);
      else
        s = QFileDialog::getSaveFileName(NULL, title, text(), QTFilter);
      break;
    case EXISTING_DIRECTORY:
      s = QFileDialog::getExistingDirectory(NULL, title, defaultPath());
      break;
    default:
      throw std::exception();
  }

  insertFile(s);
}

void 
SingleFileField::insertFile(const QString& s) 
{
  QString f(s);
  if (f.endsWith("\n"))
    f.chop(1);
  if (!f.endsWith(filterSuffix))
    f += filterSuffix;
  if (checkFile(f, filterSuffix, type)) {
    setText(f);
    defPath = QFileInfo(f).absolutePath();
  }
}

void 
SingleFileField::clean() 
{
  setText("");
}

const QStringList 
SingleFileField::returnFilename() 
{
  QStringList res;
  if (!text().isEmpty())
    res.append(text());
  return res;
}
// vi:sw=2:ts=2:et:co=80:
