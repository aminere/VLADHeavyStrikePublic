/* 

Amine Rehioui
Created: April 10th 2010

*/		

#include "ShootEditorCommon.h"

#include "wxCommonProperties.h" // Vector2ToVariant

#include "CameraManager.h"

#include "EditorSettings.h"

#include "Entity3DController.h"

#include "EditorUtils.h"

#include "CollisionUtils.h"

#include "UndoManager.h"

#include "Plane.h"
#include "Triangle.h"
#include "ParticleGenerator.h"
#include "EditorRenderer.h"
#include "SkyBoxEntity.h"
#include "MeshEntity.h"

namespace shoot
{
	// Define event table
	BEGIN_EVENT_TABLE(ViewPort, wxGLCanvas)
		EVT_MOTION(ViewPort::OnMouseMove)
		EVT_LEFT_DOWN(ViewPort::OnMouseLeftDown)
		EVT_LEFT_UP(ViewPort::OnMouseLeftUp)
		EVT_MIDDLE_DOWN(ViewPort::OnMouseMiddleDown)
		EVT_MIDDLE_UP(ViewPort::OnMouseMiddleUp)
		EVT_RIGHT_DOWN(ViewPort::OnMouseRightDown)
		EVT_RIGHT_UP(ViewPort::OnMouseRightUp)
		EVT_MOUSEWHEEL(ViewPort::OnMouseWheelRolled)
		EVT_KEY_DOWN(ViewPort::OnKeyDown)
		EVT_KEY_UP(ViewPort::OnKeyUp)
		EVT_LEAVE_WINDOW(ViewPort::OnMouseLeave)
		EVT_SIZE(ViewPort::OnResized)
		EVT_SET_FOCUS(ViewPort::OnFocusGained)
		EVT_KILL_FOCUS(ViewPort::OnFocusLost)
		EVT_PAINT(ViewPort::OnPaint)
		EVT_ERASE_BACKGROUND(ViewPort::OnEraseBackground)
	END_EVENT_TABLE()

	//! Constructor
	ViewPort::ViewPort(wxWindow *parent)
	: super(parent, wxID_ANY, NULL, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE)
	, m_bDraggingStarted(false)
	, m_bZoomingStarted(false)
	, m_bStrafingStarted(false)
	, m_bLookingStarted(false)
	, m_pGLContext(NULL)
	{	
		m_Default3DCamera = snew Camera();
		m_Default3DCamera->SetZNear(.1f);
		m_Default3DCamera->SetZFar(1000.0f);

		m_Default2DCamera = snew Camera();

#ifndef DX11
		m_pGLContext = new wxGLContext(this);
		m_pGLContext->SetCurrent(*this);
#endif // DX11

		ResetView();
	}

	// Destructor
	ViewPort::~ViewPort()
	{
	}

	//! returns the 2D entity under a specific point
	Entity2D* ViewPort::GetSelectedEntity(Entity* pParent, const Vector2& clickPosition)
	{
		// check children first
		s32 numChildren = pParent->GetChildCount();
		for(s32 i=numChildren-1; i>=0; --i)
		{
			Entity2D* pSelectedEntity = GetSelectedEntity(pParent->GetChild(i), clickPosition);
			if(pSelectedEntity)
			{
				return pSelectedEntity;
			}
		}

		// check entity - if we reached the root just return NULL
		if(pParent->GetParent() != NULL)
		{
			if(Entity2D* pEntity2D = DYNAMIC_CAST(pParent, Entity2D))
			{
				// Take camera transformation into account
				Camera* pCamera2D = EntityRenderer::Instance()->Get2DCamera();
				Vector3 vCameraTranslation = pCamera2D->GetViewMatrix().GetTranslation();
				Vector3 vCameraScale = pCamera2D->GetViewMatrix().GetScale();
				Vector2 localClickPos = clickPosition - Vector2(vCameraTranslation.X, vCameraTranslation.Y);
				localClickPos.Set(localClickPos.X/vCameraScale.X, localClickPos.Y/vCameraScale.Y);

				Matrix44 matrix = pEntity2D->GetTransformationMatrix();
				Matrix44 inverse;
				if(matrix.GetInverse(inverse))
				{
					Vector3 invClickPos3D = inverse.TransformVect(Vector3::Create(localClickPos.X, localClickPos.Y, 0.0f));				
					Vector2 invClickPos(invClickPos3D.X, invClickPos3D.Y);
					if(pEntity2D->GetBoundingBox().Contains(invClickPos))
					{
						return pEntity2D;
					}
				}
			}
		}

		return NULL;
	}

	//! returns the 3D entity selected by a 3D ray
	void ViewPort::GetSelectedEntity(Entity* pParent, const Vector3& vRayStart, const Vector3& vRayDir, Entity3D*& pResult)
	{
		Camera* pCamera = EntityRenderer::Instance()->Get3DCamera();
		if(!pCamera || pCamera == pParent || pParent->IsA(SkyBoxEntity::TypeID))
		{
			return;
		}

		Entity3D* pEntity3D = DYNAMIC_CAST(pParent, Entity3D);
		if(pEntity3D)
		{
			GraphicComponent* pGraphic = pEntity3D->GetComponent<GraphicComponent>();
			VertexBuffer* pVertexBuffer = pGraphic ? pGraphic->GetVertexBuffer() : NULL;

			if(pVertexBuffer
			&& pVertexBuffer->GetPrimitiveType() == GraphicsDriver::PT_Triangle)
			{
				Matrix44 matrix = pEntity3D->GetTransformationMatrix();
				u16* pIndices = pVertexBuffer->GetIndices();
				Vertex3D* pVertices = pVertexBuffer->GetVertices();
				u32 numTriangles = pIndices ? (pVertexBuffer->GetNumIndices()/3) : (pVertexBuffer->GetNumVertices()/3);

				for(u32 i=0; i<numTriangles; ++i)
				{
					u32 index1 = pIndices ? pIndices[i*3 + 0] : (i*3 + 0);
					u32 index2 = pIndices ? pIndices[i*3 + 1] : (i*3 + 1);
					u32 index3 = pIndices ? pIndices[i*3 + 2] : (i*3 + 2);

					Vector3 v1 = matrix.TransformVect(pVertexBuffer->GetVertices()[index1].Pos);
					Vector3 v2 = matrix.TransformVect(pVertexBuffer->GetVertices()[index2].Pos);
					Vector3 v3 = matrix.TransformVect(pVertexBuffer->GetVertices()[index3].Pos);

					Plane plane(v1, v2, v3);
					Vector3 vIntersection;
					Plane::E_Classification eClass;
					if(plane.IntersectWithRay(vRayStart, vRayDir, &vIntersection, &eClass))
					{
						if(eClass == Plane::C_Front)
						{
							Triangle triangle(v1, v2, v3);
							if(triangle.IsPointInside(vIntersection))
							{
								Vector3 vDirToIntersection = vIntersection - vRayStart;
								f32 dist = vDirToIntersection.GetLength();
								vDirToIntersection = vDirToIntersection / dist; // normalize

								if((!pResult || dist < m_fDistToClosestPickedEntity)
									&& vDirToIntersection.DotProduct(vRayDir) > 0.0f) // only consider intersections in front of the camera
								{
									pResult = pEntity3D;
									m_fDistToClosestPickedEntity = dist;
									EditorRenderer::Instance()->SetPickingInfo(vRayStart, vRayDir, vIntersection);
									EditorRenderer::Instance()->SetPickingInfoValid(true);
									break;
								}
							}
						}
					}
				}
			}
			else
			{
				AABBox3D box(-Vector3::One/2.0f, Vector3::One/2.0f);

				if(ParticleGenerator* pParticleGenerator = DYNAMIC_CAST(pEntity3D, ParticleGenerator))
				{
					box = pParticleGenerator->GetBoundingBox();
				}

				// Take entity transformation into account
				Matrix44 matrix = pEntity3D->GetTransformationMatrix();
				Matrix44 inverse;
				if(matrix.GetInverse(inverse))
				{
					Vector3 localRayStart, localRayEnd;
					localRayStart = inverse.TransformVect(vRayStart);
					localRayEnd = inverse.TransformVect(vRayStart + vRayDir * 999.0f);	
					Vector3 vIntersection;
					if(CollisionUtils::AABBox3DIntersectsWithRay(box, localRayStart, localRayEnd, &vIntersection))
					{
						Vector3 vRealIntersection = matrix.TransformVect(vIntersection);
						Vector3 vDirToIntersection = vRealIntersection - vRayStart;
						f32 dist = vDirToIntersection.GetLength();
						vDirToIntersection = vDirToIntersection / dist; // normalize

						if((!pResult || dist < m_fDistToClosestPickedEntity)
						&& vDirToIntersection.DotProduct(vRayDir) > 0.0f) // only consider intersections in front of the camera
						{
							pResult = pEntity3D;
							m_fDistToClosestPickedEntity = dist;
							EditorRenderer::Instance()->SetPickingInfo(vRayStart, vRayDir, vRealIntersection);
							EditorRenderer::Instance()->SetPickingInfoValid(true);
						}
					}
				}
			}
		}

		// check children
		s32 numChildren = pParent->GetChildCount();
		for(u32 i=0; i<pParent->GetChildCount(); ++i)
		{
			GetSelectedEntity(pParent->GetChild(i), vRayStart, vRayDir, pResult);				
		}
	}

	//! checks if camera move is allowed in the current frame
	bool ViewPort::IsCameraMoveAllowed()
	{
		bool b3DCameraControl = ShootEditor::Instance()->Get3DControl();
		bool bGameCameraControl = CameraManager::Instance()->GetGameCameraControl();
		bool bIs3DGameCamera = (EntityRenderer::Instance()->Get3DCamera() != m_Default3DCamera);
		bool bIs2DGameCamera = (EntityRenderer::Instance()->Get2DCamera() != m_Default2DCamera);

		if(m_bStrafingStarted || m_bZoomingStarted || m_bLookingStarted)
		{
			if(!bGameCameraControl)
			{
				if(b3DCameraControl || m_bLookingStarted)
				{
					if(bIs3DGameCamera)
					{
						PropertyStream stream(SM_Write);
						std::string name = m_Default3DCamera->GetName();
						EntityRenderer::Instance()->Get3DCamera()->Serialize(stream);
						stream.SetMode(SM_Read);
						m_Default3DCamera->Serialize(stream);
						m_Default3DCamera->SetName(name);
						m_Default3DCamera->SetZNear(.1f);
						m_Default3DCamera->SetZFar(1000.0f);
						EntityRenderer::Instance()->Set3DCamera(m_Default3DCamera);
						ShootEditor::Instance()->UpdateCameraSelectors();
						return false;
					}
				}
				else
				{
					if(bIs2DGameCamera)
					{
						PropertyStream stream(SM_Write);
						std::string name = m_Default2DCamera->GetName();
						EntityRenderer::Instance()->Get2DCamera()->Serialize(stream);
						stream.SetMode(SM_Read);
						m_Default2DCamera->Serialize(stream);
						m_Default2DCamera->SetName(name);
						EntityRenderer::Instance()->Set2DCamera(m_Default2DCamera);
						ShootEditor::Instance()->UpdateCameraSelectors();
						return false;
					}
				}
			}
		}
		return true;
	}

	//! resets the view
	void ViewPort::ResetView()
	{
		m_Default3DCamera = snew Camera();
		m_Default3DCamera->SetPosition(Vector3::Create(0.0f, -10.0f, 0.0f));
		m_Default3DCamera->SetZNear(.1f);
		m_Default3DCamera->SetZFar(1000.0f);
		m_Default2DCamera = snew Camera();
		m_Default2DCamera->SetType(Camera::Type_Ortho2D);
	}

	// event handlers
	void ViewPort::OnMouseMove(wxMouseEvent& event)
	{
		if(!EntityRenderer::Instance()->Get3DCamera()
		|| !EntityRenderer::Instance()->Get2DCamera()
		|| !IsCameraMoveAllowed()
		|| Engine::Instance()->IsLoading())
		{
			return;
		}

		if(ShootEditor::Instance()->IsEngineRunning())
		{
			if(event.MiddleIsDown())
			{
				InputManager::TouchState state;
				state.bPressed = true;
				state.vPosition.Set(event.GetX(), event.GetY());
				InputManager::Instance()->SetTouchStateAsync(state);
			}
		}

		// Update the status bar
		char strPosition[128];
		std::stringstream ss;
		ss << event.GetX() << ", " << event.GetY();
		ss.get(strPosition, 128);
		static_cast<wxFrame*>(wxTheApp->GetTopWindow())->SetStatusText(strPosition, 1);

		bool b3DCameraControl = ShootEditor::Instance()->Get3DControl();
		Camera* pTarget3DCamera = EntityRenderer::Instance()->Get3DCamera();
		Camera* pTarget2DCamera = EntityRenderer::Instance()->Get2DCamera();

		Vector2 mousePos(event.GetX(), event.GetY());
		Vector2 deltaPos = mousePos - m_LastMousePosition;
		ShootEditor::E_ControlMode eControlMode = ShootEditor::Instance()->GetControlMode();
		Entity* pSelectedEntity = ShootEditor::Instance()->GetSelectedEntity();

		// if rotating/scaling a 2D entity, keep original mouse pos
		if(m_bDraggingStarted
		&& pSelectedEntity && pSelectedEntity->IsA(Entity2D::TypeID)
		&& (eControlMode == ShootEditor::ControlMode_Rotate || eControlMode == ShootEditor::ControlMode_Scale))
		{
		}
		else
		{
			m_LastMousePosition = mousePos;
		}

		// Handle dragging of selected entities
		if(m_bDraggingStarted)
		{
			if(pSelectedEntity)
			{
				if(pSelectedEntity->IsA(Entity3D::TypeID))
				{
					Vector3 rayStart, rayDir;
					pTarget3DCamera->Get3DRayFromScreenCoords(rayStart, rayDir, Point(s32(mousePos.X), s32(mousePos.Y)), Vector2(GetSize().x, GetSize().y));
					Entity3DController::Instance()->OnMouseDragged(rayStart, rayDir);
				}
				else if(Entity2D* pEntity2D = DYNAMIC_CAST(pSelectedEntity, Entity2D))
				{
					switch(eControlMode)
					{
					case ShootEditor::ControlMode_Translate:
						{
							// Take camera zoom into account
							Vector3 vCameraScale = pTarget2DCamera->GetViewMatrix().GetScale();
							Vector2 localDeltaPos(deltaPos.X/vCameraScale.X, deltaPos.Y/vCameraScale.Y);					

							// Take into account transformation matrices, reverse transform the new screen position
							// using the product of the local translation matrix and the parent transformation matrix
							Matrix44 parentMatrix = Matrix44::Identity;
							if(pEntity2D->GetParent()
							&& pEntity2D->GetParent()->IsA(Entity2D::TypeID))
							{
								parentMatrix = static_cast<Entity2D*>(pEntity2D->GetParent())->GetTransformationMatrix();
							}

							// Get local translation matrix: Local rotation and scaling should not be used
							// because they don't affect the entity local position, but if they are included in the 
							// matrix then the resulting inverse matrix will give a wrong screen position
							Matrix44 localTranslationMatrix = Matrix44::Identity;
							Vector2 localPos2D = pEntity2D->GetPosition() - pEntity2D->GetCenter();
							localTranslationMatrix.Translate(Vector3::Create(localPos2D.X, localPos2D.Y, 0.0f));
							Matrix44 matrix = localTranslationMatrix * parentMatrix;

							Vector3 absolutePos = matrix.GetTranslation();				
							Matrix44 inverse;
							if(matrix.GetInverse(inverse))
							{
								Vector3 invPos3D = inverse.TransformVect(absolutePos + Vector3::Create(localDeltaPos.X, localDeltaPos.Y, 0.0f));
								Vector2 invPos(invPos3D.X, invPos3D.Y);
								pEntity2D->Translate(invPos);
							}
						}
						break;

					case ShootEditor::ControlMode_Rotate:
						{
							Vector3 center(pEntity2D->GetCenterTransformationMatrix().GetTranslation());
							Vector3 _3DLastMousePos = Vector3::Create(m_LastMousePosition.X, m_LastMousePosition.Y, 0.0f);
							Vector3 _3DMousePos = Vector3::Create(mousePos.X, mousePos.Y, 0.0f);
							Vector3 rotationStart = (_3DLastMousePos-center).Normalize();
							Vector3 currentRotation = (_3DMousePos-center).Normalize();
							f32 angle = Math::ACos(rotationStart.DotProduct(currentRotation));
							f32 direction = -rotationStart.CrossProduct(currentRotation).Z;
							pEntity2D->SetRotation(m_fEntity2DStartRotation + angle*Math::RadToDegFactor*direction);
						}
						break;

					case ShootEditor::ControlMode_Scale:
						{
							f32 fScale = Math::FAbs(deltaPos.X) > Math::FAbs(deltaPos.Y) ? deltaPos.X : deltaPos.Y;
							Vector2 vScale(fScale, fScale);
							Vector2 vSize(pEntity2D->GetBoundingBox().Size());
							Vector2 deltaScale(vScale.X/vSize.X, vScale.Y/vSize.Y);
							pEntity2D->SetScale(m_vEntity2DStartScale+deltaScale);
						}
						break;
					}
				}
			}
		}
		else if(m_bStrafingStarted)
		{				
			b3DCameraControl ? CameraManager::Instance()->StrafeCamera(pTarget3DCamera, deltaPos)
				: CameraManager::Instance()->StrafeCamera(pTarget2DCamera, deltaPos);
		}
		else if(m_bZoomingStarted)
		{
			b3DCameraControl ? CameraManager::Instance()->ZoomCamera(pTarget3DCamera, deltaPos.Y)
				: CameraManager::Instance()->ZoomCamera(pTarget2DCamera, deltaPos.Y);
		}
		else if(m_bLookingStarted)
		{
			CameraManager::Instance()->RotateCamera(pTarget3DCamera, deltaPos);
		}
		else if(ShootEditor::Instance()->GetSelectedEntity())
		{
			// update 3D picking ray
			Vector3 rayStart, rayDir;					
			pTarget3DCamera->Get3DRayFromScreenCoords(rayStart, rayDir, Point(s32(mousePos.X), s32(mousePos.Y)), Vector2(GetSize().x, GetSize().y));
			if(Entity3DController::Instance()->HasEntity())
			{
				Entity3DController::Instance()->OnPickingRayMoved(rayStart, rayStart + rayDir * 999.0f);
			}
		}
	}

	void ViewPort::OnMouseLeftDown(wxMouseEvent& event)
	{
		if(Engine::Instance()->IsLoading())
		{
			return;
		}

		SetFocus();
			
		Vector2 clickPos(event.GetX(), event.GetY());
		Camera* pCamera = EntityRenderer::Instance()->Get3DCamera();

		if(event.AltDown())
		{
			if(pCamera != m_Default3DCamera.Get() && pCamera != m_Default2DCamera.Get())
			{
				UndoManager::Instance()->RecordTransformState(pCamera);
			}
			m_bLookingStarted = true;
		}
		else
		{			
			Entity* pSelectedEntity = NULL;

			ShootEditor::E_ControlMode eControlMode = ShootEditor::Instance()->GetControlMode();
			bool bRotatingOrScaling = (eControlMode == ShootEditor::ControlMode_Rotate) || (eControlMode == ShootEditor::ControlMode_Scale);
			bool bControlling2DEntity = ShootEditor::Instance()->GetSelectedEntity() && ShootEditor::Instance()->GetSelectedEntity()->IsA(Entity2D::TypeID);
			if(bControlling2DEntity && bRotatingOrScaling)
			{
				pSelectedEntity = ShootEditor::Instance()->GetSelectedEntity();
			}
			else if(Entity3DController::Instance()->IsAxeHighlighted())
			{
				pSelectedEntity = ShootEditor::Instance()->GetSelectedEntity();
			}
			else if(!ShootEditor::Instance()->Get3DControl())
			{
				// check 2D entities first
				pSelectedEntity = GetSelectedEntity(Engine::Instance()->GetContextStack(), clickPos);
			}

			Vector3 rayStart = Vector3::Zero, rayDir = Vector3::One;
			if(!pSelectedEntity)
			{
				// check 3D entities
				pCamera->Get3DRayFromScreenCoords(rayStart, rayDir, Point(s32(clickPos.X), s32(clickPos.Y)), Vector2(GetSize().x, GetSize().y));

				Entity3D* p3DEntity = NULL;
				m_fDistToClosestPickedEntity = Math::Maxf32;
				EditorRenderer::Instance()->SetPickingInfoValid(false);
				GetSelectedEntity(Engine::Instance()->GetContextStack(), rayStart, rayDir, p3DEntity);
				if(p3DEntity && p3DEntity->IsA(SubMesh::TypeID))
				{
					p3DEntity = p3DEntity->GetAncestor<MeshEntity>();
				}
				pSelectedEntity = p3DEntity;
			}
			
			pSelectedEntity = ShootEditor::Instance()->SelectEntity(pSelectedEntity, rayStart, rayDir);

			// start dragging if it's a 2D entity
			if(Entity2D* pEntity2D = DYNAMIC_CAST(pSelectedEntity, Entity2D))
			{
				UndoManager::Instance()->RecordTransformState(pEntity2D);
				m_bDraggingStarted = true;				
				switch(eControlMode)
				{
				case ShootEditor::ControlMode_Rotate: m_fEntity2DStartRotation = pEntity2D->GetRotation(); break;
				case ShootEditor::ControlMode_Scale: m_vEntity2DStartScale = pEntity2D->GetScale(); break;
				}
			}
			else
			{
				// start dragging if 3D axe was highlighted		
				m_bDraggingStarted = Entity3DController::Instance()->IsAxeHighlighted();
				if(m_bDraggingStarted)
				{
					Entity3D* pEntity3D = DYNAMIC_CAST(pSelectedEntity, Entity3D);
					UndoManager::Instance()->RecordTransformState(pEntity3D);
				}
			}
		}

		m_LastMousePosition = clickPos;
	}

	void ViewPort::OnMouseLeftUp(wxMouseEvent& event)
	{
		if(m_bDraggingStarted)
		{
			if(Entity2D* pEntity2D = DYNAMIC_CAST(ShootEditor::Instance()->GetSelectedEntity(), Entity2D))
			{
				ShootEditor::Instance()->GetObjectInspector()->UpdateProperty("Position", WXVARIANT(pEntity2D->GetPosition()));
				ShootEditor::Instance()->GetObjectInspector()->UpdateProperty("Rotation", WXVARIANT(pEntity2D->GetRotation()));
				ShootEditor::Instance()->GetObjectInspector()->UpdateProperty("Scale", WXVARIANT(pEntity2D->GetScale()));
			}
		}
		m_bDraggingStarted = false;

		if(m_bLookingStarted)
		{
			CameraManager::Instance()->CheckPropertyOverrides(EntityRenderer::Instance()->Get3DCamera());
			CameraManager::Instance()->CheckPropertyOverrides(EntityRenderer::Instance()->Get2DCamera());
			m_bLookingStarted = false;
		}		
	}

	void ViewPort::OnMouseMiddleDown(wxMouseEvent& event)
	{
		if(ShootEditor::Instance()->IsEngineRunning())
		{
			InputManager::TouchState state;
			state.bPressed = true;
			state.vPosition.Set(event.GetX(), event.GetY());
			InputManager::Instance()->SetTouchStateAsync(state);
		}
	}

	void ViewPort::OnMouseMiddleUp(wxMouseEvent& event)
	{
		if(ShootEditor::Instance()->IsEngineRunning())
		{
			InputManager::TouchState state;
			state.bPressed = false;
			state.vPosition.Set(event.GetX(), event.GetY());
			InputManager::Instance()->SetTouchStateAsync(state);
		}
	}

	void ViewPort::OnMouseRightDown(wxMouseEvent& event)
	{
		if(Engine::Instance()->IsLoading())
		{
			return;
		}

		if(event.AltDown())
		{
			m_bZoomingStarted = true;
		}
		else
		{
			m_bStrafingStarted = true;				
		}

		Camera* pCamera = EntityRenderer::Instance()->Get3DCamera();

		if(pCamera != m_Default3DCamera.Get() && pCamera != m_Default2DCamera.Get())
		{
			UndoManager::Instance()->RecordTransformState(pCamera);
		}

		CameraManager::Instance()->UpdateControlSteps();
		Vector2 clickPos(event.GetX(), event.GetY());
		m_LastMousePosition = clickPos;
	}

	void ViewPort::OnMouseRightUp(wxMouseEvent& event)
	{
		m_bZoomingStarted = false;
		m_bStrafingStarted = false;	

		CameraManager::Instance()->CheckPropertyOverrides(EntityRenderer::Instance()->Get3DCamera());
		CameraManager::Instance()->CheckPropertyOverrides(EntityRenderer::Instance()->Get2DCamera());
	}

	void ViewPort::OnMouseWheelRolled(wxMouseEvent& event)
	{			
	}

	void ViewPort::OnKeyDown(wxKeyEvent& event)
	{	
		if(ShootEditor::Instance()->IsEngineRunning())
		{
			InputManager::E_KeyType shootKey = EditorUtils::GetShootKeyType(event.GetKeyCode());
			InputManager::Instance()->SetKeyPressedAsync(shootKey, true);

			// quick pause
			if(event.GetKeyCode() == WXK_SPACE)
			{
				ShootEditor::Instance()->PauseEngine();
			}
		}
		else
		{
			// quick un-pause
			if(event.GetKeyCode() == WXK_SPACE)
			{
				ShootEditor::Instance()->StartEngine();
			}
		}
	}

	void ViewPort::OnKeyUp(wxKeyEvent& event)
	{	
		if(ShootEditor::Instance()->IsEngineRunning())
		{
			InputManager::E_KeyType shootKey = EditorUtils::GetShootKeyType(event.GetKeyCode());
			InputManager::Instance()->SetKeyPressedAsync(shootKey, false);
		}			
	}

	void ViewPort::OnMouseLeave(wxMouseEvent& event)
	{	
		if(m_bDraggingStarted)
		{			
			if(Entity2D* pEntity2D = DYNAMIC_CAST(ShootEditor::Instance()->GetSelectedEntity(), Entity2D))
			{
				ShootEditor::Instance()->GetObjectInspector()->UpdateProperty("Position", WXVARIANT(pEntity2D->GetPosition()));
			}
		}
		m_bDraggingStarted = false;
		m_bZoomingStarted = false;
		m_bStrafingStarted = false;
		m_bLookingStarted = false;		
	}	

	void ViewPort::OnResized(wxSizeEvent& event)
	{
		event.Skip();
		
		if (!m_pGLContext)
			return;

		m_pGLContext->SetCurrent(*this);
		Size newSize(event.GetSize().x, event.GetSize().y);
		Engine::Instance()->ResizeScreen(newSize);
	}

	void ViewPort::OnFocusGained(wxFocusEvent& event)
	{
		Refresh(true); // make sure background is erased and refreshed		
	}

	void ViewPort::OnFocusLost(wxFocusEvent& event)
	{
	}

	void ViewPort::OnPaint(wxPaintEvent& event)
	{
		wxPaintDC dc(this);

		if (m_pGLContext)
			m_pGLContext->SetCurrent(*this);
			
		GraphicsDriver::Instance()->SetViewPort(AABBox2D(Vector2(0.0f, 0.0f), Vector2(f32(GetSize().x), f32(GetSize().y))));

		if(!Engine::Instance()->IsLoading())
		{
			// force default cameras in case game camera is null
			if(!EntityRenderer::Instance()->Get3DCamera())
			{
				EntityRenderer::Instance()->Set3DCamera(m_Default3DCamera);
			}

			if(!EntityRenderer::Instance()->Get2DCamera())
			{
				EntityRenderer::Instance()->Set2DCamera(m_Default2DCamera);
			}
		}

		Engine::Instance()->Render();
		EditorRenderer::Instance()->Render();
		GraphicsDriver::Instance()->Present();

#ifndef DX11
		{
			glFlush();
			SwapBuffers();
		}
#endif // DX11

		f32 fDeltaTime = f32(m_StopWatch.Time())/1000.0f;
		fDeltaTime = Math::Min(fDeltaTime, 0.1f);
		ShootEditor::Instance()->SetDeltaTime(fDeltaTime);
		m_StopWatch.Start();
	}

	void ViewPort::OnEraseBackground(wxEraseEvent& WXUNUSED(event))
	{
		// Do nothing, to avoid flashing.
	}
}

