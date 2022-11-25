// SPDX-License-Identifier: GPL-3.0-only

#include "ModsUpdateTask.h"
#include "minecraft/MinecraftInstance.h"
#include "minecraft/mod/ModFolderModel.h"
#include "ui/dialogs/ModUpdateDialog.h"

ModsUpdateTask::ModsUpdateTask(MinecraftInstance * inst)
{
    m_instance = inst;
}

void ModsUpdateTask::executeTask()
{
    if (m_instance->settings()->get("UpdateModsOnLaunch").toBool()) {

        // we need to first update the mod list of the instance
        // otherwise it will not find any mods to update
        setStatus(tr("Updating mod list"));
        m_model = m_instance->loaderModList();
        m_model->update();
        connect(m_model.get(), &ModFolderModel::updateFinished, this, &ModsUpdateTask::updateMods);
    } else {
        emitSucceeded();
    }
}

void ModsUpdateTask::updateMods()
{
    if (m_updateDone) {
        return;
    }
    m_updateDone = true;
    setStatus(tr("Checking for mod updates"));

    auto mods_list = m_model->allMods();

    QWidget parent;

    ModUpdateDialog update_dialog(&parent, m_instance, m_model, mods_list);

    update_dialog.checkCandidates();

    if (update_dialog.aborted()) {
        emitFailed();
        return;
    }
    if (update_dialog.noUpdates()) {
        emitSucceeded();
        return;
    }

    if (update_dialog.exec()) {
        setStatus(tr("Updating Mods..."));

        ConcurrentTask* tasks = new ConcurrentTask(this);

        connect(tasks, &Task::failed, [this, tasks](QString reason) {
            qDebug() << "---- Updating mods task failed";
            emitFailed(reason);
            tasks->deleteLater();
        });

        connect(tasks, &Task::aborted, [this, tasks] {
            qDebug() << "---- Updating mods task aborted";
            emitSucceeded();
            tasks->deleteLater();
        });

        connect(tasks, &ConcurrentTask::progress, this, &ModsUpdateTask::progress);

        connect(tasks, &Task::succeeded, [this, tasks] {
            qDebug() << "---- Updating mods task succeded";
            emitSucceeded();
            tasks->deleteLater();
        });

        for (auto task : update_dialog.getTasks()) {
            tasks->addTask(task);
        }
        tasks->start();
    } else {
        emitSucceeded();
    }
}
bool ModsUpdateTask::canAbort() const
{
    return true;
}

bool ModsUpdateTask::abort()
{
    qWarning() << "---- aborted ModsUpdateTask";
    return true;
}
