#include <unordered_map>
#include <vector>
#include <fstream>
#include <string>
#include <iostream>
#include "StructDesc.h"
#include "Log.h"

/*----------------------------------------------------------------------------*/

StructDesc::StructDesc(StructDescEntry *tuples, unsigned int nTuples)
{
	SetEntries(tuples, nTuples);
}

/*----------------------------------------------------------------------------*/

void StructDesc::SetEntry(std::string name, unsigned int offset, unsigned int components, types::DataType type)
{
	auto res = mEntryMap.find(name);
	if (res != mEntryMap.end()) {
		LogMsgOnce(Log::TYPE_INFO, "Entry %s already exists", name.c_str());
		for (auto it = mEntryList.begin(); it != mEntryList.end(); ++it) {
			if (it->mName == name) {
				mEntryList.erase(it);
				break;
			}
		}
	}

	StructDescEntry entry = StructDescEntry(name, components, offset, type);
	mEntryMap[name] = entry;
	mEntryList.push_back(entry);
}

/*----------------------------------------------------------------------------*/

void StructDesc::SetEntry(StructDescEntry &entry)
{
	SetEntry(entry.mName, entry.mOffset, entry.mComponents, entry.mType);
}

/*----------------------------------------------------------------------------*/

void StructDesc::SetEntries(StructDescEntry *tuples, unsigned int nTuples)
{
	unsigned int p = 0;
	for (unsigned int i = 0; i < nTuples; i++) {
		SetEntry(tuples[i].mName, p, tuples[i].mComponents, tuples[i].mType);
		p += tuples[i].mSize * tuples[i].mComponents;
	}
	mStride = p;
}

/*----------------------------------------------------------------------------*/

StructDescEntry *StructDesc::GetEntry(const std::string &name)
{
	auto it = mEntryMap.find(name);
	if (it == mEntryMap.end())
		return nullptr;
	return &it->second;
}

/*----------------------------------------------------------------------------*/

void AssembleData(u8 *target, int nStructs, const StructDesc &targetDesc, StructDesc **desc, u8 **data, int nStreams)
{
	for (auto it = targetDesc.mEntryList.begin(); it != targetDesc.mEntryList.end(); ++it) {
		int i = 0;
		for (; i < nStreams; i++) {
			auto elem = desc[i]->mEntryMap.find(it->mName);
			if (elem == desc[i]->mEntryMap.end())
				continue;
			u8 *tptr = target + it->mOffset;
			u8 *sptr = data[i] + elem->second.mOffset;
			Assert(elem->second.mComponents >= it->mComponents);
			if (elem->second.mType == it->mType) {
				Assert(elem->second.mSize == it->mSize);
				for (int k = 0; k < nStructs; k++, tptr += targetDesc.mStride, sptr += desc[i]->mStride)
					memcpy(tptr, sptr, it->mSize * it->mComponents);
				break;
			} else {
				// Must run-time type cast
				for (int k = 0; k < nStructs; k++, tptr += targetDesc.mStride, sptr += desc[i]->mStride) {
					u8 *ttptr = tptr;
					u8 *ssptr = sptr;
					for (unsigned int c = 0; c < it->mComponents; c++, ttptr += it->mSize, ssptr += elem->second.mSize)
						TypeCast(ttptr, it->mType, ssptr, elem->second.mType);
				}
				break;
			}
		}
		if (i == nStreams) {
			LogWarning("Couldn't find entry %s in any of the input struct descriptions", it->mName.c_str());
			continue;
		}
	}
}

/*----------------------------------------------------------------------------*/

void AssembleData(u8 *target, int nStructs, const StructDesc &targetDesc, StructDesc &desc, u8 *data)
{
	StructDesc *srcDesc = &desc;
	AssembleData(target, nStructs, targetDesc, &srcDesc, &data, 1);
}

/*----------------------------------------------------------------------------*/

void WriteData(std::string filename, const StructDesc &desc, u8 *data, int n)
{
	std::ofstream out(filename, std::ios::binary);
	WriteData(out, desc, data, n);
	out.close();
}

void WriteData(std::ostream &os, const StructDesc &desc, u8 *data, int n)
{
	os << int(desc.mEntryList.size()) << " ";
	os << desc.mStride << " ";
	for (auto it = desc.mEntryList.begin(); it != desc.mEntryList.end(); ++it) {
		os 	<< it->mName << " "
			<< it->mOffset << " "
			<< it->mComponents << " "
			<< int(it->mType) << " ";
	}
	os << size_t(n);
	os.write((const char *) data, n * desc.mStride);
}

/*----------------------------------------------------------------------------*/

void ReadData(std::string filename, StructDesc &desc, u8 **data, int &n)
{
	std::ifstream in(filename, std::ios::binary);
	ReadData(in, desc, data, n);
	in.close();
}

void ReadData(std::istream &is, StructDesc &desc, u8 **data, int &n)
{
	int nEntries;
	is >> nEntries;
	is >> desc.mStride;
	for (int i = 0; i < nEntries; i++) {
		std::string name;
		unsigned int offset, components;
		int type;
		is 	>> name >> offset >> components >> type;
		desc.SetEntry(name, offset, components, (types::DataType) type);
	}
	is >> n;
	*data = new u8[n * desc.mStride];
	is.read((char *) *data, n * desc.mStride);
}

/*----------------------------------------------------------------------------*/
