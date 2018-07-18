// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright © 2009-2018 ANSSI. All Rights Reserved.
/**
 * @file SinglePrivateKeyFileField.cpp
 * cryptclt single private key selector implementation.
 * @author Olivier Levillain <clipos@ssi.gouv.fr>
 * @author Vincent Strubel <clipos@ssi.gouv.fr>
 *
 * Copyright (C) 2010 ANSSI
 * @n
 * All rights reserved.
 */

#include "SinglePrivateKeyFileField.h"
#include "err.h"

#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFileDialog>
#include <QLineEdit>
#include <QStringList>
#include <QFileInfo>
#include <cryptd/cryptd_files.h>
#include <clip/acidfile.h>

#include <exception>


SinglePrivateKeyFileField::SinglePrivateKeyFileField(QWidget* parent, const char* desc, 
              const char* dialogTitle, const char* f, file_field_t t)
  : SingleFileField(parent, desc, dialogTitle, f, SINGLE_EXISTING_FILE)
{
  if (t != SINGLE_EXISTING_PRIVATE_KEY)
    throw std::exception();
}

void 
SinglePrivateKeyFileField::insertFile(const QString& s) 
{
  QString f(s);
  char *bundle = NULL;
  acid_key_t *certs = NULL;
  const char *id;
  uint32_t ret, blen;
  size_t bsize;

  if (f.endsWith("\n"))
    f.chop(1);
  if (!f.endsWith(filterSuffix))
    f += filterSuffix;
  if (!checkFile(f, filterSuffix, type))
    return;
  QFileInfo info(f);

  ret = cryptd_get_file(s.toUtf8().data(), &bundle, &bsize);
  if (ret != CMD_OK || bsize > UINT32_MAX)
    return;

  blen = (uint32_t) bsize;
  if (acidfile_check_headers(bundle, blen, Acid_FType_Priv, NULL)) {
    error(QString("Le fichier %1 n'est pas une clé "
                  "privée correctement formatée.").arg(s));
    goto out;
  }  
  
  if (acidfile_pxr_get_certs(bundle, &certs)) {
    error(QString("Le fichier %1 n'est pas une clé "
                  "privée correctement formatée.").arg(s));
    goto out;
  }

  id = acid_key_field(certs->next, ACID_Subject);
  if (!id) {
    error(QString("Le fichier %1 n'est pas une clé "
                  "privée correctement formatée.").arg(s));
    goto out;
  }

  emit idChanged(id, info.absoluteFilePath());
  
  setText(f);
  defPath = info.absolutePath();

  /* Fall through */
out:
  if (bundle)
    free(bundle);
  if (certs)
    acid_key_free_all(certs);
}
// vi:sw=2:ts=2:et:co=80:
