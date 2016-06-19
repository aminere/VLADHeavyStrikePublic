/* 

Amine Rehioui
Created: March 30th 2013

*/

#include "Shoot.h"

#include "QuadTree.h"

#include "MeshEntity.h"

#include "CollisionUtils.h"

#include "Context.h"

namespace shoot
{
	DEFINE_OBJECT(QuadTree);

	//! constructor
	QuadTree::QuadTree()
		: m_NodeCount(0)
		, m_NodesDrawn(0)
		// properties
		, m_vNodeSize(50.0f, 50.0f)
	{
		m_Root = snew QuadTreeNode();
	}

	//! destructor
	QuadTree::~QuadTree()
	{
	}

	//! serializes the object to/from a PropertyStream
	void QuadTree::Serialize(PropertyStream& stream)
	{
		super::Serialize(stream);

		if(stream.Serialize(PT_Vec2, "NodeSize", &m_vNodeSize))
		{
			if(m_Context.IsValid())
			{
				Build(m_Context.Get());
			}
		}
	}

	//! builds the quad tree
	void QuadTree::Build(Context* pContext)
	{
		m_Context = pContext;
		m_Root = snew QuadTreeNode();

		for(u32 i=0; i<pContext->GetChildCount(); ++i)
		{
			ExtractEntities(pContext->GetChild(i));
		}
		
		if(!m_Root->GetEntityList().empty())
		{
			std::list< Handle<Entity3D> >::const_iterator it = m_Root->GetEntityList().begin();
			AABBox3D rootBBox = (*it)->GetBoundingBox();
			++it;
			for(; it != m_Root->GetEntityList().end(); ++it)
			{
				rootBBox = rootBBox + (*it)->GetBoundingBox();
			}

			m_Root->SetBBox(rootBBox);
			m_Root->Build(m_vNodeSize);
		}
	}

	//! updates the QuadTree
	void QuadTree::Update()
	{
		m_ProcessedEntities.clear();
		m_Root->Update(this);

		// update the always active
		for(std::list< Handle<Entity> >::iterator it = m_lAlwaysActive.begin(); it != m_lAlwaysActive.end(); )
		{
			if((*it).IsValid())
			{
				m_Context->Update((*it));
				++it;
			}
			else
			{
				it = m_lAlwaysActive.erase(it);
			}
		}
	}

	//! registers the QuadTree for rendering
	void QuadTree::RegisterForRendering()
	{
		m_NodeCount = 0;
		m_NodesDrawn = 0;
		m_lActiveNodes.clear();

		Camera* pCamera = m_Camera.IsValid() ? m_Camera.Get() : EntityRenderer::Instance()->Get3DCamera();
		const Frustum& frustum = pCamera->GetFrustum();
		m_ProcessedEntities.clear();
		m_Root->RegisterForRendering(frustum, this);

		// register the always active
		for(std::list< Handle<Entity> >::iterator it = m_lAlwaysActive.begin(); it != m_lAlwaysActive.end(); )
		{
			if((*it).IsValid())
			{
				m_Context->RegisterForRendering((*it));
				++it;
			}
			else
			{
				it = m_lAlwaysActive.erase(it);
			}
		}
	}

	//! called when an entity is dynamically added
	void QuadTree::OnEntityAdded(Entity* pEntity)
	{
		m_lAlwaysActive.push_back(Handle<Entity>(pEntity));
	}

	//! recursively extracts 3D entities from the context
	void QuadTree::ExtractEntities(Entity* pEntity)
	{
		if(Entity3D* pEntity3D = DYNAMIC_CAST(pEntity, Entity3D))
		{
			if(pEntity3D->IsAlwaysActive())
			{
				m_lAlwaysActive.push_back(Handle<Entity>(pEntity3D));
			}
			else
			{
				m_Root->AddEntity(pEntity3D);
				for(u32 i=0; i<pEntity3D->GetChildCount(); ++i)
				{
					ExtractEntities(pEntity3D->GetChild(i));
				}
			}
		}
		else
		{
			m_lAlwaysActive.push_back(Handle<Entity>(pEntity));
		}
	}

	//! increments the nodes drawn
	void QuadTree::IncrNodesDrawn(QuadTreeNode* pNode)
	{ 
		m_lActiveNodes.push_back(pNode);
		++m_NodesDrawn;
	}

	//! rebuilds the QuadTree
	void QuadTree::Rebuild() const
	{
		m_Root->Build(m_vNodeSize);
	}

	//! constructor
	QuadTreeNode::QuadTreeNode()
		: m_bLeaf(true)
		, m_FrustumTestResult(Frustum::TR_Out)
	{
	}

	//! destructor
	QuadTreeNode::~QuadTreeNode()
	{
	}

	//! builds the quad tree node
	void QuadTreeNode::Build(const Vector2& vNodeSize)
	{
		Vector3 vSize = m_BBox.Size();
		//if(vSize.X > vNodeSize.X || vSize.Y > vNodeSize.Y)
		//{
			//Vector2 vHalfSize(m_BBox.Size()/2.0f);
			//Vector2 XnYnMin(m_BBox.Min());
			//Vector2 XnYnMax(XnYnMin+vHalfSize);
			//Vector2 XpYnMin(m_BBox.Min()+Vector2(vHalfSize.X, 0.0f));
			//Vector2 XpYnMax(XpYnMin+vHalfSize);
			//Vector2 XnYpMin(m_BBox.Min()+Vector2(0.0f, vHalfSize.Y));
			//Vector2 XnYpMax(XnYpMin+vHalfSize);
			//Vector2 XpYpMin(XnYnMax);
			//Vector2 XpYpMax(XpYpMin+vHalfSize);

			//AABBox2D bboxes[T_Count] = 
			//{
			//	AABBox2D(XnYnMin, XnYnMax),
			//	AABBox2D(XpYnMin, XpYnMax),
			//	AABBox2D(XnYpMin, XnYpMax),
			//	AABBox2D(XpYpMin, XpYpMax)
			//};

		// optimization for vertical scrolling space shooter
		if(vSize.Y > vNodeSize.Y)
		{
			Vector3 vHalfSize = Vector3::Create(m_BBox.Size().X, m_BBox.Size().Y/2.0f, m_BBox.Size().Z);
			Vector3 YnMin = Vector3::Create(m_BBox.Min().X, m_BBox.Min().Y, m_BBox.Min().Z);
			Vector3 YnMax(YnMin+vHalfSize);
			Vector3 YpMin(YnMin+Vector3::Create(0.0f, vHalfSize.Y, 0.0f));
			Vector3 YpMax(YpMin+vHalfSize);

			AABBox3D bboxes[T_Count] = 
			{
				AABBox3D(YnMin, YnMax),
				AABBox3D(YpMin, YpMax)
			};

			for(std::list< Handle<Entity3D> >::iterator it = m_EntityList.begin(); it != m_EntityList.end(); ++it)
			{
				AABBox3D bbox = (*it)->GetBoundingBox();
				AABBox2D bbox2D(Vector2(bbox.Min().X, bbox.Min().Y), Vector2(bbox.Max().X, bbox.Max().Y));
				bool bNodeFound = false;

				for(u32 i=0; i<T_Count; ++i)
				{
					AABBox2D bboxes2D(Vector2(bboxes[i].Min().X, bboxes[i].Min().Y), Vector2(bboxes[i].Max().X, bboxes[i].Max().Y));
					if(CollisionUtils::AABBox2DIntersectsWithAABBox2D(bboxes2D, bbox2D))
					{
						if(!m_Nodes[i].IsValid())
						{
							m_Nodes[i] = snew QuadTreeNode();
							m_Nodes[i]->SetBBox(bboxes[i]);
							m_bLeaf = false;
						}
						m_Nodes[i]->AddEntity((*it).Get());
						bNodeFound = true;
					}
				}

				if(!bNodeFound)
				{
#ifndef SHOOT_EDITOR
					SHOOT_ASSERT(m_bLeaf, "QuadTree Build Error");
#endif // SHOOT_EDITOR
				}
			}

			if(!m_bLeaf)
			{
				m_EntityList.clear();
				m_CollisionComponents.clear();

				for(u32 i=0; i<T_Count; ++i)
				{
					if(m_Nodes[i].IsValid())
					{
						m_Nodes[i]->Build(vNodeSize);
					}
				}
			}
		}
	}

	//! adds an entity to the node
	void QuadTreeNode::AddEntity(Entity3D* pEntity3D)
	{
		m_EntityList.push_back(Handle<Entity3D>(pEntity3D));
		
		if(EnvironmentCollisionComponent* pEnvCol = pEntity3D->GetComponent<EnvironmentCollisionComponent>())
		{
			m_CollisionComponents.push_back(Handle<EnvironmentCollisionComponent>(pEnvCol));
		}
	}

	//! Updates the node and its children
	void QuadTreeNode::Update(QuadTree* pQuadTree)
	{
		if(m_FrustumTestResult != Frustum::TR_Out)
		{
			if(m_bLeaf)
			{
				for(std::list< Handle<Entity3D> >::iterator it = m_EntityList.begin(); it != m_EntityList.end(); )
				{
					if((*it).IsValid())
					{
						if((*it)->IsEnabled() && !pQuadTree->GetEntityProcessed((*it).Get()))
						{
							(*it)->Update();
							for(u32 i=0; i<(*it)->GetComponentCount(); ++i)
							{
								(*it)->GetComponent(i)->Update();
							}
							pQuadTree->SetEntityProcessed((*it).Get());
						}
						++it;
					}
					else
					{
						it = m_EntityList.erase(it);
					}
				}
			}
			else
			{
				for(u32 i=0; i<T_Count; ++i)
				{
					if(m_Nodes[i].IsValid())
					{
						m_Nodes[i]->Update(pQuadTree);
					}
				}
			}
		}
	}

	// Registers the node and its children for rendering
	void QuadTreeNode::RegisterForRendering(const Frustum& frustum, QuadTree* pQuadTree, bool bSkipFrustumTest /*= false*/)
	{
		pQuadTree->IncrNodeCount();
		
		if(!bSkipFrustumTest)
		{
			// sphere test first, then bbox test
			Vector3 vNodeCenter = m_BBox.Min() + m_BBox.Size()/2.0f;
			f32 fNodeRadius = Math::Max(m_BBox.Size().X, m_BBox.Size().Y, m_BBox.Size().Z)/2.0f;
			m_FrustumTestResult = frustum.Test(vNodeCenter, fNodeRadius);

			if(m_FrustumTestResult == Frustum::TR_Intersect)
			{
				m_FrustumTestResult = frustum.Test(m_BBox);
			}
		}
		else
		{
			m_FrustumTestResult = Frustum::TR_In;
		}

		if(m_FrustumTestResult != Frustum::TR_Out)
		{
			if(m_bLeaf)
			{
				pQuadTree->IncrNodesDrawn(this);
				for(std::list< Handle<Entity3D> >::iterator it = m_EntityList.begin(); it != m_EntityList.end(); )
				{
					if((*it).IsValid())
					{
						if((*it)->IsVisible() && !pQuadTree->GetEntityProcessed((*it).Get()))
						{
							(*it)->RegisterForRendering();
							pQuadTree->SetEntityProcessed((*it).Get());
						}
						++it;
					}
					else
					{
						it = m_EntityList.erase(it);
					}
				}
			}
			else
			{
				for(u32 i=0; i<T_Count; ++i)
				{
					if(m_Nodes[i].IsValid())
					{
						m_Nodes[i]->RegisterForRendering(frustum, pQuadTree, m_FrustumTestResult == Frustum::TR_In);
					}
				}
			}
		}
	}
}

