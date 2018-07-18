// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright © 2009-2018 ANSSI. All Rights Reserved.
/**
 * @file cryptclt.cpp
 * cryptclt common main.
 * Note that this file is preprocessed to generate different clients.
 * @author Olivier Levillain <clipos@ssi.gouv.fr>
 * @author Vincent Strubel <clipos@ssi.gouv.fr>
 *
 * Copyright (C) 2009 SGDN
 * Copyright (C) 2010 ANSSI
 * @n
 * All rights reserved.
 */

#include <QApplication>
#include <QTextCodec>
#include <QTranslator>

#include "frmClient.h"

#include <stdio.h>
#include <stdlib.h>


int main (int argc, char** argv) {
  QApplication a(argc, argv);

  // Mise en place des traductions
  QTranslator qt (0);
  qt.load ("qt_fr.qm", PREFIX"/share/qt4/translations");
  a.installTranslator(&qt);

  QTextCodec::setCodecForCStrings(QTextCodec::codecForName("utf8"));
    
  // Création et affichage de la fenêtre principale
  frmClient form(argc, argv);
  form.show();

  exit(a.exec());
}

// vi:sw=2:ts=2:et:co=80:
