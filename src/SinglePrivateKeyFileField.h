// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright © 2009-2018 ANSSI. All Rights Reserved.
/**
 * @file SinglePrivateKeyFileField.h
 * cryptclt single private key selector header.
 * @author Olivier Levillain <clipos@ssi.gouv.fr>
 * @author Vincent Strubel <clipos@ssi.gouv.fr>
 *
 * Copyright (C) 2010 ANSSI
 * @n
 * All rights reserved.
 */

#ifndef SINGLEPRIVATEKEYFILEFIELD_H
#define SINGLEPRIVATEKEYFILEFIELD_H

#include "SingleFileField.h"

class SinglePrivateKeyFileField : public SingleFileField
{
    Q_OBJECT

 public:
  SinglePrivateKeyFileField(QWidget* parent, const char* desc, 
          const char* titreDialogue, const char* f, file_field_t t);

 public slots:
  void insertFile(const QString& s);

};

#endif // SINGLEPRIVATEKEYFILEFIELD_H
// vi:sw=2:ts=2:et:co=80:
