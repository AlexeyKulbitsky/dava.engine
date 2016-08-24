#include "LibraryModule.h"
#include "SceneViewOperations.h"

#include "TArcCore/ContextAccessor.h"
#include "WindowSubSystem/UI.h"
#include "WindowSubSystem/ActionUtils.h"

#include "Reflection/Registrator.h"
#include "Base/Type.h"

#include <QListWidget>
#include <QFileSystemModel>
#include <QTimer>

class FileSystemData : public DAVA::TArc::DataNode
{
public:
    FileSystemData(LibraryModule* self_, QFileSystemModel* model_)
        : self(self_)
        , model(model_)
    {
    }

    ~FileSystemData()
    {
        delete model;
    }

    QFileSystemModel* GetModel() const
    {
        return model;
    }

    void OpenScene(QModelIndex index)
    {
        DAVA::String path = model->filePath(index).toStdString();
        self->InvokeOperation(SceneViewOperations::OpenScene, path);
    }

private:
    LibraryModule* self = nullptr;
    QFileSystemModel* model = nullptr;
    DAVA_VIRTUAL_REFLECTION(FileSystemData, DAVA::TArc::DataNode)
    {
        DAVA::ReflectionRegistrator<FileSystemData>::Begin()
        .Field("fileSystemModel", &FileSystemData::GetModel, nullptr)
        .Method("openScene", &FileSystemData::OpenScene)
        .End();
    }
};

void LibraryModule::OnContextCreated(DAVA::TArc::DataContext& context)
{
}

void LibraryModule::OnContextDeleted(DAVA::TArc::DataContext& context)
{
}

void LibraryModule::PostInit()
{
    QFileSystemModel* model = new QFileSystemModel();
    model->setNameFilters(QStringList() << "*.sc2");
    model->setNameFilterDisables(false);
    model->setRootPath(QDir::rootPath());

    DAVA::TArc::DataContext& globalContext = GetAccessor().GetGlobalContext();

    globalContext.CreateData(std::make_unique<FileSystemData>(this, model));

    DAVA::TArc::UI& ui = GetUI();

    DAVA::TArc::WindowKey windowKey(DAVA::FastName("TemplateTArc"));

    DAVA::TArc::ActionPlacementInfo actionInfo = DAVA::TArc::CreateMenuPoint("View/Dock");
    DAVA::TArc::DockPanelInfo info("Library", actionInfo, false, Qt::LeftDockWidgetArea);

    ui.AddView(windowKey, DAVA::TArc::PanelKey(info.title, info), "qrc:/Library.qml",
               GetAccessor().CreateWrapper(DAVA::ReflectedType::Get<FileSystemData>()));
}
