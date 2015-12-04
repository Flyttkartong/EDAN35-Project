#pragma once

#include <cstdio>
#include <string>
#include <map>
#include <unordered_map>
#include <list>
#include "Types.h"
#include "VectorMath.h"

struct StructDescEntry {
	StructDescEntry() {}
	StructDescEntry(std::string name_, unsigned int components_, unsigned int offset_, types::DataType type) :
		mName(name_), mComponents(components_), mOffset(offset_), mType(type), mSize((unsigned int) TypeSize(type)) {}

	std::string		mName;
	unsigned int		mOffset;
	unsigned int		mComponents;
	unsigned int		mSize;
        types::DataType		mType;
};

template<typename T> inline StructDescEntry DescEntry(std::string name_, unsigned int arrayLength_ = 1, unsigned int offset_ = 0) {
	return StructDescEntry(name_, arrayLength_ * Elements<T>(), offset_, TypeOf<T>());
}

class StructDesc {
public:
	StructDesc() : mStride(0) {}
	StructDesc(StructDescEntry *tuples, unsigned int nTuples);

public:
	void SetEntry(std::string name, unsigned int offset, unsigned int components, types::DataType type);
	void SetEntry(StructDescEntry &entry);

	// If nTuples == 0, the array is assumed to be null-terminated
	void SetEntries(StructDescEntry *tuples, unsigned int nTuples); 
	size_t GetEntryCount() const {return mEntryMap.size();}
	StructDescEntry *GetEntry(const std::string &name);
	
public:
	unsigned int mStride;
	std::unordered_map<std::string, StructDescEntry> mEntryMap;
	std::list<StructDescEntry> mEntryList;
};

/*
 * This assembles data from multiple data sources in to target.
 * targetDesc determines which elements should be fetched from data.
 * target must be allocated by the caller (must be at least nStructs * targetDesc.mStride bytes).
 */
void AssembleData(u8 *target, int nStructs, const StructDesc &targetDesc, StructDesc **desc, u8 **data, int nStreams);
void AssembleData(u8 *target, int nStructs, const StructDesc &targetDesc, StructDesc &desc, u8 *data);

void WriteData(std::string filename, const StructDesc &desc, u8 *data, int n);
void WriteData(std::ostream &os, const StructDesc &desc, u8 *data, int n);
void ReadData(std::string filename, StructDesc &desc, u8 **data, int &n);
void ReadData(std::istream &is, StructDesc &desc, u8 **data, int &n);
