#ifndef __ENTITY_PARENT_CHANGE_COMMAND_H__
#define __ENTITY_PARENT_CHANGE_COMMAND_H__

#include "Commands2/Base/Command2.h"

class EntityParentChangeCommand : public Command2
{
public:
    EntityParentChangeCommand(DAVA::Entity* entity, DAVA::Entity* newParent, DAVA::Entity* newBefore = NULL);
    ~EntityParentChangeCommand();

    virtual void Undo();
    virtual void Redo();
    virtual DAVA::Entity* GetEntity() const;

    DAVA::Entity* entity;
    DAVA::Entity* oldParent;
    DAVA::Entity* oldBefore;
    DAVA::Entity* newParent;
    DAVA::Entity* newBefore;
};

#endif // __ENTITY_PARENT_CHANGE_COMMAND_H__
