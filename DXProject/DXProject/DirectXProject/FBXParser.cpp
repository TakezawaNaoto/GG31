#include "FBXParser.h"
#include <stdio.h>
#include <string.h>
// #include "glut.h"

FBXParser::FBXParser()
{

}
FBXParser::~FBXParser()
{

}

bool FBXParser::Load(const char* fileName)
{
	//--- データを一括で読み込む
	FILE* fp;
	fp = fopen(fileName, "rt");
	if (fp == NULL)
	{
		return false;	// 開けなかった
	}

	// ファイルのサイズを調べる
	fseek(fp, 0, SEEK_END);
	long fileSize = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	// メモリを確保して読込
	char* pData = new char[fileSize];
	fread(pData, fileSize, 1, fp);
	fclose(fp);

	//--- ジオメトリデータ読込
	char *ptr = pData;
	// 読込位置まで移動
	ptr = FindKeyword(ptr, "Geometry:");
	char* pGeometry = ptr;	// Geometryの位置を一旦保存
	if (ptr != NULL)
	{
		// 頂点の読込
		ptr = FindKeyword(ptr, "Vertices:");
		if (ptr != NULL)
		{
			ptr = FindKeyword(ptr, "*");
			if (ptr != NULL)
			{
				// 頂点数を読み取り
				int num = atoi(ptr + 1);
				printf("頂点データ数：%d\n", num);
				m_vtx.resize(num / 3);
				// 頂点データを読み取り
				ptr = FindKeyword(ptr, "a:");
				if (ptr != NULL)
				{
					ptr += 2;
					for (int i = 0; i < m_vtx.size(); ++i)
					{
						m_vtx[i].x = atof(ptr);
						ptr = FindKeyword(ptr, ",") + 1;
						m_vtx[i].y = atof(ptr);
						ptr = FindKeyword(ptr, ",") + 1;
						m_vtx[i].z = atof(ptr);
						ptr = FindKeyword(ptr, ",") + 1;
					}
				}
			}
		}

		// インデックスデータの読み込み
		IdxList idxWork;
		ptr = FindKeyword(pGeometry, "PolygonVertexIndex:");
		if (ptr != NULL)
		{
			ptr = FindKeyword(ptr, "*");
			if (ptr != NULL)
			{
				// インデックス数を読み取り
				int num = atoi(ptr + 1);
				printf("インデックスデータ数：%d\n", num);
				// インデックスデータを読み取り
				ptr = FindKeyword(ptr, "a:");
				if (ptr != NULL)
				{
					ptr += 2;
					int idxCnt = 0;	// 多角形(四角形以上)は分割するために、何個目のインデックスか数える
					for (int i = 0; i < num; ++i)
					{
						idxWork.push_back(atoi(ptr));
						ptr = FindKeyword(ptr, ",") + 1;
					}

					// 面(三角形)の数を表示
					printf("三角形の数：%d\n", m_idx.size() / 3);
				}
			}
		}

		// 法線データの読み込み
		NormalList workNormal;
		ptr = FindKeyword(pGeometry, "LayerElementNormal:");
		char* pNormal = ptr;	// LayerElementNormalの位置を一旦保存
		if (ptr != NULL)
		{
			// マッピングの読み取り
			ptr = FindKeyword(ptr, "MappingInformationType:");
			if (ptr != NULL)
			{
				ptr = FindKeyword(ptr, "\"");
				if (ptr != NULL)
				{
					ptr += 1;
					if(ptr == strstr(ptr, "ByPolygonVertex"))
					{
						m_normalMapping = BY_POLYGON_VERTEX;
					}
					else if(ptr == strstr(ptr, "ByControlPoint"))
					{
						m_normalMapping = BY_CONTROL_POINT;
					}
					else
					{
						m_normalMapping = ALL_SAME;
					}
				}
			}
			else
			{
				m_normalMapping = ALL_SAME;
			}
			// リファレンスの読み取り
			ptr = FindKeyword(ptr, "ReferenceInformationType");
			if (ptr != NULL)
			{
				// Direct
				if (ptr == strstr(ptr, "Direct"))
				{
					m_normalReference = DIRECT;
				}
				// IndexToDirect
				else if(ptr == strstr(ptr, "IndexToDirect"))
				{
					m_normalReference = INDEX_TO_DIRECT;
				}
				else
				{
					m_normalReference = DIRECT;
				}
			}
			// デバッグ表示
			printf("Normal[");
			switch (m_normalMapping)
			{
			case BY_POLYGON_VERTEX: printf("ByPolygonVertex"); break;
			case BY_CONTROL_POINT:  printf("ByControlPoint"); break;
			default:				printf("AllSame"); break;
			}
			printf(" / ");
			switch (m_normalReference)
			{
			case INDEX_TO_DIRECT:	printf("IndexToDirect"); break;
			default:				printf("Direct"); break;
			}
			printf("]\n");
#if(0)
			const char* MappingName[] = { "ByPolygonVertex", "ByControlPoint", "AllSame" };
			const char* ReferenceName[] = { "Direct", "IndexToDirect" };
			printf("Normal[%s / %s]\n", MappingName[m_normalMapping], ReferenceName[m_normalReference]);
#endif
			// 法線の読み取り
			ptr = FindKeyword(ptr, "Normals:");
			if (ptr != NULL)
			{
				ptr = FindKeyword(ptr, "*");
				if (ptr != NULL)
				{
					// 法線数
					int num = atoi(ptr + 1);
					printf("法線データ数:%d\n", num);
					workNormal.resize(num / 3);
					// 法線データ読み取り
					ptr = FindKeyword(ptr, "a:");
					if (ptr != NULL)
					{
						ptr += 2;
						for (int i = 0; i < workNormal.size(); ++i)
						{
							workNormal[i].x = atof(ptr);
							ptr = FindKeyword(ptr, ",") + 1;
							workNormal[i].y = atof(ptr);
							ptr = FindKeyword(ptr, ",") + 1;
							workNormal[i].z = atof(ptr);
							ptr = FindKeyword(ptr, ",") + 1;
						}
					}
				}
			}

			// 法線インデックスの読み取り
			ptr = FindKeyword(pNormal, "NormalsW:");
			if (ptr != NULL)
			{
				ptr = FindKeyword(ptr, "*");
				if (ptr != NULL)
				{
					// インデックス数読み取り
					int num = atoi(ptr + 1);
					printf("法線インデックス数:%d\n", num);
					// インデックス読み取り
					ptr = FindKeyword(ptr, "a:");
					if (ptr != NULL)
					{
						ptr += 2;
						for (int i = 0; i < num; ++i)
						{
							m_normalIndex.push_back(atoi(ptr));
							ptr = FindKeyword(ptr, ",") + 1;
						}
					}

					// UVデータの読込
					UVList workUV;
					IdxList workUVIdx;
					ptr = FindKeyword(pGeometry, "LayerElementUV:");
					char* pUV = ptr;	// LayerElementUVの位置をいったん保存
					if (ptr != NULL)
					{
						// マッピングの読み取り
						ptr = FindKeyword(ptr, "MappingInformationType:");
						if (ptr != NULL)
						{
							ptr = FindKeyword(ptr, "\"");
							if (ptr != NULL)
							{
								ptr += 1;
								if (ptr == strstr(ptr, "ByPolygonVertex"))
								{
									m_uvMapping = BY_POLYGON_VERTEX;
								}
								else if (ptr == strstr(ptr, "ByContolPoint"))
								{
									m_uvMapping = BY_CONTROL_POINT;
								}
								else
								{
									m_uvMapping = ALL_SAME;
								}
							}
						}
						else
						{
							m_uvMapping = ALL_SAME;
						}

						// リファレンスの読み取り
						ptr = FindKeyword(ptr, "ReferenceInformationType");
						if (ptr != NULL)
						{
							ptr = FindKeyword(ptr, "\"");
							if (ptr)
							{
								++ptr;
								// Direct
								if (ptr == strstr(ptr, "Direct"))
								{
									m_uvReference = DIRECT;
								}
								// IndexToDirect
								else if (ptr == strstr(ptr, "IndexToDirect"))
								{
									m_uvReference = INDEX_TO_DIRECT;
								}
								else
								{
									m_uvReference = DIRECT;
								}
							}
						}
						else
						{
							m_uvReference = DIRECT;
						}
				

						// デバッグ表示
#if 1
						const char* MappingName[] = { "ByPolygonVertex", "ByControlPoint", "AllSame" };
						const char* ReferenceName[] = { "Direct", "IndexToDirect" };
						printf("UV[%s / %s]\n", MappingName[m_uvMapping], ReferenceName[m_uvReference]);
#endif
						// UVの読み取り
						ptr = FindKeyword(ptr, "UV:");
						if (ptr != NULL)
						{
							ptr = FindKeyword(ptr, "*");
							if (ptr != NULL)
							{
								// UV数
								int num = atoi(ptr + 1);
								printf("UVデータ数:%d\n", num);
								workUV.resize(num / 2);
								// UVデータ読み取り
								ptr = FindKeyword(ptr, "a:");
								if (ptr != NULL)
								{
									ptr += 2;
									for (int i = 0; i < workUV.size(); ++i)
									{
										workUV[i].u = atof(ptr);
										ptr = FindKeyword(ptr, ",") + 1;
										workUV[i].v = atof(ptr);
										ptr = FindKeyword(ptr, ",") + 1;
									}
								}
							}
						}

						// UVインデックスの読み取り
						ptr = FindKeyword(pUV, "UVIndex:");
						if (ptr != NULL)
						{
							ptr = FindKeyword(ptr, "*");
							if (ptr != NULL)
							{
								// インデックス数読み取り
								int num = atoi(ptr + 1);
								printf("UVインデックス読み取り%d\n", num);
								// インデックス読み取り
								ptr = FindKeyword(ptr, "a:");
								if (ptr != NULL)
								{
									ptr += 2;
									for (int i = 0; i < num; ++i)
									{
										workUVIdx.push_back(atoi(ptr));
										ptr = FindKeyword(ptr, ",") + 1;
									}
								}
							}
						}
					}

					// ポリゴンと法線とUVのインデックスを展開
					bool isNormalMappingPolygon = m_normalMapping == BY_POLYGON_VERTEX;
					bool isUVMappingPolygon = (m_uvMapping == BY_POLYGON_VERTEX);
					int idxCnt = 0;
					for (int i = 0; i < idxWork.size(); ++i)
					{
						int idx = idxWork[i];
						++idxCnt;
						// 三角形に分割
						if (idxCnt >= 4)
						{
							m_idx.push_back(m_idx[m_idx.size() - 3]);
							m_idx.push_back(m_idx[m_idx.size() - 2]);
							// 法線を分割
							if (isNormalMappingPolygon)
							{
								m_normal.push_back(m_normal[m_normal.size() - 3]);
								m_normal.push_back(m_normal[m_normal.size() - 2]);
							}
							// UVを分割
							if (isUVMappingPolygon)
							{
								// インデックスが直データで分割
								if (IsUVReferenceIndex())
								{
									m_uvIndex.push_back(m_uvIndex[m_uvIndex.size() - 3]);
									m_uvIndex.push_back(m_uvIndex[m_uvIndex.size() - 2]);
								}
								else
								{
									m_uv.push_back(m_uv[m_uv.size() - 3]);
									m_uv.push_back(m_uv[m_uv.size() - 2]);
								}
							}
						}
					if (idx < 0)
					{
						idx = ~idx;
						idxCnt = 0;
					}
					m_idx.push_back(idx);
					if (isNormalMappingPolygon)
					{
						m_normal.push_back(workNormal[i]);
					}
					if (isUVMappingPolygon)
					{
						if (IsUVReferenceIndex())
						{
							m_uvIndex.push_back(workUVIdx[i]);
						}
						else
						{
							//e
							m_uv.push_back(workUV[i]);
						}
					}
				}
					// ポリゴンに紐づいてなければ、そのまま読み取った配列をコピーする
					if (!isNormalMappingPolygon)
					{
						m_normal = workNormal;
					}
					if (!IsMappingPolygon())
					{
						m_uv = workUV;
						m_uvIndex = workUVIdx;
					}
					else
					{
						if (IsUVReferenceIndex()) { m_uv = workUV; }
						else { m_uvIndex = idxWork; }
					}
				}
			}
		}
	}

	delete[] pData;
	return true;
}

std::vector<FBXParser::Vertex> FBXParser::DebugDrawNormal()
{
	std::vector<Vertex> retVtx;
	Vertex vtx;
	// 頂点に紐づくのか、面に紐づくのか
	bool isMappingPolygon =
		m_normalMapping == BY_POLYGON_VERTEX;
	// 紐づき先で数が変わる
	int normNum = isMappingPolygon ?
		m_idx.size() : m_vtx.size();

	for (int i = 0; i < normNum; ++i)
	{
		// 紐づけに応じて参照する頂点を変える
		int idx = isMappingPolygon ?
			m_idx[i] : i;
		vtx.x = m_vtx[idx].x;
		vtx.y = m_vtx[idx].y;
		vtx.z = m_vtx[idx].z;
		retVtx.push_back(vtx);

		if (i >= m_normal.size()) continue;
		vtx.x = m_vtx[idx].x + m_normal[i].x;
		vtx.y = m_vtx[idx].y + m_normal[i].y;
		vtx.z = m_vtx[idx].z + m_normal[i].z;
		retVtx.push_back(vtx);

		//// 頂点から法線方向に進んだ位置を終点
		//if (i >= m_normal.size()) continue;
		//glVertex3f(
		//	m_vtx[idx].x + m_normal[i].x,
		//	m_vtx[idx].y + m_normal[i].y,
		//	m_vtx[idx].z + m_normal[i].z);
		
	}
	return retVtx;
}

char* FBXParser::FindKeyword(char* ptr, const char* keyword)
{
	// 指定された文字列のアドレスを検索
	// ※見つからなければNULL
	ptr = strstr(ptr, keyword);
	return ptr;
}