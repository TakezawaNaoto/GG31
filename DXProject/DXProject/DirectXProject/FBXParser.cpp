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
	//--- �f�[�^���ꊇ�œǂݍ���
	FILE* fp;
	fp = fopen(fileName, "rt");
	if (fp == NULL)
	{
		return false;	// �J���Ȃ�����
	}

	// �t�@�C���̃T�C�Y�𒲂ׂ�
	fseek(fp, 0, SEEK_END);
	long fileSize = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	// ���������m�ۂ��ēǍ�
	char* pData = new char[fileSize];
	fread(pData, fileSize, 1, fp);
	fclose(fp);

	//--- �W�I���g���f�[�^�Ǎ�
	char *ptr = pData;
	// �Ǎ��ʒu�܂ňړ�
	ptr = FindKeyword(ptr, "Geometry:");
	char* pGeometry = ptr;	// Geometry�̈ʒu����U�ۑ�
	if (ptr != NULL)
	{
		// ���_�̓Ǎ�
		ptr = FindKeyword(ptr, "Vertices:");
		if (ptr != NULL)
		{
			ptr = FindKeyword(ptr, "*");
			if (ptr != NULL)
			{
				// ���_����ǂݎ��
				int num = atoi(ptr + 1);
				printf("���_�f�[�^���F%d\n", num);
				m_vtx.resize(num / 3);
				// ���_�f�[�^��ǂݎ��
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

		// �C���f�b�N�X�f�[�^�̓ǂݍ���
		IdxList idxWork;
		ptr = FindKeyword(pGeometry, "PolygonVertexIndex:");
		if (ptr != NULL)
		{
			ptr = FindKeyword(ptr, "*");
			if (ptr != NULL)
			{
				// �C���f�b�N�X����ǂݎ��
				int num = atoi(ptr + 1);
				printf("�C���f�b�N�X�f�[�^���F%d\n", num);
				// �C���f�b�N�X�f�[�^��ǂݎ��
				ptr = FindKeyword(ptr, "a:");
				if (ptr != NULL)
				{
					ptr += 2;
					int idxCnt = 0;	// ���p�`(�l�p�`�ȏ�)�͕������邽�߂ɁA���ڂ̃C���f�b�N�X��������
					for (int i = 0; i < num; ++i)
					{
						idxWork.push_back(atoi(ptr));
						ptr = FindKeyword(ptr, ",") + 1;
					}

					// ��(�O�p�`)�̐���\��
					printf("�O�p�`�̐��F%d\n", m_idx.size() / 3);
				}
			}
		}

		// �@���f�[�^�̓ǂݍ���
		NormalList workNormal;
		ptr = FindKeyword(pGeometry, "LayerElementNormal:");
		char* pNormal = ptr;	// LayerElementNormal�̈ʒu����U�ۑ�
		if (ptr != NULL)
		{
			// �}�b�s���O�̓ǂݎ��
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
			// ���t�@�����X�̓ǂݎ��
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
			// �f�o�b�O�\��
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
			// �@���̓ǂݎ��
			ptr = FindKeyword(ptr, "Normals:");
			if (ptr != NULL)
			{
				ptr = FindKeyword(ptr, "*");
				if (ptr != NULL)
				{
					// �@����
					int num = atoi(ptr + 1);
					printf("�@���f�[�^��:%d\n", num);
					workNormal.resize(num / 3);
					// �@���f�[�^�ǂݎ��
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

			// �@���C���f�b�N�X�̓ǂݎ��
			ptr = FindKeyword(pNormal, "NormalsW:");
			if (ptr != NULL)
			{
				ptr = FindKeyword(ptr, "*");
				if (ptr != NULL)
				{
					// �C���f�b�N�X���ǂݎ��
					int num = atoi(ptr + 1);
					printf("�@���C���f�b�N�X��:%d\n", num);
					// �C���f�b�N�X�ǂݎ��
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

					// UV�f�[�^�̓Ǎ�
					UVList workUV;
					IdxList workUVIdx;
					ptr = FindKeyword(pGeometry, "LayerElementUV:");
					char* pUV = ptr;	// LayerElementUV�̈ʒu����������ۑ�
					if (ptr != NULL)
					{
						// �}�b�s���O�̓ǂݎ��
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

						// ���t�@�����X�̓ǂݎ��
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
				

						// �f�o�b�O�\��
#if 1
						const char* MappingName[] = { "ByPolygonVertex", "ByControlPoint", "AllSame" };
						const char* ReferenceName[] = { "Direct", "IndexToDirect" };
						printf("UV[%s / %s]\n", MappingName[m_uvMapping], ReferenceName[m_uvReference]);
#endif
						// UV�̓ǂݎ��
						ptr = FindKeyword(ptr, "UV:");
						if (ptr != NULL)
						{
							ptr = FindKeyword(ptr, "*");
							if (ptr != NULL)
							{
								// UV��
								int num = atoi(ptr + 1);
								printf("UV�f�[�^��:%d\n", num);
								workUV.resize(num / 2);
								// UV�f�[�^�ǂݎ��
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

						// UV�C���f�b�N�X�̓ǂݎ��
						ptr = FindKeyword(pUV, "UVIndex:");
						if (ptr != NULL)
						{
							ptr = FindKeyword(ptr, "*");
							if (ptr != NULL)
							{
								// �C���f�b�N�X���ǂݎ��
								int num = atoi(ptr + 1);
								printf("UV�C���f�b�N�X�ǂݎ��%d\n", num);
								// �C���f�b�N�X�ǂݎ��
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

					// �|���S���Ɩ@����UV�̃C���f�b�N�X��W�J
					bool isNormalMappingPolygon = m_normalMapping == BY_POLYGON_VERTEX;
					bool isUVMappingPolygon = (m_uvMapping == BY_POLYGON_VERTEX);
					int idxCnt = 0;
					for (int i = 0; i < idxWork.size(); ++i)
					{
						int idx = idxWork[i];
						++idxCnt;
						// �O�p�`�ɕ���
						if (idxCnt >= 4)
						{
							m_idx.push_back(m_idx[m_idx.size() - 3]);
							m_idx.push_back(m_idx[m_idx.size() - 2]);
							// �@���𕪊�
							if (isNormalMappingPolygon)
							{
								m_normal.push_back(m_normal[m_normal.size() - 3]);
								m_normal.push_back(m_normal[m_normal.size() - 2]);
							}
							// UV�𕪊�
							if (isUVMappingPolygon)
							{
								// �C���f�b�N�X�����f�[�^�ŕ���
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
					// �|���S���ɕR�Â��ĂȂ���΁A���̂܂ܓǂݎ�����z����R�s�[����
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
	// ���_�ɕR�Â��̂��A�ʂɕR�Â��̂�
	bool isMappingPolygon =
		m_normalMapping == BY_POLYGON_VERTEX;
	// �R�Â���Ő����ς��
	int normNum = isMappingPolygon ?
		m_idx.size() : m_vtx.size();

	for (int i = 0; i < normNum; ++i)
	{
		// �R�Â��ɉ����ĎQ�Ƃ��钸�_��ς���
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

		//// ���_����@�������ɐi�񂾈ʒu���I�_
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
	// �w�肳�ꂽ������̃A�h���X������
	// ��������Ȃ����NULL
	ptr = strstr(ptr, keyword);
	return ptr;
}