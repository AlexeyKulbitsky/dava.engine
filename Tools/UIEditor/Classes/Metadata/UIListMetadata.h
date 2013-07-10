/*==================================================================================
    Copyright (c) 2008, DAVA, INC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA, INC nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA, INC AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL DAVA, INC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#ifndef __UIEditor__UIListMetadata__
#define __UIEditor__UIListMetadata__

#include "UIControlMetadata.h"
#include "UI/UIList.h"

namespace DAVA {

// Metadata class for DAVA UIList control.
class UIListMetadata : public UIControlMetadata
{
    Q_OBJECT
	
	Q_PROPERTY(int AggregatorID READ GetAggregatorID WRITE SetAggregatorID);
	Q_PROPERTY(int Orientation READ GetOrientation WRITE SetOrientation);

public:
    UIListMetadata(QObject* parent = 0);

protected:
    // Initialize the appropriate control.
    virtual void InitializeControl(const String& controlName, const Vector2& position);
    virtual void UpdateExtraData(HierarchyTreeNodeExtraData& extraData, eExtraDataUpdateStyle updateStyle);

    virtual QString GetUIControlClassName() { return "UIList"; };
	
    // Helper to access active UI List.
    UIList* GetActiveUIList() const;
	
	// Properties getters/setters
	int GetAggregatorID();
    void SetAggregatorID(int value);
	
	int GetOrientation();
	void SetOrientation(int value);
	
	virtual void SetActiveControlRect(const Rect& rect);

private:
	void UpdateListCellSize(const Rect& rect, UIList::eListOrientation orientation);
};

};

#endif /* defined(__UIEditor__UIListMetadata__) */
