// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright © 2009-2018 ANSSI. All Rights Reserved.
/**
 * @file common.h
 * cryptclt common functions header.
 * @author Olivier Levillain <clipos@ssi.gouv.fr>
 * @author Vincent Strubel <clipos@ssi.gouv.fr>
 *
 * Copyright (C) 2009 SGDN
 * Copyright (C) 2010-2011 ANSSI
 * @n
 * All rights reserved.
 */

#ifndef COMMON_H
#define COMMON_H

#define QT_THREAD_SUPPORT 1

#include <QString>
class QTreeWidget;
class QListWidget;
class QMimeData;
class QIcon;

extern "C" {
#define __STDC_LIMIT_MACROS 1
#include <stdint.h>
#include <stdlib.h>
#include "list.h"
extern uint32_t client_get_list(int, char **, uint32_t *);
extern int set_nonblock(int);
}
#include <cryptd_features.h>

int sock_connect(const char* sockpath);
void close_socket(int s);

#define ICONPATH PREFIX"/share/icons"

// QT (affichage)

static const int viewMinHeight = 260;
static const int viewMinWidth = 300;
static const int lineMinWidth = 300;
static const int lineMinHeight = 15;
static const int space = 7;

extern bool yesOrNo(const QString& titre, const QString& contenu);
extern void info(const QString& contenu);
extern void error(const QString& contenu);
extern void refreshList(const char* const socketName, 
                                QListWidget& l, const QIcon &icon);
extern bool getServerInfo(const char *const socketName, uint32_t *feat, bool err = true);
extern bool checkServerInfo (const char* const socketName, uint32_t features, bool err = true);

/* Get memory size in kilobytes */
extern uint32_t get_memsize(void);
/* Get file size in kilobytes (return uint32_t -1 on error) */
extern uint32_t get_filesize(const char *name);


// Gestion des fichiers

typedef enum {
  SINGLE_EXISTING_FILE,
  SINGLE_DESTINATION_FILE,
  MULTIPLE_EXISTING_FILES,
  EXISTING_DIRECTORY,
  MULTIPLE_EXISTING_PUBLIC_KEYS,
  SINGLE_EXISTING_PRIVATE_KEY,
} file_field_t;

typedef enum {
  CRYPTCLT_PVR_PATH,
  CRYPTCLT_PPR_PATH,
  CRYPTCLT_INPUT_PATH,
  CRYPTCLT_OUTPUT_PATH
} cryptclt_path_t;


extern QStringList filenamesFromDnD(const QMimeData* e, 
                            const QString& filter, file_field_t type);
extern bool checkFile(const QString& s, 
                            const QString& filter, file_field_t type);
extern QString loadDefaultPath(cryptclt_path_t);
extern void saveDefaultPath(const QString&, cryptclt_path_t);

#endif  // COMMON_H
// vi:sw=2:ts=2:et:co=80:
