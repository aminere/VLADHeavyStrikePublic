/* 

Amine Rehioui
Created: April 18th 2010

*/

#ifndef _RESOURCE_WINDOW_H_INCLUDED_
#define _RESOURCE_WINDOW_H_INCLUDED_

#include <wx/notebook.h>

namespace shoot
{
	//! A class to manage the resources allocated by the engine
	class ResourceWindow : public wxNotebook
	{
		typedef wxNotebook super;

	public:
		
		//! Constructor
		ResourceWindow(wxWindow* pParent,	
					   wxWindowID id = wxID_ANY, 
					   const wxPoint& pos = wxDefaultPosition, 
					   const wxSize& size = wxSize(250, 300), 
					   long style = 0);

		//! Destructor
		virtual ~ResourceWindow()
		{
		}

	private:

		DECLARE_EVENT_TABLE();  
	};
}

#endif // _RESOURCE_WINDOW_H_INCLUDED_


