/* 

Amine Rehioui
Created: May 24th 2010

*/

#include "ShootEditorCommon.h"

#include "wxLinkProperty.h"

#include "EntitySelector.h"

//! static variables initialization
wxLinkProperty* wxLinkProperty::ms_pCurrentProperty = NULL;

// -----------------------------------------------------------------------
// wxLinkProperty
// -----------------------------------------------------------------------
WX_PG_IMPLEMENT_PROPERTY_CLASS(wxLinkProperty, wxPGProperty, wxString, const wxString&, TextCtrlAndButton)

//! button click callback
bool wxLinkProperty::OnButtonClick(wxPropertyGrid* propgrid, wxString& value)
{
	ms_pCurrentProperty = this;	

    EntitySelector selector(propgrid, OnEntityLinkChanged, m_strBaseType);
    selector.ShowModal();

	ms_pCurrentProperty = NULL;	
    return true;
}

//! is called after an the entity link has been changed
/** return true to close the EntitySelector */
bool wxLinkProperty::OnEntityLinkChanged(u32 entityID)
{
	wxLinkProperty* pWxLinkProperty = wxLinkProperty::GetCurrentProperty();
	pWxLinkProperty->SetLinkID(entityID);	

	Entity* pEntity = Engine::Instance()->GetContextStack();
	if(entityID >= 0)
	{
		if(entityID != pEntity->GetID())
		{
			pEntity = pEntity->GetChildByID(entityID);
		}		
	}
	else
	{
		pEntity = NULL;
	}

	// Update the link property
	if(pEntity)
	{
		std::stringstream ss;
		ss << pEntity->GetName() << " [" << std::hex << entityID << "]";		
		pWxLinkProperty->SetValue(wxString(ss.str()));
	}
	else
	{
		pWxLinkProperty->SetValue("");
	}

	// notify editor
	wxPropertyGridEvent event;
	event.SetProperty(pWxLinkProperty);
	ShootEditor::Instance()->GetObjectInspector()->OnPropertyChanged(event);
	return true;
}

