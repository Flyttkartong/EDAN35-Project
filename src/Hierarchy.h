#pragma once

template<class DataType, class InheritType, class CombinedType>
class HierarchyNode {
private:
	typedef HierarchyNode<DataType, InheritType, CombinedType> HNode;

public:
	HierarchyNode();
	HierarchyNode(InheritType node, DataType data);
	virtual ~HierarchyNode();

public:
	InheritType GetNode() const;
	CombinedType GetInherited();

	InheritType *Map();
	void Unmap();

	DataType *GetData();
	size_t GetDegree() const;

	// TODO
	HierarchyNode<DataType, InheritType, CombinedType> *ConnectParent(HierarchyNode<DataType, InheritType, CombinedType> *newParent);
	HierarchyNode<DataType, InheritType, CombinedType> *DisconnectParent();
	void ConnectChild(HierarchyNode<DataType, InheritType, CombinedType> *child);
	void DisconnectChild(HierarchyNode<DataType, InheritType, CombinedType> *child);

	static void ConnectParentChild(HierarchyNode<DataType, InheritType, CombinedType> *parent, HierarchyNode<DataType, InheritType, CombinedType> *child);
	static void DisconnectParentChild(HierarchyNode<DataType, InheritType, CombinedType> *parent, HierarchyNode<DataType, InheritType, CombinedType> *child);

	HierarchyNode<DataType, InheritType, CombinedType> *GetParent() const;
	std::vector<HierarchyNode<DataType, InheritType, CombinedType> *> *GetChildren();

	template<class Op> void TraverseBredthFirst(Op &o);
	template<class Op> void TraverseDepthFirst(Op &o);

protected:
	template<class Op> void TraverseBredthFirstRec(Op &o, int level);
	template<class Op> void TraverseDepthFirstRec(Op &o, int level);
	void MakeCurrent();

	virtual CombinedType Combine(HierarchyNode<DataType, InheritType, CombinedType> *parent, HierarchyNode<DataType, InheritType, CombinedType> *child) = 0;

	/*
	TODO
	GetSiblings()
	GetLevel()
	*/

protected:
	InheritType mNode; // TODO: Change name
	CombinedType mCached;
	u64 mVersion, mCachedVersion;
	u64 mLastSeenParentVersion;
	bool mMapped;
	DataType mData;

	HierarchyNode<DataType, InheritType, CombinedType> *mParent;
	std::vector<HierarchyNode<DataType, InheritType, CombinedType> *> mChildren;
};

#include "Hierarchy.inl"
