// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright © 2009-2018 ANSSI. All Rights Reserved.
/**
 * @file frmClientDown.h
 * cryptclt diode down client main window header.
 * @author Olivier Levillain <clipos@ssi.gouv.fr>
 * @author Vincent Strubel <clipos@ssi.gouv.fr>
 *
 * Copyright (C) 2009 SGDN
 * Copyright (C) 2010 ANSSI
 * @n
 * All rights reserved.
 */

#ifndef FRMCLIENTDOWN_H
#define FRMCLIENTDOWN_H

#include <QDialog>
#include <QString>

class QLabel;
class QLineEdit;
class QTabWidget;
class QFrame;
class QListBox;
class FileField;


class frmClientDown : public QDialog
{
    Q_OBJECT

public:
  frmClientDown(int argc, char** argv);

private:
  QFrame* frame;
  bool ok;
  bool clamCheck;

  FileField* toExport;
  QPushButton* btnExport;
  QPushButton* btnExportAndQuit;
  QPushButton* btnQuit;

  bool exportOneFile(const QString &);
  
protected:
  virtual void keyPressEvent(QKeyEvent* e);

private slots:

  virtual void doExport();
  virtual void exportAndQuit();

  virtual void quit();
  
};

#endif // FRMCLIENTDOWN_H

// vi:sw=2:ts=2:et:co=80:
