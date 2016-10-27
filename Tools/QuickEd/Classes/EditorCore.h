#pragma once

#include "Base/Introspection.h"
#include "AssetCache/AssetCacheClient.h"

#include <QObject>

class QAction;
class Document;
class DocumentGroup;
class Project;
class PackageNode;
class SpritesPacker;
class MainWindow;

namespace DAVA
{
class AssetCacheClient;
class ResultList;
}

class EditorCore
: public QObject
  ,
  public DAVA::InspBase
{
    Q_OBJECT
public:
    explicit EditorCore(QObject* parent = nullptr);
    ~EditorCore();

    void Start();

signals:
    void AssetCacheChanged(DAVA::AssetCacheClient* assetCacheClient);

private slots:
    void OnNewProject();
    void OnOpenProject();
    void OnCloseProject();
    void OnExit();
    void OnGLWidgedInitialized();
    void OnShowHelp();

private:
    static std::tuple<std::unique_ptr<Project>, DAVA::ResultList> CreateProject(const QString& path, MainWindow* mainWindow);
    static std::tuple<QString, DAVA::ResultList> CreateNewProject();

    void OpenProject(const QString& path);
    bool CloseProject();

    bool IsUsingAssetCache() const;
    void SetUsingAssetCacheEnabled(bool enabled);

    void EnableCacheClient();
    void DisableCacheClient();

    void OnProjectOpen(const Project* project);
    void OnProjectClose(const Project* project);

    const QStringList& GetRecentProjects() const;
    QString GetLastProject() const;
    void AddRecentProject(const QString& projectPath);

    DAVA::String GetRecentProjectsAsString() const;
    void SetRecentProjectsFromString(const DAVA::String& str);

    void UnpackHelp();

    std::unique_ptr<DAVA::AssetCacheClient> cacheClient;

    std::unique_ptr<Project> project;
    std::unique_ptr<MainWindow> mainWindow;

    DAVA::AssetCacheClient::ConnectionParams connectionParams;
    bool assetCacheEnabled;

    QStringList recentProjects;

    DAVA::uint32 projectsHistorySize;

public:
    INTROSPECTION(EditorCore,
                  PROPERTY("isUsingAssetCache", "Asset cache/Use asset cache", IsUsingAssetCache, SetUsingAssetCacheEnabled, DAVA::I_PREFERENCE)
                  PROPERTY("projectsHistory", "ProjectInternal/ProjectsHistory", GetRecentProjectsAsString, SetRecentProjectsFromString, DAVA::I_SAVE | DAVA::I_PREFERENCE)
                  //maximum size of projects history
                  MEMBER(projectsHistorySize, "Project/projects history size", DAVA::I_SAVE | DAVA::I_PREFERENCE)
                  )
};
