/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/



#include "VisibilityToolActions.h"
#include "../Qt/Scene/System/LandscapeEditorDrawSystem/VisibilityToolProxy.h"
#include "../Qt/Scene/SceneEditor2.h"
#include "../Qt/Scene/SceneSignals.h"

#include "../Qt/Main/QtUtils.h"

ActionEnableVisibilityTool::ActionEnableVisibilityTool(SceneEditor2* forSceneEditor)
:	CommandAction(CMDID_VISIBILITY_TOOL_ENABLE)
,	sceneEditor(forSceneEditor)
{
}

void ActionEnableVisibilityTool::Redo()
{
	if (sceneEditor == NULL)
	{
		return;
	}
	
	bool enabled = sceneEditor->visibilityToolSystem->IsLandscapeEditingEnabled();
	if (enabled)
	{
		return;
	}

	sceneEditor->DisableTools(SceneEditor2::LANDSCAPE_TOOLS_ALL);

	bool success = !sceneEditor->IsToolsEnabled(SceneEditor2::LANDSCAPE_TOOLS_ALL);
	
	if (!success )
	{
		ShowErrorDialog(ResourceEditor::LANDSCAPE_EDITOR_SYSTEM_DISABLE_EDITORS);
	}
	
	LandscapeEditorDrawSystem::eErrorType enablingError = sceneEditor->visibilityToolSystem->EnableLandscapeEditing();
	if (enablingError != LandscapeEditorDrawSystem::LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS)
	{
		ShowErrorDialog(LandscapeEditorDrawSystem::GetDescriptionByError(enablingError));
	}
    
    if(success &&
       LandscapeEditorDrawSystem::LANDSCAPE_EDITOR_SYSTEM_NO_ERRORS == enablingError)
    {
        sceneEditor->foliageSystem->SetFoliageVisible(false);
    }
    
	SceneSignals::Instance()->EmitVisibilityToolToggled(sceneEditor);
}

ActionDisableVisibilityTool::ActionDisableVisibilityTool(SceneEditor2* forSceneEditor)
:	CommandAction(CMDID_VISIBILITY_TOOL_DISABLE)
,	sceneEditor(forSceneEditor)
{
}

void ActionDisableVisibilityTool::Redo()
{
	if (sceneEditor == NULL)
	{
		return;
	}
	
	bool disabled = !sceneEditor->visibilityToolSystem->IsLandscapeEditingEnabled();
	if (disabled)
	{
		return;
	}

	disabled = sceneEditor->visibilityToolSystem->DisableLandscapeEdititing();
	if (!disabled)
	{
		ShowErrorDialog(ResourceEditor::VISIBILITY_TOOL_DISABLE_ERROR);
	}
    
    if(disabled)
    {
        sceneEditor->foliageSystem->SetFoliageVisible(true);
    }

	SceneSignals::Instance()->EmitVisibilityToolToggled(sceneEditor);
}


ActionSetVisibilityPoint::ActionSetVisibilityPoint(Image* _originalImage,
												   Sprite* _cursorSprite,
												   VisibilityToolProxy* _visibilityToolProxy,
												   const Vector2& _visibilityPoint,
                                                   const Vector2& _cursorSize)
:	CommandAction(CMDID_VISIBILITY_TOOL_SET_POINT, "Set Visibility Point")
{
    cursorSprite = SafeRetain(_cursorSprite);
    visibilityToolProxy = SafeRetain(_visibilityToolProxy);
    redoVisibilityPoint = _visibilityPoint;
    cursorSize = _cursorSize;
}

ActionSetVisibilityPoint::~ActionSetVisibilityPoint()
{
	SafeRelease(cursorSprite);
	SafeRelease(visibilityToolProxy);
}

void ActionSetVisibilityPoint::Redo()
{
	Sprite* visibilityToolSprite = visibilityToolProxy->GetSprite();
    RenderSystem2D::Instance()->PushRenderTarget();
    RenderSystem2D::Instance()->SetRenderTarget(visibilityToolSprite);
	RenderManager::Instance()->ClearWithColor(0.f, 0.f, 0.f, 0.f);

    Sprite::DrawState drawState;
    drawState.SetPosition(VirtualCoordinatesSystem::Instance()->ConvertPhysicalToVirtual(redoVisibilityPoint - cursorSize / 2.f));
    drawState.SetScaleSize(cursorSize.x, cursorSize.y, cursorSprite->GetWidth(), cursorSprite->GetHeight());
    
    RenderSystem2D::Instance()->Setup2DMatrices();
    RenderSystem2D::Instance()->Draw(cursorSprite, &drawState);

    RenderSystem2D::Instance()->PopRenderTarget();

	visibilityToolProxy->UpdateVisibilityPointSet(true);
	visibilityToolProxy->UpdateRect(Rect(0.f, 0.f, visibilityToolSprite->GetWidth(), visibilityToolSprite->GetHeight()));
	visibilityToolProxy->SetVisibilityPoint(redoVisibilityPoint);
}


ActionSetVisibilityArea::ActionSetVisibilityArea(Image* originalImage,
												 VisibilityToolProxy* visibilityToolProxy,
												 const Rect& updatedRect)
:	CommandAction(CMDID_VISIBILITY_TOOL_SET_AREA, "Set Visibility Area")
{
	Image* currentImage = visibilityToolProxy->GetSprite()->GetTexture()->CreateImageFromMemory(RenderState::RENDERSTATE_2D_BLEND);

//	undoImage = Image::CopyImageRegion(originalImage, updatedRect);
	redoImage = Image::CopyImageRegion(currentImage, updatedRect);

	SafeRelease(currentImage);

	this->visibilityToolProxy = SafeRetain(visibilityToolProxy);
	this->updatedRect = updatedRect;
}

ActionSetVisibilityArea::~ActionSetVisibilityArea()
{
//	SafeRelease(undoImage);
	SafeRelease(redoImage);
	SafeRelease(visibilityToolProxy);
}

void ActionSetVisibilityArea::Redo()
{
	ApplyImage(redoImage);
}

//void ActionSetVisibilityArea::Undo()
//{
//	ApplyImage(undoImage);
//}

void ActionSetVisibilityArea::ApplyImage(DAVA::Image *image)
{
	Sprite* visibilityToolSprite = visibilityToolProxy->GetSprite();

	Texture* texture = Texture::CreateFromData(image->GetPixelFormat(), image->GetData(),
											   image->GetWidth(), image->GetHeight(), false);
	texture->GeneratePixelesation();
	Sprite* sprite = Sprite::CreateFromTexture(texture, 0, 0, (float32)image->GetWidth(), (float32)image->GetHeight());

    RenderSystem2D::Instance()->PushRenderTarget();
    RenderSystem2D::Instance()->SetRenderTarget(visibilityToolSprite);

    Rect rect = VirtualCoordinatesSystem::Instance()->ConvertPhysicalToVirtual(updatedRect);
    
    RenderSystem2D::Instance()->PushClip();
    RenderSystem2D::Instance()->SetClip(rect);
    
    RenderSystem2D::Instance()->Setup2DMatrices();
    RenderSystem2D::Instance()->UpdateClip();

	RenderManager::Instance()->ClearWithColor(0.f, 0.f, 0.f, 0.f);
    
    Sprite::DrawState drawState;
    drawState.SetPosition(rect.x, rect.y);
    RenderSystem2D::Instance()->Draw(sprite, &drawState);

    RenderSystem2D::Instance()->PopClip();
    RenderSystem2D::Instance()->PopRenderTarget();

	visibilityToolProxy->UpdateRect(updatedRect);

	SafeRelease(texture);
	SafeRelease(sprite);
}
