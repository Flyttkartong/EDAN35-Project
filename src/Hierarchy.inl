#include <algorithm>
#include "Log.h"

template<class DataType, class InheritType, class CombinedType>
HierarchyNode<DataType, InheritType, CombinedType>::HierarchyNode() :
	mVersion(0), mLastSeenParentVersion(-1), mMapped(false), mParent(nullptr)
{
}

template<class DataType, class InheritType, class CombinedType>
HierarchyNode<DataType, InheritType, CombinedType>::HierarchyNode(InheritType node, DataType local) :
	mNode(node), mVersion(0), mLastSeenParentVersion(-1), mMapped(false), mLocal(local), mParent(nullptr)
{
}

template<class DataType, class InheritType, class CombinedType>
HierarchyNode<DataType, InheritType, CombinedType>::~HierarchyNode()
{

}

template<class DataType, class InheritType, class CombinedType>
InheritType HierarchyNode<DataType, InheritType, CombinedType>::GetNode() const
{
	return mNode;
}

template<class DataType, class InheritType, class CombinedType>
CombinedType HierarchyNode<DataType, InheritType, CombinedType>::GetInherited()
{
	MakeCurrent();
	return mCached;
}

template<class DataType, class InheritType, class CombinedType>
InheritType *HierarchyNode<DataType, InheritType, CombinedType>::Map()
{
	if (mMapped)
		LogError("Node is already mapped");
	MakeCurrent();
	mMapped = true;

	return &mNode;
}

template<class DataType, class InheritType, class CombinedType>
void HierarchyNode<DataType, InheritType, CombinedType>::Unmap()
{
	if (!mMapped)
		LogError("Node is not mapped");
	mMapped = false;
	mVersion++;
}

template<class DataType, class InheritType, class CombinedType>
DataType *HierarchyNode<DataType, InheritType, CombinedType>::GetData()
{
	return &mData;
}

template<class DataType, class InheritType, class CombinedType>
size_t HierarchyNode<DataType, InheritType, CombinedType>::GetDegree() const
{
	return mChildren.size();
}

template<class DataType, class InheritType, class CombinedType>
HierarchyNode<DataType, InheritType, CombinedType> *HierarchyNode<DataType, InheritType, CombinedType>::ConnectParent(HierarchyNode<DataType, InheritType, CombinedType> *newParent)
{
	HierarchyNode<DataType, InheritType, CombinedType> *oldParent = mParent;
	ConnectParentChild(newParent, this);
	return oldParent;
}

template<class DataType, class InheritType, class CombinedType>
HierarchyNode<DataType, InheritType, CombinedType> *HierarchyNode<DataType, InheritType, CombinedType>::DisconnectParent()
{
	HierarchyNode<DataType, InheritType, CombinedType> *oldParent = mParent;
	DisconnectParentChild(mParent, this);
	mParent = nullptr;
	return oldParent;
}

template<class DataType, class InheritType, class CombinedType>
void HierarchyNode<DataType, InheritType, CombinedType>::ConnectChild(HierarchyNode<DataType, InheritType, CombinedType> *child)
{
	ConnectParentChild(this, child);
}

template<class DataType, class InheritType, class CombinedType>
void HierarchyNode<DataType, InheritType, CombinedType>::DisconnectChild(HierarchyNode<DataType, InheritType, CombinedType> *child)
{
	DisconnectParentChild(this, child);
}

template<class DataType, class InheritType, class CombinedType>
void HierarchyNode<DataType, InheritType, CombinedType>::ConnectParentChild(HierarchyNode<DataType, InheritType, CombinedType> *parent, HierarchyNode<DataType, InheritType, CombinedType> *child)
{
	parent->GetChildren()->push_back(child);
	child->mParent = parent;
	child->mLastSeenParentVersion = parent->mVersion - 1;
	child->mCachedVersion = child->mVersion - 1;
}

template<class DataType, class InheritType, class CombinedType>
void HierarchyNode<DataType, InheritType, CombinedType>::DisconnectParentChild(HierarchyNode<DataType, InheritType, CombinedType> *parent, HierarchyNode<DataType, InheritType, CombinedType> *child)
{
	std::vector<HierarchyNode<DataType, InheritType> *> *children = parent->GetChildren();
	auto elem = std::find(children->begin(), children->end(), item);
	if (elem == children->end())
		return;
	children->erase(elem);
	child->mParent = nullptr;
	child->mCachedVersion = child->mVersion - 1;
}


template<class DataType, class InheritType, class CombinedType>
HierarchyNode<DataType, InheritType, CombinedType> *HierarchyNode<DataType, InheritType, CombinedType>::GetParent() const
{
	return mParent;
}

template<class DataType, class InheritType, class CombinedType>
std::vector<HierarchyNode<DataType, InheritType, CombinedType> *> *HierarchyNode<DataType, InheritType, CombinedType>::GetChildren()
{
	return &mChildren;
}

template<class DataType, class InheritType, class CombinedType>
void HierarchyNode<DataType, InheritType, CombinedType>::MakeCurrent()
{
	if (mParent != nullptr) {
		mParent->MakeCurrent();
		if (mLastSeenParentVersion == mParent->mVersion && mCachedVersion == mVersion)
			return;
		mLastSeenParentVersion = mParent->mVersion;
	} else if (mCachedVersion == mVersion)
		return;
	mVersion++;
	mCachedVersion = mVersion;
	mCached = Combine(mParent, this);
}

template<class DataType, class InheritType, class CombinedType>
template<class Op> void HierarchyNode<DataType, InheritType, CombinedType>::TraverseBredthFirst(Op &o)
{
	o(*this, 0);
	this->TraverseBredthFirstRec(o, 1);
}

template<class DataType, class InheritType, class CombinedType>
template<class Op> void HierarchyNode<DataType, InheritType, CombinedType>::TraverseBredthFirstRec(Op &o, int level)
{
	for each (auto it in GetChildren())
		o(*it, level);
	for each (auto it in GetChildren())
		it->TraverseBredthFirstRec(o, level + 1);
}

template<class DataType, class InheritType, class CombinedType>
template<class Op> void HierarchyNode<DataType, InheritType, CombinedType>::TraverseDepthFirst(Op &o)
{
	TraverseDepthFirstRec(o, 0);
}

template<class DataType, class InheritType, class CombinedType>
template<class Op> void HierarchyNode<DataType, InheritType, CombinedType>::TraverseDepthFirstRec(Op &o, int level)
{
	o(*this, level);
	for each (auto it in *GetChildren())
		it->TraverseDepthFirstRec(o, level + 1);
}

