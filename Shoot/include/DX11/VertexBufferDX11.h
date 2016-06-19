/* 

Amine Rehioui
Created: July 28th 2013

*/

#ifndef _VERTEX_BUFFER_DX11_H_INCLUDED_
#define _VERTEX_BUFFER_DX11_H_INCLUDED_

#include "VertexBuffer.h"

namespace shoot
{
	//! VertexBufferDX11
	class VertexBufferDX11 : public VertexBuffer
	{
	public:

		//! constructor
		VertexBufferDX11();

		//! destructor
		virtual ~VertexBufferDX11();

		//! Load into video memory
		void GraphicLoad();

		//! Unload from video memory
		void GraphicUnload();

		//! updates the vertex buffer data
		virtual void UpdateData();

		//! begins rendering using this vertex buffer
		virtual void Begin();

		//! default render method
		virtual void Draw();

		//! instanced render method
		virtual void Draw(const std::vector<Matrix44>& aWorldTransforms, const std::vector<Matrix44>& aTextureTransforms, Material* pMaterial);

		//! ends rendering by this vertex buffer
		virtual void End();

		//! create a copy of the object
		virtual Object* Copy() const;

	protected:

		//! creates a vertex buffer
		ID3D11Buffer* CreateVertexBuffer();

		u32 m_AllocatedVerticesDX11;
		COMReference<ID3D11Buffer> m_VB;
		COMReference<ID3D11Buffer> m_IB;
	};
}

#endif // _VERTEX_BUFFER_DX11_H_INCLUDED_

