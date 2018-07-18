// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright © 2009-2018 ANSSI. All Rights Reserved.
/**
 * @file common.cpp
 * cryptclt common functions.
 * @author Olivier Levillain <clipos@ssi.gouv.fr>
 * @author Vincent Strubel <clipos@ssi.gouv.fr>
 *
 * Copyright (C) 2009 SGDN
 * Copyright (C) 2010-2012 ANSSI
 * @n
 * All rights reserved.
 */

#include "common.h"

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <sys/errno.h>
#include <unistd.h>

#include <QListWidget>
#include <QMessageBox>
#include <QMimeData>
#include <QString>
#include <QTreeWidget>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QRegExp>
#include <QUrl>

#include <stdio.h>
#include <cryptd_red.h>

#include "err.h"

int g_verbose = 0;
int g_daemonized = 0; /* Not used */


// Socket

int 
sock_connect(const char* sockpath)
{
  int s;
  struct sockaddr_un sau;
  
  sau.sun_family = AF_UNIX;
  snprintf(sau.sun_path, sizeof(sau.sun_path), 
     "%s", sockpath);
  
  s = socket(PF_UNIX, SOCK_STREAM, 0);
  if (s < 0)
    return s;
  if (connect(s, (struct sockaddr *)&sau, sizeof(struct sockaddr_un)) < 0) {
    close(s);
    return -1;
  }
  
  if (set_nonblock(s)) {
    close(s);
    return -1;
  }
  
  return s;
}

void 
close_socket(int s) 
{
  close(s);
}


// QT

bool 
yesOrNo(const QString& titre, const QString& contenu) 
{
  return (QMessageBox::question(0, titre, contenu, 
              QMessageBox::Yes | QMessageBox::No,
              QMessageBox::Yes) == QMessageBox::Yes);
}


void 
error(const QString& contenu) 
{
  QMessageBox::warning (0, "Erreur", contenu, 
              QMessageBox::Ok, QMessageBox::Ok);
}

void 
info(const QString& contenu) 
{
  QMessageBox::information (0, "Information", contenu, 
              QMessageBox::Ok, QMessageBox::Ok);
}

void 
refreshList(const char* const socketName, QListWidget& l, const QIcon &icon) 
{
  int s = -1; 
  uint32_t len, ret;
  QString selection, str;
  QListWidgetItem *selected = NULL;
  QAbstractItemView::SelectionMode selMode = l.selectionMode();
  char *buf = NULL, *ptr, *base;

  s = sock_connect(socketName);
  if (s < 0) {
    error ("Erreur lors de la connexion au démon cryptd.");
    goto err;
  }

  ret = cryptd_get_list(s, &buf, &len);
  if (ret != CMD_OK) {
    error(QString("Impossible de récupérer la liste des "
                    "identifiants à exporter : ") + cmderr(ret));
    goto err;
  }

  if (buf == NULL) {
    l.clear();
    return;
  }

  if (buf[len - 1] != '\0') {
    error ("Liste d'identifiants mal formée.");
    goto err;
  }

  base = buf;
  if (selMode == QAbstractItemView::SingleSelection && l.currentItem())
    selection = l.currentItem()->text();

  l.setSelectionMode(QAbstractItemView::NoSelection);
  l.clearSelection();
  l.clear();
  l.setSelectionMode(selMode);

  while ((ptr = strchr(base, '\n'))) {
    *ptr = 0;
    str = QString::fromUtf8(base);
    if (!selection.isEmpty() && str == selection) {
      selected = new QListWidgetItem(icon, str, 0);
      l.addItem(selected);
    } else 
      l.addItem(new QListWidgetItem(icon, str, 0));
    base = ptr + 1;
  }

  if (selected)
    l.setCurrentItem(selected);

  close(s);
  return;

 err:
  if (s >= 0)
    close(s);
  if (buf)
    free(buf);
}

static inline const char *
feature_err(uint32_t wanted_feature)
{
  switch (wanted_feature) {
    case CryptdCrypt:
      return "Le démon cryptd ne supporte pas la fonctionnalité "
                        "de diode cryptographique.";
    case CryptdDiode:
      return "Le démon cryptd ne supporte pas la fonctionnalité "
                        "de diode montante.";
    case CryptdChPw:
      return "Le démon cryptd ne supporte pas la fonctionnalité "
                        "de changement de mot de passe.";
    default:
      return "Le démon cryptd ne supporte pas les fonctionnalités nécessaires.";
  }
}

bool
getServerInfo(const char *const socketName, uint32_t *feat, bool err)
{
  int s = -1;
  uint32_t ret, version, features;

  s = sock_connect(socketName);
  if (s < 0) {
    if (err)
      error("Erreur lors de la connexion au démon cryptd.");
    goto err;
  }

  ret = cryptd_get_info(s, &version, &features);
  if (ret != CMD_OK) {
    if (err)
      error("Version du démon cryptd incompatible");
    goto err;
  }
  ret = cryptd_check_version(version);
  if (ret != CMD_OK) {
    if (err)
      error("Version du démon cryptd incompatible");
    goto err;
  }

  *feat = features;
  close(s);
  return true;

err:
  if (s >= 0)
    close(s);
  return false;
}

bool 
checkServerInfo(const char *const socketName, uint32_t wanted_features, bool err) 
{
  uint32_t features;

  if (!getServerInfo(socketName, &features, err))
    return false;

  if ((features & wanted_features) != wanted_features) {
    if (err)
      error(feature_err(wanted_features));
    return false;
  }

  return true;
}

QStringList 
filenamesFromDnD(const QMimeData *md, const QString& filter, file_field_t type)
{
  QStringList res;

  if (md->hasUrls()) {
    QList<QUrl>l = md->urls();
    for (QList<QUrl>::ConstIterator i = l.constBegin(); 
                                      i != l.constEnd(); i++) { 
      QString tmp, cur;
      cur = QUrl::fromPercentEncoding((*i).encodedPath());
      if (cur.startsWith ("file:///"))
        tmp = cur.mid (7);
      else
        tmp = cur;

      if (!checkFile(tmp, filter, type))
        return QStringList();
      res.append (tmp);
    }
  }

  return res;
}


bool 
checkFile(const QString& s, const QString& filter, file_field_t type) 
{
  if (s.isEmpty() || (!s.endsWith(filter))) {
    qDebug("empty or invalid path: %s", s.toUtf8().data());
    return false;
  }

  if ((type == SINGLE_EXISTING_FILE || type == MULTIPLE_EXISTING_FILES) 
                      && !QFile::exists(s)) {
    qDebug("non existing path: %s", s.toUtf8().data());
    return false;
  }

  if ((type == EXISTING_DIRECTORY) && !QDir(s).exists())
    return false;

  return true;
}

static inline const char *
save_path(cryptclt_path_t type)
{
  switch (type) {
    case CRYPTCLT_PVR_PATH:
      return "default_pvr";
    case CRYPTCLT_PPR_PATH:
      return "default_ppr";
    case CRYPTCLT_INPUT_PATH:
      return "default_input";
    case CRYPTCLT_OUTPUT_PATH:
      return "default_output";
    default:
      return NULL;
  }
}

QString 
loadDefaultPath(cryptclt_path_t type) 
{
  QFile file(QDir::homePath() + QDir::separator() 
      + ".cryptclt" + QDir::separator() + save_path(type));
  
  if (file.exists()) {
    file.open(QIODevice::ReadOnly);
    QString line = QString::fromUtf8(file.readLine().data());
    if (line.endsWith("\n"))
      line.resize(line.size() - 1);
    return line;
  }

  switch (type) {
    case CRYPTCLT_PVR_PATH:
      return QString();
    default:
      return QDir::homePath();
  }

}

void 
saveDefaultPath(const QString& s, cryptclt_path_t type) 
{
  QString path(QDir::homePath() + QDir::separator() + ".cryptclt");
  const char *fname = save_path(type);
  if (!fname) {
    qDebug("unsupported save type: %d", type);
    return;
  }

  if (!QDir(path).exists())
    QDir().mkpath(path);

  QFile file(path + QDir::separator() + fname);
  if (file.exists())
    file.remove();

  file.open(QIODevice::WriteOnly|QIODevice::Truncate);
  QTextStream(&file) << s;
}

uint32_t 
get_memsize(void) 
{
  QFile meminfo("/proc/meminfo");
  meminfo.open(QIODevice::ReadOnly);
  QString total = meminfo.readLine();

  QRegExp sep("\\s+");
  QString num = total.section(sep, 1, 1);

  return num.toUInt(); /* Size in kilobytes */
}

uint32_t 
get_filesize(const char *name) 
{
  struct stat st;

  if (stat(name, &st)) {
    error(QString("Impossible de lire la taille du fichier %1").arg(name));
    return (uint32_t)-1;
  }

  return st.st_size / 1024; /* Size in kilobytes */
}
// vi:sw=2:ts=2:et:co=80:
