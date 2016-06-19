/* 

Amine Rehioui
Created: April 10th 2010

*/	

#include "ShootEditorCommon.h"
#include "wxCommonProperties.h"
#include "wxArrayProperty.h"
#include "wxStructProperty.h"
#include "wxLinkProperty.h"
#include "PropertyLocator.h"
#include "wxReferenceProperty.h"
#include "wxEventProperty.h"

#include "PropertyList.h"
#include "EditorUtils.h"
#include "ObjectSelector.h"

namespace shoot
{
	// Define event table
	BEGIN_EVENT_TABLE(PropertyList, wxPropertyGrid)
		EVT_PG_CHANGED( wxID_ANY, PropertyList::OnPropertyGridChange )
		EVT_PG_CHANGING( wxID_ANY, PropertyList::OnPropertyGridChanging )
		EVT_PG_SELECTED( wxID_ANY, PropertyList::OnPropertySelected )
		EVT_PG_RIGHT_CLICK( wxID_ANY, PropertyList::OnPropertyRightClick )
	END_EVENT_TABLE()

	//! Constructor
	PropertyList::PropertyList(wxWindow* pParent,
							   PropertyListListener* pListener,
							   wxWindowID id /*= wxID_ANY*/, 
							   const wxPoint& pos /*= wxDefaultPosition*/, 
							   const wxSize& size /*= wxSize(250, 300)*/, 
							   long style /*= wxPG_SPLITTER_AUTO_CENTER | wxPG_BOLD_MODIFIED*/, 
							   const wxString& name /*= wxPropertyGridNameStr*/) 
	: super(pParent, id, pos, size, style, name)
	, m_pListener(pListener)
	, m_pSelectedObject(NULL)
	{
		// register custom property editors
		wxPGRegisterEditorClass( ArrayPropertyEditor );
		wxPGRegisterEditorClass( ReferencePropertyEditor );
	}

	//! Fills the list with the properties from an object
	void PropertyList::Fill(Object* pObject)
	{
		this->Clear();

		if(pObject)
		{
			PropertyStream stream(shoot::SM_Write);
			pObject->Serialize(stream);

			// show the type as a disabled properties
			wxPGProperty* pWxType = Append(new wxStringProperty("Type", wxPG_LABEL, pObject->GetClassName()));
			DisableProperty(pWxType);

			for(u32 i=0; i<stream.GetNumProperties(); ++i)
			{
				IProperty* pProperty = stream.GetProperty(i);
				wxPGProperty* pWxProperty = GetWxProperty(pProperty, this);
				if(pWxProperty)
				{
					Append(pWxProperty);
					
					if(Entity* pEntity = DYNAMIC_CAST(pObject, Entity))
					{
						// disable property if it is part of a template instance and override is off 
						if(!pEntity->GetTemplatePath().empty())
						{
							bool bOverriden = (pEntity->GetOverridenProperties().GetProperty(pProperty->GetName()) != NULL);
							if(!bOverriden)
							{
								DisableProperty(pWxProperty);
							}
						}						
					}
					//else if(Component* pComponent = DYNAMIC_CAST(pObject, Component))
					//{
					//	// disable property if the component's owner is a template instance
					//	if(!pComponent->GetParent()->GetTemplatePath().empty())
					//	{
					//		DisableProperty(pWxProperty);
					//	}
					//}

					if(pProperty->GetType() == PT_Array
					|| pProperty->GetType() == PT_Reference
					|| pProperty->GetType() == PT_Struct)
					{
						FillArrayProperties(pProperty, pWxProperty);
					}

					if(pProperty->GetName() == "ID")
					{
						pWxProperty->SetAttribute(wxPG_UINT_BASE, wxPG_BASE_HEX);
						pWxProperty->SetFlag(wxPG_PROP_DISABLED);
					}
				}
			}
		}

		m_pSelectedObject = pObject;
	}

	//! creates a wx property from a shoot property
	wxPGProperty* PropertyList::GetWxProperty(IProperty* pProperty, PropertyList* pInstance)
	{
		wxPGProperty* pWxProperty = NULL;

		switch(pProperty->GetType())
		{
		case PT_Int:
			{
				s32 value;
				pProperty->GetData(&value);
				pWxProperty = new wxIntProperty(pProperty->GetName(), wxPG_LABEL, value);
			}					
			break;

		case PT_UInt:
			{
				u32 value;
				pProperty->GetData(&value);
				pWxProperty = new wxUIntProperty(pProperty->GetName(), wxPG_LABEL, value);
			}					
			break;

		case PT_Float:
			{
				f32 value;
				pProperty->GetData(&value);
				pWxProperty = new wxFloatProperty(pProperty->GetName(), wxPG_LABEL, value);						
				pWxProperty->SetAttribute(wxPG_FLOAT_PRECISION, 2);
			}	
			break;

		case PT_String:
			{
				std::string value;
				pProperty->GetData(&value);
				pWxProperty = new wxStringProperty(pProperty->GetName(), wxPG_LABEL, value);					
			}					
			break;

		case PT_Bool:
			{
				bool value;
				pProperty->GetData(&value);
				pWxProperty = new wxBoolProperty(pProperty->GetName(), wxPG_LABEL, value);
				pWxProperty->SetAttribute(wxPG_BOOL_USE_CHECKBOX, true);
			}					
			break;

		case PT_Vec2:
			{
				Vector2 value;
				pProperty->GetData(&value);
				pWxProperty = new wxVector2Property(pProperty->GetName(), wxPG_LABEL, value);
				pWxProperty->Item(0)->SetAttribute(wxPG_FLOAT_PRECISION, 2);
				pWxProperty->Item(1)->SetAttribute(wxPG_FLOAT_PRECISION, 2);
			}
			break;

		case PT_Vec3:
			{
				Vector3 value;
				pProperty->GetData(&value);
				pWxProperty = new wxVector3Property(pProperty->GetName(), wxPG_LABEL, value);
				pWxProperty->Item(0)->SetAttribute(wxPG_FLOAT_PRECISION, 2);
				pWxProperty->Item(1)->SetAttribute(wxPG_FLOAT_PRECISION, 2);
				pWxProperty->Item(2)->SetAttribute(wxPG_FLOAT_PRECISION, 2);
			}
			break;

		case PT_Vec4:
			{
				Vector4 value;
				pProperty->GetData(&value);
				pWxProperty = new wxVector4Property(pProperty->GetName(), wxPG_LABEL, value);
				pWxProperty->Item(0)->SetAttribute(wxPG_FLOAT_PRECISION, 2);
				pWxProperty->Item(1)->SetAttribute(wxPG_FLOAT_PRECISION, 2);
				pWxProperty->Item(2)->SetAttribute(wxPG_FLOAT_PRECISION, 2);
				pWxProperty->Item(3)->SetAttribute(wxPG_FLOAT_PRECISION, 2);
			}
			break;

		case PT_Point:
			{
				Point value;
				pProperty->GetData(&value);
				pWxProperty = new wxPointProperty(pProperty->GetName(), wxPG_LABEL, value);
			}
			break;				

		case PT_AABBox2D:
			{						
				AABBox2D value;
				pProperty->GetData(&value);
				pWxProperty = new wxAABBox2DProperty(pProperty->GetName(), wxPG_LABEL, value);															
			}
			break;

		case PT_AABBox3D:
			{						
				AABBox3D value;
				pProperty->GetData(&value);
				pWxProperty = new wxAABBox3DProperty(pProperty->GetName(), wxPG_LABEL, value);															
			}
			break;

		case PT_File:
			{
				FileProperty* pFileProperty = static_cast<FileProperty*>(pProperty);
				std::string value;
				pFileProperty->GetData(&value);

				pWxProperty = new wxFileProperty(pProperty->GetName(), wxPG_LABEL, value);

				std::string filter = "All Files (*.*)|*.*";
				if(pFileProperty->GetFilter())
				{
					filter = filter + std::string("|") + pFileProperty->GetFilter();
				}

				pWxProperty->SetAttribute(wxPG_FILE_WILDCARD, filter.c_str());
				pWxProperty->SetAttribute(wxPG_FILE_DIALOG_TITLE, wxT("Open File"));
				pWxProperty->SetAttribute(wxPG_FILE_SHOW_FULL_PATH, false);
				pWxProperty->SetAttribute(wxPG_FILE_SHOW_RELATIVE_PATH, ".");				
			}
			break;

		case PT_Enum:
			{
				EnumProperty* pEnumProperty = static_cast<EnumProperty*>(pProperty);						
				s32 value;
				pProperty->GetData(&value);				
				pWxProperty = new wxEnumProperty(pProperty->GetName(), wxPG_LABEL, (const wxChar**)pEnumProperty->GetLiterals(), (const long*)pEnumProperty->GetValues(), value);
			}
			break;

		case PT_Color:
			{
				Color value;
				pProperty->GetData(&value);
				// TODO: implement/get RGBA dialog editor in wxWidgets - edit alpha separately for now
				wxColour wxCol(value.R*255.0f, value.G*255.0f, value.B*255.0f, 0);
				pWxProperty = new wxColourProperty(pProperty->GetName(), wxPG_LABEL, wxCol);
			}
			break;

		case PT_Size:
			{
				Size value;
				pProperty->GetData(&value);
				pWxProperty = new wxSizeProperty(pProperty->GetName(), wxPG_LABEL, value);
			}
			break;

		case PT_Link:
			{						
				std::string strValue;
				Link<Entity> link;
				pProperty->GetData(&link);
				if(link.GetObjectID() > 0)
				{
					Entity* pEntity = Engine::Instance()->GetContextStack();							
					if(link.GetObjectID() != pEntity->GetID())
					{
						pEntity = pEntity->GetChildByID(link.GetObjectID());
					}
					SHOOT_ASSERT(pEntity, "Entity link pointing to unexisting entity");
					std::stringstream ss;
					ss << pEntity->GetName() << " [" << std::hex << link.GetObjectID() << "]";					
					strValue = ss.str();
				}				 
				LinkProperty* pLinkProperty = static_cast<LinkProperty*>(pProperty);
				wxLinkProperty* pWxLinkProperty = new wxLinkProperty(pProperty->GetName(), wxPG_LABEL, strValue);				
				pWxLinkProperty->SetLinkID(link.GetObjectID());
				pWxLinkProperty->SetBaseType(pLinkProperty->GetLink()->GetClassName());
				pWxProperty = pWxLinkProperty;
			}					
			break;

		case PT_Struct:
			{
				StructProperty* pStructProperty = static_cast<StructProperty*>(pProperty);
				pWxProperty = new wxStructProperty(pInstance, pProperty->GetName(), wxPG_LABEL, pStructProperty->GetStream());
			}
			break;

		case PT_Array:
			{
				ArrayProperty* pArrayProperty = static_cast<ArrayProperty*>(pProperty);
				pWxProperty = new wxArrayProperty(pProperty->GetName(), pArrayProperty, pInstance);
				pInstance->SetPropertyEditor(pWxProperty, wxPG_EDITOR(ArrayPropertyEditor));
			}
			break;

		case PT_Reference:
			{
				ReferenceProperty* pRefProperty = static_cast<ReferenceProperty*>(pProperty);
				pWxProperty = new wxReferenceProperty(pProperty->GetName(), pRefProperty, pInstance);
				pInstance->SetPropertyEditor(pWxProperty, wxPG_EDITOR(ReferencePropertyEditor));
			}
			break;

		case PT_Range:
			{
				Range value;
				pProperty->GetData(&value);
				pWxProperty = new wxRangeProperty(pProperty->GetName(), wxPG_LABEL, value);
			}
			break;

		case PT_Event:
			{
				EventProperty* pEventProperty = static_cast<EventProperty*>(pProperty);
				wxEventProperty* pWxEventProperty = new wxEventProperty(pProperty->GetName(), wxPG_LABEL, "[Event Trigger]");
				pWxEventProperty->SetEventName(pEventProperty->GetString());
				pWxProperty = pWxEventProperty;
			}
			break;
		}

		return pWxProperty;
	}

	//! fills a shoot property from a wx property
	void PropertyList::FillProperty(wxPGProperty* pWxProperty, IProperty* pProperty)
	{
		switch(pProperty->GetType())
		{				
		case PT_String:	
			{
				std::string value = pWxProperty->GetValue().GetString().c_str();
				pProperty->SetData(value);	
			}
			break;

		case PT_File:
			{
				std::string value = pWxProperty->GetValue().GetString().c_str();
					
				if(value.length() > 0)
				{
					// make path relative to data OR audio folders
					std::string rootFolder = "data";
					s32 rootFolderLength = 4;
					size_t rootKeywordIndex = value.find(rootFolder);
					if(rootKeywordIndex == std::string::npos)
					{
						rootFolder = "audio";
						rootFolderLength = 5;
						rootKeywordIndex = value.find(rootFolder);
					}

					if(rootKeywordIndex != std::string::npos)
					{
						value = value.substr(rootKeywordIndex+rootFolderLength+1);						
					}

					// force forward slashes
					wxString wxStr(value);
					wxStr.Replace("\\", "/");
					value = wxStr.c_str();
				}

				pProperty->SetData(value);
			}
			break;

		case PT_Int:
		case PT_Enum:
			{
				s32 value = pWxProperty->GetValue().GetInteger();
				pProperty->SetData(&value);
			}
			break;

		case PT_UInt:
			{
				u32 value = pWxProperty->GetValue().GetInteger();
				pProperty->SetData(&value);
			}
			break;

		case PT_Float:
			{
				f32 value = pWxProperty->GetValue().GetDouble();
				pProperty->SetData(&value);
			}
			break;

		case PT_Bool:
			{
				bool bValue = pWxProperty->GetValue().GetBool();
				pProperty->SetData(&bValue);
			}
			break;

		case PT_Vec2:
			{
				Vector2 vec = Vector2FromVariant(pWxProperty->GetValue());
				pProperty->SetData(&vec);
			}
			break;

		case PT_Vec3:
			{
				Vector3 vec = Vector3FromVariant(pWxProperty->GetValue());
				pProperty->SetData(&vec);
			}
			break;

		case PT_Vec4:
			{
				Vector4 vec = Vector4FromVariant(pWxProperty->GetValue());
				pProperty->SetData(&vec);
			}
			break;

		case PT_Point:
			{
				Point point = PointFromVariant(pWxProperty->GetValue());
				pProperty->SetData(&point);
			}
			break;

		case PT_AABBox2D:
			{
				AABBox2D box = AABBox2DFromVariant(pWxProperty->GetValue());						
				pProperty->SetData(&box);
			}
			break;

		case PT_AABBox3D:
			{
				AABBox3D box = AABBox3DFromVariant(pWxProperty->GetValue());						
				pProperty->SetData(&box);
			}
			break;

		case PT_Color:
			{	
				wxColour wxCol;
				wxCol << pWxProperty->GetValue();
				Color color(f32(wxCol.Red())/255.0f, f32(wxCol.Green())/255.0f, f32(wxCol.Blue())/255.0f);
				pProperty->SetData(&color);
			}
			break;

		case PT_Size:
			{
				Size size = SizeFromVariant(pWxProperty->GetValue());
				pProperty->SetData(&size);
			}
			break;

		case PT_Struct:
			{
				wxStructProperty* pWxStructProperty = static_cast<wxStructProperty*>(pWxProperty);
				StructProperty* pStructProperty = static_cast<StructProperty*>(pProperty);
				PropertyStream& stream = pStructProperty->GetStream();
				for(u32 i=0; i<stream.GetNumProperties(); ++i)
				{
					IProperty* pSubProperty = stream.GetProperty(i);
					wxPGProperty* pSubWxProperty = pWxStructProperty->Item(i);
					FillProperty(pSubWxProperty, pSubProperty); 
				}				
			}
			break;

		case PT_Array:
			{
				wxArrayProperty* pWxArrayProperty = static_cast<wxArrayProperty*>(pWxProperty);
				ArrayProperty* pArrayProperty = static_cast<ArrayProperty*>(pProperty);
				switch(pWxArrayProperty->GetEventType())
				{
				case wxArrayProperty::E_PropertyAdded:
					{
						s32 childrenCount = pWxArrayProperty->GetChildCount();
						SHOOT_ASSERT(childrenCount > 0, "wxArrayProperty::E_PropertyAdded event triggered on empty array");
						wxPGProperty* pNewWxProperty = pWxArrayProperty->Item(childrenCount-1);
						IProperty* pNewProperty = IProperty::CreateFromType(pWxArrayProperty->GetSubPropertyTemplate()->GetType());
						pNewProperty->SetName(pWxArrayProperty->GetSubPropertyTemplate()->GetName());

						// in case the sub property is a struct, initialze its stream with properties
						if(pNewProperty->GetType() == PT_Struct)
						{
							StructProperty* pStructProperty = static_cast<StructProperty*>(pNewProperty);
							pStructProperty->GetStream().SetMode(SM_Write);
							pWxArrayProperty->GetStructTemplate()->Serialize(pStructProperty->GetStream());
						}

						FillProperty(pNewWxProperty, pNewProperty);
						pArrayProperty->GetProperties().push_back(pNewProperty);
					}
					break;

				case wxArrayProperty::E_PropertyRemoved:
					{
						IProperty* pProperty = pArrayProperty->GetProperties().back();
						pArrayProperty->GetProperties().pop_back();
						delete pProperty;
					}
					break;

				case wxArrayProperty::E_SubPropertyChanged:
					{
						s32 index = pWxArrayProperty->GetChangedSubPropertyIndex();
						IProperty* pSubProperty = pArrayProperty->GetProperties()[index];
						wxPGProperty* pSubWxProperty = pWxArrayProperty->Item(index);
						FillProperty(pSubWxProperty, pSubProperty); 						
					}
					break;
				}
				pWxArrayProperty->SetEventType(wxArrayProperty::E_None);
			}
			break;

		case PT_Reference:
			{
				wxReferenceProperty* pWxRefProperty = static_cast<wxReferenceProperty*>(pWxProperty);
				ReferenceProperty* pRefProperty = static_cast<ReferenceProperty*>(pProperty);

				switch(pWxRefProperty->GetEventType())
				{
				case wxReferenceProperty::E_PropertySetNew:
				case wxReferenceProperty::E_PropertySetFromTemplate:
					{
						IReference* pRefInterface = pRefProperty->GetRefInterface();
						Object* pObject = pRefInterface->Get();
						SHOOT_ASSERT(!pObject, "Reference already set");

						bool bAborted = false;
						if(pWxRefProperty->GetEventType() == wxReferenceProperty::E_PropertySetNew)
						{
							// pick between all possible child types				
							std::string className;
							ObjectSelector selector(pWxRefProperty->GetParent());
							if(selector.Select(pWxRefProperty->GetClassName(), className))
							{	
								pObject = ObjectFactory::Instance()->Create(className);
								pRefInterface->SetObject(pObject);
								pRefProperty->SetClassName(pObject->GetClassName());				
							}
							else
							{
								bAborted = true;
							}
						}
						else
						{
							pObject = ObjectManager::Instance()->Get(pWxRefProperty->GetTemplatePath().c_str(), true);							
							pRefInterface->SetObject(pObject);
							pRefProperty->SetTemplatePath(pWxRefProperty->GetTemplatePath());
						}

						if(!bAborted)
						{
							// fill the wx property
							PropertyStream& stream = pRefProperty->GetStream();
							stream.SetMode(SM_Write);
							pObject->Serialize(stream);
							pWxRefProperty->Fill(pRefProperty);

							// if this is an entity, initialize it
							if(Entity* pEntity = DYNAMIC_CAST(pObject, Entity))
							{
								pEntity->Init();
							}
						}
					}
					break;	

				case wxReferenceProperty::E_PropertyCleared:
					{						
						pRefProperty->GetRefInterface()->Clear();
						pRefProperty->GetStream().Clear();

						// restore class name to base
						pRefProperty->SetClassName(pRefProperty->GetRefInterface()->GetClassName());
						pRefProperty->SetTemplatePath("");

						pWxRefProperty->Fill(pRefProperty);
					}
					break;

				case wxReferenceProperty::E_SubPropertyChanged:
					{
						PropertyStream& stream = pRefProperty->GetStream();
						for(u32 i=0; i<stream.GetNumProperties(); ++i)
						{
							IProperty* pSubProperty = stream.GetProperty(i);
							wxPGProperty* pSubWxProperty = pWxRefProperty->Item(i);
							FillProperty(pSubWxProperty, pSubProperty); 
						}
					}
					break;

				case wxReferenceProperty::E_PropertySaved:
					{
						IReference* pRefInterface = pRefProperty->GetRefInterface();						
						wxReferenceProperty* pWxRefProperty = static_cast<wxReferenceProperty*>(pWxProperty);

						Object* pObject = pRefInterface->Get();
						SHOOT_ASSERT(pObject, "Reference has NULL object");
						std::string path = pObject->GetTemplatePath();
						bool bExists = !pObject->GetTemplatePath().empty();
						if(bExists || EditorUtils::OpenFileDialog(pWxRefProperty->GetParent(), path, "Object files (*.xml)|*.xml", "Save File", wxFD_SAVE))
						{
							EditorUtils::FormatObjectPath(pObject, path);	
							pObject->SaveToXML(path.c_str());							
						}
					}
					break;
				}

				pWxRefProperty->SetEventType(wxReferenceProperty::E_None);
			}
			break;

		case PT_Range:
			{
				Range range = RangeFromVariant(pWxProperty->GetValue());
				pProperty->SetData(&range);
			}
			break;

		case PT_Link:
			{
				wxLinkProperty* pWxLinkProperty = static_cast<wxLinkProperty*>(pWxProperty);
				u32 ID = pWxLinkProperty->GetLinkID();
				std::stringstream ss(std::stringstream::in | std::stringstream::out);
				ss << std::hex << ID;
				pProperty->SetData(ss.str());
			}
			break;
		}		
	}

	//! recursively fills sub array properties
	/** Array properties must be filled after they are added to the list */
	void PropertyList::FillArrayProperties(IProperty* pProperty, wxPGProperty* pWxProperty)
	{
		if(pProperty->GetType() == PT_Array)
		{
			ArrayProperty* pArrayProperty = static_cast<ArrayProperty*>(pProperty);
			wxArrayProperty* pWxArrayProperty = static_cast<wxArrayProperty*>(pWxProperty);
			pWxArrayProperty->Fill(pArrayProperty);

			for(u32 i=0; i<pArrayProperty->GetProperties().size(); ++i)
			{
				IProperty* pSubProperty = pArrayProperty->GetProperties()[i];
				FillArrayProperties(pSubProperty, pWxProperty->Item(i));
			}
		}
		else if(pProperty->GetType() == PT_Reference)
		{
			ReferenceProperty* pRefProperty = static_cast<ReferenceProperty*>(pProperty);
			wxReferenceProperty* pWxRefProperty = static_cast<wxReferenceProperty*>(pWxProperty);
			pWxRefProperty->Fill(pRefProperty);	
		}
		else if(pProperty->GetType() == PT_Struct)
		{
			StructProperty* pStructProperty = static_cast<StructProperty*>(pProperty);
			for(u32 i=0; i<pStructProperty->GetStream().GetNumProperties(); ++i)
			{
				IProperty* pSubProperty = pStructProperty->GetStream().GetProperty(i);
				FillArrayProperties(pSubProperty, pWxProperty->Item(i));
			}
		}
	}

	// Event handlers
	void PropertyList::OnPropertyGridChange(wxPropertyGridEvent &event)
	{		
		m_pListener->OnPropertyChanged(event);
	}

	void PropertyList::OnPropertyGridChanging(wxPropertyGridEvent &event)
	{  
		m_pListener->OnPropertyChanging(event);
	}

	void PropertyList::OnPropertySelected(wxPropertyGridEvent& event)
	{
		m_pListener->OnPropertySelected(event);
	}

	void PropertyList::OnPropertyRightClick(wxPropertyGridEvent& event)
	{
		m_pListener->OnPropertyRightClick(event);
	}
}

