// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "minecraft/mod/ModFolderModel.h"
#include "tasks/Task.h"
#include "ui/dialogs/ModUpdateDialog.h"
class MinecraftInstance;

class ModsUpdateTask : public Task
{
    Q_OBJECT
public:
    ModsUpdateTask(MinecraftInstance * inst);

    void executeTask() override;

    bool canAbort() const override;

private slots:
    void updateMods();

public slots:
    bool abort() override;

private:
    MinecraftInstance *m_instance;
    std::shared_ptr<ModFolderModel> m_model;
    bool m_updateDone = false;
};
