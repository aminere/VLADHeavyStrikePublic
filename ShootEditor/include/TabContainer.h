/* 

Amine Rehioui
Created: March 13th 2011

*/

#ifndef _TAB_CONTAINER_H_INCLUDED_
#define _TAB_CONTAINER_H_INCLUDED_

#include <wx/combobox.h>

#include "ViewPortContainer.h"

namespace shoot
{
	// forwards
	class Event;

	//! Tab Container
	class TabContainer : public wxPanel							   
	{
		typedef wxPanel super;

	public:
		
		//! Camera selector type
		enum E_CameraSelector
		{
			CS_3D,
			CS_2D,
			CS_Count
		};

		//! Constructor
		TabContainer(wxWindow* pParent,
					 wxGLContext* pGLContext = NULL,
					 wxWindowID id = wxID_ANY, 
					 const wxPoint& pos = wxDefaultPosition, 
					 const wxSize& size = wxSize(800, 600), 
					 long style = 0);

		//! Destructor
		virtual ~TabContainer();

		//! returns the view port container
		ViewPortContainer* GetViewPortContainer() { return m_pViewPortContainer; }

		//! returns the selected entity
		Entity* GetSelectedEntity() { return m_SelectedEntity; }

		//! sets the selected entity
		void SetSelectedEntity(Entity* pEntity) { m_SelectedEntity = pEntity; }

		//! called when the tab is just selected
		void OnSelected();

		//! called when the tab is about to close, return false to forbid closing
		bool OnClose();

		//! updates a camera selectors
		void UpdateCameraSelectors();

		//! reloads the current level
		void Reload();

		//! loads a level
		void Load(const std::string& strPath);

		// event handlers
		void OnNew(wxCommandEvent& event);
		void OnOpen(wxCommandEvent& event);
		void OnSave(wxCommandEvent& event);
		void OnSaveAs(wxCommandEvent& event);		
		void OnReloadLastSave(wxCommandEvent& event);
		void On3DCameraChanged(wxCommandEvent& event);
		void On2DCameraChanged(wxCommandEvent& event);
		void OnResetView(wxCommandEvent& event);
		void OnClearFade(wxCommandEvent& event);
		void OnToggleBlackBars(wxCommandEvent& event);

		DECLARE_EVENT_TABLE();

	private:

		//! sets tab name
		void SetTabName(const std::string& name);

		//! sets the current project path
		void SetProjectPath(const std::string& path);

		ViewPortContainer* m_pViewPortContainer;		

		std::string m_CurrentProjectPath;

		wxComboBox* m_pCameraSelectors[CS_Count];

		Camera* m_p3DCamera;
		Camera* m_p2DCamera;

		Handle<Entity> m_SelectedEntity;
	};
}

#endif // _TAB_CONTAINER_H_INCLUDED_

