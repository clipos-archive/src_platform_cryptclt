// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright © 2009-2018 ANSSI. All Rights Reserved.
/**
 * @file frmWait.h
 * @author Olivier Levillain <clipos@ssi.gouv.fr>
 * @author Vincent Strubel <clipos@ssi.gouv.fr>
 *
 * Copyright (C) 2009 SGDN
 * Copyright (C) 2010 ANSSI
 * @n
 * All rights reserved.
 */

#ifndef FRMWAIT_H
#define FRMWAIT_H

#include <QDialog>

class QProcess;
class QKeyEvent;
class QCloseEvent;

class frmWait : public QDialog
{
  Q_OBJECT

 public:
  frmWait(const QString& title, const QString& msg, bool *ptrFinished);
  frmWait(const QString& title, const QString& msg, QProcess& p);

 protected:
  virtual void keyPressEvent(QKeyEvent *e);
  virtual void closeEvent(QCloseEvent *e);

 private:
  bool* finished;
  QProcess* runningProcess;
  void init(const QString& title, const  QString& msg);
};

#endif // FRMWAIT_H

// vi:sw=2:ts=2:et:co=80:
