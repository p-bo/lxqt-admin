/*
 *
 * Copyright (C) 2014  Hong Jen Yee (PCMan) <pcman.tw@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "mainwindow.h"
#include <QDebug>
#include <QMessageBox>
#include "userdialog.h"
#include "groupdialog.h"
#include "usermanager.h"

MainWindow::MainWindow():
    QMainWindow(),
    mUserManager(new UserManager(this))
{
    ui.setupUi(this);
    connect(ui.actionAdd, SIGNAL(triggered(bool)), SLOT(onAdd()));
    connect(ui.actionDelete, SIGNAL(triggered(bool)), SLOT(onDelete()));
    connect(ui.actionProperties, SIGNAL(triggered(bool)), SLOT(onEditProperties()));

    connect(ui.actionRefresh, SIGNAL(triggered(bool)), SLOT(reloadUsers()));
    connect(ui.actionRefresh, SIGNAL(triggered(bool)), SLOT(reloadGroups()));

    connect(mUserManager, &UserManager::usersChanged, this, &MainWindow::reloadUsers);
    connect(mUserManager, &UserManager::groupsChanged, this, &MainWindow::reloadGroups);

    reloadUsers();
    reloadGroups();
}

MainWindow::~MainWindow()
{
}

void MainWindow::reloadUsers()
{
    ui.userList->clear();
    const auto& users = mUserManager->users();
    for(const UserInfo* user: users)
    {
        uid_t uid = user->uid();
        if(uid > 499 && !user->shell().isEmpty()) // exclude system users
        {
            QTreeWidgetItem* item = new QTreeWidgetItem();
            item->setData(0, Qt::DisplayRole, user->name());
            QVariant obj = QVariant::fromValue<void*>((void*)user);
            item->setData(0, Qt::UserRole, obj);
            item->setData(1, Qt::DisplayRole, uid);
            item->setData(2, Qt::DisplayRole, user->fullName());
            GroupInfo* group = mUserManager->findGroupInfo(user->gid());
            if(group != nullptr) {
                item->setData(3, Qt::DisplayRole, group->name());
            }
            item->setData(4, Qt::DisplayRole, user->homeDir());
            ui.userList->addTopLevelItem(item);
        }
    }
}

void MainWindow::reloadGroups()
{
    ui.groupList->clear();
    // load groups
    const auto& groups = mUserManager->groups();
    for(const GroupInfo* group: groups)
    {
        QTreeWidgetItem* item = new QTreeWidgetItem();
        item->setData(0, Qt::DisplayRole, group->name());
        QVariant obj = QVariant::fromValue<void*>((void*)group);
        item->setData(0, Qt::UserRole, obj);
        item->setData(1, Qt::DisplayRole, group->gid());
        ui.groupList->addTopLevelItem(item);
    }
}

UserInfo *MainWindow::userFromItem(QTreeWidgetItem *item)
{
    if(item)
    {
        QVariant obj = item->data(0, Qt::UserRole);
        return reinterpret_cast<UserInfo*>(obj.value<void*>());
    }
    return nullptr;
}

GroupInfo* MainWindow::groupFromItem(QTreeWidgetItem *item)
{
    if(item)
    {
        QVariant obj = item->data(0, Qt::UserRole);
        return reinterpret_cast<GroupInfo*>(obj.value<void*>());
    }
    return nullptr;
}

void MainWindow::onAdd()
{
    if(ui.tabWidget->currentIndex() == PageUsers)
    {
        UserInfo newUser;
        UserDialog dlg(mUserManager, &newUser, this);
        if(dlg.exec() == QDialog::Accepted)
        {
            mUserManager->addUser(&newUser);
        }
    }
    else if (ui.tabWidget->currentIndex() == PageGroups)
    {
        GroupInfo newGroup;
        GroupDialog dlg(mUserManager, &newGroup, this);
        if(dlg.exec() == QDialog::Accepted)
        {
            mUserManager->addGroup(&newGroup);
        }
    }
}

void MainWindow::onDelete()
{
    if(ui.tabWidget->currentIndex() == PageUsers)
    {
        QTreeWidgetItem* item = ui.userList->currentItem();
        UserInfo* user = userFromItem(item);
        if(user)
        {
            if(QMessageBox::question(this, tr("Confirm"), tr("Are you sure you want to delete the selected user?"), QMessageBox::Ok|QMessageBox::Cancel) == QMessageBox::Ok)
            {
                mUserManager->deleteUser(user);
            }
        }
    }
    else if(ui.tabWidget->currentIndex() == PageGroups)
    {
        QTreeWidgetItem* item = ui.groupList->currentItem();
        GroupInfo* group = groupFromItem(item);
        if(group)
        {
            if(QMessageBox::question(this, tr("Confirm"), tr("Are you sure you want to delete the selected group?"), QMessageBox::Ok|QMessageBox::Cancel) == QMessageBox::Ok)
            {
                mUserManager->deleteGroup(group);
            }
        }
    }
}

void MainWindow::onEditProperties()
{
    if(ui.tabWidget->currentIndex() == PageUsers)
    {
        QTreeWidgetItem* item = ui.userList->currentItem();
        UserInfo* user = userFromItem(item);
        if(user) {
            UserInfo newSettings(*user);
            UserDialog dlg(mUserManager, &newSettings, this);
            if(dlg.exec() == QDialog::Accepted)
            {
                mUserManager->modifyUser(user, &newSettings);
            }
        }
    }
    else if(ui.tabWidget->currentIndex() == PageGroups)
    {
        QTreeWidgetItem* item = ui.groupList->currentItem();
        GroupInfo* group = groupFromItem(item);
        if(group) {
            GroupInfo newSettings(*group);
            GroupDialog dlg(mUserManager, &newSettings, this);
            if(dlg.exec() == QDialog::Accepted)
            {
                mUserManager->modifyGroup(group, &newSettings);
            }
        }
    }
}

