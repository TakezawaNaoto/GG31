#pragma once

#include <vector>

class FBXParser
{
public:
	enum Mapping
	{
		BY_POLYGON_VERTEX,	// 三角形(ポリゴン)を構成する頂点に紐づく
		BY_CONTROL_POINT,	// ポリゴンを構成する前の、生データの頂点に紐づく
		ALL_SAME,			// 全部同じ設定
	};
	enum Reference
	{
		DIRECT,				// インデックスを利用しない、直接紐づいたデータ
		INDEX_TO_DIRECT,	// インデックスを利用したデータ設定
	};

	struct Vertex
	{
		float x, y, z;
	};
	struct Normal
	{
		float x, y, z;
	};
	struct UV
	{
		float u, v;
	};
	using VtxList = std::vector<Vertex>;
	using IdxList = std::vector<unsigned long>;
	using NormalList = std::vector<Normal>;
	using UVList = std::vector<UV>;

public:
	FBXParser();
	~FBXParser();

	bool Load(const char* fileName);

// #ifdef _DEBUG
	std::vector<Vertex> DebugDrawNormal();
// #endif // DEBUG


	Vertex* GetVertex() { return m_vtx.data(); }
	int GetVextexNum() { return m_vtx.size(); }
	unsigned long* GetIndex() { return m_idx.data(); }
	int GetIndexNum() { return m_idx.size(); }

	bool IsNormalMappingPolygon() { return m_normalMapping == BY_POLYGON_VERTEX; }
	bool IsNormalReferenceIndex() { return m_normalReference == INDEX_TO_DIRECT; }
	Normal* GetNormal() { return m_normal.data(); }
	unsigned long* GetNormalIndex() { return m_normalIndex.data(); }

	bool IsMappingPolygon() { return m_uvMapping == BY_POLYGON_VERTEX; }
	bool IsUVReferenceIndex() { return m_uvReference == INDEX_TO_DIRECT; }
	UV* GetUV() { return m_uv.data(); }
	unsigned long* GetUVIndex() { return m_uvIndex.data(); }

private:
	char* FindKeyword(char* ptr, const char* keyword);

private:
	VtxList m_vtx;	// 頂点情報
	IdxList m_idx;	// インっデックス(面)情報

	Mapping m_normalMapping;
	Reference m_normalReference;
	NormalList m_normal;
	IdxList m_normalIndex;

	Mapping m_uvMapping;
	Reference m_uvReference;
	UVList m_uv;
	IdxList m_uvIndex;
};