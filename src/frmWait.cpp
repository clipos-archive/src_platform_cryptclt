// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright © 2009-2018 ANSSI. All Rights Reserved.
/**
 * @file frmWait.cpp
 * @author Olivier Levillain <clipos@ssi.gouv.fr>
 * @author Vincent Strubel <clipos@ssi.gouv.fr>
 *
 * Copyright (C) 2009 SGDN
 * Copyright (C) 2010 ANSSI
 * @n
 * All rights reserved.
 */

#include "frmWait.h"

#include <QLabel>
#include <QTimer>
#include <QLayout>
#include <QProcess>
#include <QKeyEvent>
#include <QCloseEvent>

void 
frmWait::init(const QString& title, const  QString& msg) 
{
  setWindowTitle(title);
  QLabel* lbl = new QLabel(msg, this);
  lbl->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));

  QVBoxLayout* formLayout = new QVBoxLayout(this);
  formLayout->setSpacing(5);

  QHBoxLayout* horzLayout = new QHBoxLayout(0);
  horzLayout->addSpacerItem(
    new QSpacerItem(30, 0, QSizePolicy::Expanding, QSizePolicy::Expanding));
  horzLayout->addWidget (lbl);
  horzLayout->addSpacerItem(
    new QSpacerItem(30, 0, QSizePolicy::Expanding, QSizePolicy::Expanding));
  
  formLayout->addSpacerItem(
    new QSpacerItem(0, 30, QSizePolicy::Expanding, QSizePolicy::Expanding));
  formLayout->addLayout(horzLayout);
  formLayout->addSpacerItem(
    new QSpacerItem (0, 15, QSizePolicy::Expanding, QSizePolicy::Expanding));
  
  QTimer* tmrTest = new QTimer(this);
  connect(tmrTest, SIGNAL(timeout()), this, SLOT(close()));
  tmrTest->start(100);
}


frmWait::frmWait(const QString& title, 
    const QString& msg, bool *ptrFinished)
  : QDialog (0, Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint), 
    finished(ptrFinished), runningProcess(NULL)
{
  init(title, msg);
}

frmWait::frmWait(const QString& title, const  QString& msg, QProcess& p)
  : QDialog (0, Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint), 
    finished(NULL), runningProcess(&p)
{
  init(title, msg);
}

void 
frmWait::keyPressEvent(QKeyEvent* e)
{
  if (e->key() == Qt::Key_Escape)
    e->accept();
}


void 
frmWait::closeEvent(QCloseEvent* e)
{
  if (((finished != NULL)  && (*finished)) ||
      ((runningProcess != NULL) && 
      (runningProcess->state() != QProcess::Running)))
    e->accept();
  else
    e->ignore();
}

// vi:sw=2:ts=2:et:co=80:
