#pragma once

#include <vector>

class FBXParser
{
public:
	enum Mapping
	{
		BY_POLYGON_VERTEX,	// �O�p�`(�|���S��)���\�����钸�_�ɕR�Â�
		BY_CONTROL_POINT,	// �|���S�����\������O�́A���f�[�^�̒��_�ɕR�Â�
		ALL_SAME,			// �S�������ݒ�
	};
	enum Reference
	{
		DIRECT,				// �C���f�b�N�X�𗘗p���Ȃ��A���ڕR�Â����f�[�^
		INDEX_TO_DIRECT,	// �C���f�b�N�X�𗘗p�����f�[�^�ݒ�
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
	VtxList m_vtx;	// ���_���
	IdxList m_idx;	// �C�����f�b�N�X(��)���

	Mapping m_normalMapping;
	Reference m_normalReference;
	NormalList m_normal;
	IdxList m_normalIndex;

	Mapping m_uvMapping;
	Reference m_uvReference;
	UVList m_uv;
	IdxList m_uvIndex;
};