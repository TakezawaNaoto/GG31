#include "FBXPlayer.h"
#include "Geometory.h"

FBXPlayer::FBXPlayer()
	: m_pBuffers(nullptr)
	, m_meshNum(0)
	, m_pFBXBone(nullptr)
	, m_pBones(nullptr)
	, m_playAnimeNo(-1)
	, m_blendAnimeNo(-1)
	, m_blendFrame(0)
{
	for (int i = 0; i < MaxAnime; ++i)
	{
		m_pAnimes[i] = nullptr;
	}
	for (int i = 0; i < MaxBlend; ++i)
	{
		m_pBlendBones[i] = nullptr;
	}
}
FBXPlayer::~FBXPlayer()
{
	Reset();
}

bool FBXPlayer::Load(const char* fileName)
{
	// �������̃N���A
	Reset();

	// �ǂݍ���
	m_pFBXBone = new ggfbx::BoneListInfo;
	ggfbx::MeshList mesh;
	if (!ggfbx::Load(fileName, &mesh, m_pFBXBone, nullptr, nullptr))
	{
		return false;
	}

	// �o�b�t�@�쐬
	m_meshNum = mesh.size();
	m_pBuffers = new DXBuffer[mesh.size()];
	auto meshIt = mesh.begin();
	while (meshIt != mesh.end())
	{
		// �C���f�b�N�X�o�b�t�@�𗘗p���ĕ`�悷�邩����
		bool isIdxDraw;
		isIdxDraw = true;
		if (meshIt->normalInfo.mapping == ggfbx::MAPPING_INDEX)
		{
			isIdxDraw = false;
		}
		if (meshIt->uvList.size() > 0 && meshIt->uvList[0].mapping == ggfbx::MAPPING_INDEX)
		{
			// �P��uv�݂̂ɑΉ�
			isIdxDraw = false;
		}

		// ���_�o�b�t�@�쐬
		int vtxNum;
		if (isIdxDraw)
		{
			vtxNum = static_cast<int>(meshIt->vertexList.size());
		}
		else
		{
			vtxNum = static_cast<int>(meshIt->indexList.size());
		}
		FBXVertex* pVtx = new FBXVertex[vtxNum];
		FBXVertex* pVtxTop = pVtx;
		for (int i = 0; i < vtxNum; ++i)
		{
			// ���ꂼ��̃C���f�b�N�X���擾
			int vtxIdx = i;
			int normIdx = i;
			int uvIdx = i;
			if (!isIdxDraw)
			{
				vtxIdx = meshIt->indexList[i];
				if (meshIt->normalInfo.mapping == ggfbx::MAPPING_VERTEX)
				{
					normIdx = vtxIdx;
				}
				if (meshIt->uvList.size() > 0 && meshIt->uvList[0].mapping == ggfbx::MAPPING_VERTEX)
				{
					uvIdx = vtxIdx;
				}
			}

			// ���_
			DirectX::XMVECTOR vec;
			vec = DirectX::XMVector3TransformCoord(
				DirectX::XMVectorSet(meshIt->vertexList[vtxIdx].x, meshIt->vertexList[vtxIdx].y, meshIt->vertexList[vtxIdx].z, 0),
				DirectX::XMMatrixSet(
					meshIt->transform._11, meshIt->transform._12, meshIt->transform._13, meshIt->transform._14,
					meshIt->transform._21, meshIt->transform._22, meshIt->transform._23, meshIt->transform._24,
					meshIt->transform._31, meshIt->transform._32, meshIt->transform._33, meshIt->transform._34,
					meshIt->transform._41, meshIt->transform._42, meshIt->transform._43, meshIt->transform._44)
			);
			DirectX::XMStoreFloat3(&pVtx->pos, vec);
			// �J���[
			pVtx->color = DirectX::XMFLOAT4(1, 1, 1, 1);
			// UV
			if (meshIt->uvList.size() > 0)
			{
				pVtx->uv.x = meshIt->uvList[0].list[uvIdx].x;
				pVtx->uv.y = 1.0f - meshIt->uvList[0].list[uvIdx].y;
			}
			else
			{
				pVtx->uv = DirectX::XMFLOAT2(0.0f, 0.0f);
			}
			// �@��
			if (meshIt->normalInfo.list.size() > 0)
			{
				vec = DirectX::XMVector3TransformCoord(
					DirectX::XMVectorSet(meshIt->normalInfo.list[normIdx].x,	meshIt->normalInfo.list[normIdx].y, meshIt->normalInfo.list[normIdx].z, 0),
					DirectX::XMMatrixSet(
						meshIt->transform._11, meshIt->transform._12, meshIt->transform._13, 0,
						meshIt->transform._21, meshIt->transform._22, meshIt->transform._23, 0,
						meshIt->transform._31, meshIt->transform._32, meshIt->transform._33, 0,
						0, 0, 0, 1)
				);
				DirectX::XMStoreFloat3(&pVtx->normal, vec);
			}
			else
			{
				pVtx->normal = DirectX::XMFLOAT3(0, 0, 1);
			}
			// �E�F�C�g
			if (meshIt->skinInfo.weightList.size() > 0)
			{
				for (int j = 0; j < ggfbx::MeshInfo::SkinInfo::WeightInfo::WEIGHT_NUM_MAX; ++j)
				{
					auto value = meshIt->skinInfo.weightList[vtxIdx].value[j];
					pVtx->weight[j] = value.weight;
					pVtx->index[j] = value.index;
				}
			}
			else
			{
				for (int j = 0; j < ggfbx::MeshInfo::SkinInfo::WeightInfo::WEIGHT_NUM_MAX; ++j)
				{
					pVtx->weight[j] = 0.0f;
					pVtx->index[j] = 0;
				}
				pVtx->weight[0] = 1.0f;
			}

			++pVtx;
		}
		
		// �o�b�t�@�\�z
		DXBuffer::Desc desc = {};
		desc.pVtx = pVtxTop;
		desc.vtxSize = sizeof(FBXVertex);
		desc.vtxCount = vtxNum;
		desc.topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

		if (isIdxDraw)
		{
			long* pIdx = new long[meshIt->indexList.size()];
			for (int i = 0; i < meshIt->indexList.size(); ++i)
			{
				pIdx[i] = meshIt->indexList[i];
			}
			desc.pIdx = pIdx;
			desc.idxSize = sizeof(long);
			desc.idxCount = static_cast<UINT>(meshIt->indexList.size());
			m_pBuffers[meshIt - mesh.begin()].Create(desc);
			delete[] pIdx;
		}
		else
		{
			m_pBuffers[meshIt - mesh.begin()].Create(desc);
		}
		delete[] pVtxTop;

		++meshIt;
	}





	// �Q�[���Ŏg�p����`�ɋz�o��
	/*
	 STL ... �W���e���v���[�g���C�u����(Standard Template Library)
	 std::vector ���I�z��
	 std::list ���I���X�g
	*/
	int boneNum = m_pFBXBone->list.size();
	m_pBones = new BoneInfo[boneNum];
	m_pBlendBones[0] = new MatrixParam[boneNum];
	m_pBlendBones[1] = new MatrixParam[boneNum];
	for (int i = 0; i < m_pFBXBone->list.size(); ++i)
	{
		DirectX::XMStoreFloat4x4(
			&m_pBones[i].matrix, DirectX::XMMatrixIdentity());
		// ���O��]�̌v�Z
		DirectX::XMMATRIX rotX =
			DirectX::XMMatrixRotationX(
				DirectX::XMConvertToRadians(m_pFBXBone->list[i].preRotate.x));
		DirectX::XMMATRIX rotY =
			DirectX::XMMatrixRotationY(
				DirectX::XMConvertToRadians(m_pFBXBone->list[i].preRotate.y));
		DirectX::XMMATRIX rotZ =
			DirectX::XMMatrixRotationZ(
				DirectX::XMConvertToRadians(m_pFBXBone->list[i].preRotate.z));
		m_pBones[i].preRotate = rotX * rotY * rotZ;
		// FBX����ǂݍ��񂾍��̏����p�����A�Q�[�����Ŏg�p���鍜�ɃR�s�[����
		m_pBones[i].param.translate.x = m_pFBXBone->list[i].translate.x;
		m_pBones[i].param.translate.y = m_pFBXBone->list[i].translate.y;
		m_pBones[i].param.translate.z = m_pFBXBone->list[i].translate.z;
		m_pBones[i].param.rotation.x = m_pFBXBone->list[i].rotation.x;
		m_pBones[i].param.rotation.y = m_pFBXBone->list[i].rotation.y;
		m_pBones[i].param.rotation.z = m_pFBXBone->list[i].rotation.z;
		m_pBones[i].param.scale.x = m_pFBXBone->list[i].scale.x;
		m_pBones[i].param.scale.y = m_pFBXBone->list[i].scale.y;
		m_pBones[i].param.scale.z = m_pFBXBone->list[i].scale.z;
		// ���S����(�������g������������ĂȂ���Ԃ������)��
		// �J�ڗp�̍��ɏ����p���̏���ݒ�
		m_pBlendBones[0][i] = m_pBones[i].param;
		m_pBlendBones[1][i] = m_pBones[i].param;
	}

	// �X�L�����b�V���p�̃f�[�^�\�z
	// (���b�V���ƃ{�[�������т���
	meshIt = mesh.begin();
	while (meshIt != mesh.end())
	{
		if (meshIt->skinInfo.boneList.size() > 0)
		{
			MeshInverse meshInv;
			meshInv.num = meshIt->skinInfo.boneList.size();
			meshInv.pList = new MeshInverse::List[meshInv.num];
			for (int i = 0; i < meshInv.num; ++i)
			{
				// ���b�V���ɕR�Â��Ă鍜���A���S�̂̔z���
				// �ǂ̈ʒu�ɏ������Ă��邩
				for (int j = 0; j < m_pFBXBone->list.size(); ++j)
				{
					if (meshIt->skinInfo.boneList[i].name ==
						m_pFBXBone->list[j].name)
					{
						meshInv.pList[i].boneIndex = j;
						// �t�s��̌v�Z
						DirectX::XMFLOAT4X4 offset(
							meshIt->skinInfo.boneList[i].offset[0]);
						meshInv.pList[i].invMat =
							DirectX::XMMatrixInverse(nullptr,
								DirectX::XMLoadFloat4x4(&offset));	
						break;
					}
				}
			}
			// �\�z�����f�[�^�𓮓I�z��ɒǉ�
			m_meshInverse.push_back(meshInv);
		}
		++meshIt;
	}
}

// �߂�l�Ŕz��̂ǂ̈ʒu�ɃA�j���[�V�����̏����i�[�������Ԃ�
// �ǂݍ��߂Ȃ���������-1��Ԃ�
int FBXPlayer::LoadAnime(const char* fileName)
{
	// �z��ɋ󂫂����邩
	int animeIndex = -1;
	for (int i = 0; i < MaxAnime; ++i)
	{
		if (m_pAnimes[i] == nullptr)
		{
			animeIndex = i;
			break;
		}
	}
	if (animeIndex == -1)
	{
		return animeIndex;
	}

	// �A�j���[�V�����̓ǂݍ���
	m_pAnimes[animeIndex] = new AnimeInfo;
	m_pAnimes[animeIndex]->pAnime = new ggfbx::AnimationInfo;
	if (!ggfbx::Load(fileName, nullptr, nullptr, nullptr, m_pAnimes[animeIndex]->pAnime))
	{
		// �ǂݍ��݂Ɏ��s������A�z��̊Y���ӏ��𑁋}�ɋ󂯂�
		delete m_pAnimes[animeIndex]->pAnime;
		delete m_pAnimes[animeIndex];
		m_pAnimes[animeIndex] = nullptr;
		return -1;
	}

	// ���̑�������
	m_pAnimes[animeIndex]->playFrame = 0.0f;
	m_pAnimes[animeIndex]->loop = false;
	m_pAnimes[animeIndex]->speed = 1.0f;

	return animeIndex;
}

// �A�j���[�V�����̍Đ��w��
void FBXPlayer::PlayAnime(int animeNo, bool isLoop)
{
	// �Đ��`�F�b�N
	if (animeNo < 0 || animeNo >= MaxAnime || !m_pAnimes[animeNo])
	{
		return;
	}

	m_playAnimeNo = animeNo;
	m_pAnimes[m_playAnimeNo]->playFrame = 0.0f;
	m_pAnimes[m_playAnimeNo]->loop = isLoop;
}
// �J�ڱ�Ұ��݂̎w��
void FBXPlayer::PlayBlendAnime(int blendNo, float time, bool isLoop)
{
	// �Đ��`�F�b�N
	if (m_playAnimeNo == blendNo)
	{
		return;
	}
	if (blendNo < 0 || blendNo >= MaxAnime || !m_pAnimes[blendNo])
	{
		return;
	}

	m_blendAnimeNo = blendNo;
	m_blendFrame = time * 60;
	m_pAnimes[m_blendAnimeNo]->playFrame = 0.0f;
	m_pAnimes[m_blendAnimeNo]->loop = isLoop;
}

// �A�j���[�V�����Đ�����
bool FBXPlayer::IsPlayAnime(int animeNo)
{
	// �w�肳�ꂽ�A�j���[�V�������Đ�����Ă��邩
	if (animeNo != -1)
	{
		if (m_playAnimeNo != animeNo)
		{
			return false;
		}
	}

	// ���[�v�A�j���Ȃ��΍Đ����Ă���
	if (m_pAnimes[m_playAnimeNo]->loop)
	{
		return true;
	}
	// �P���Đ��A�j���̏I���t���[���܂ōĐ�������
	if (m_pAnimes[m_playAnimeNo]->playFrame >=
		m_pAnimes[m_playAnimeNo]->pAnime->totalFrame)
	{
		return false;
	}

	// �܂��Đ����̃A�j��������
	return true;
}

void FBXPlayer::Step(float speed)
{
	// �A�j���[�V�����̍X�V
	if (m_playAnimeNo != -1)
	{
		// ���ݍĐ����̃A�j���[�V�������v�Z
		CalcAnimeFrame(m_playAnimeNo, speed);
		CalcAnime(m_playAnimeNo, m_pBlendBones[0]);

		// �J�ڃA�j���̍X�V����
		if (m_blendAnimeNo != -1)
		{
			//--------------
			// ���[�V�����̍������킹(�u�����h���[�V����)
			//--------------
			// �u�����h������A�j���[�V�����̌v�Z
			CalcAnimeFrame(m_blendAnimeNo, speed);
			CalcAnime(m_blendAnimeNo, m_pBlendBones[1]);

			// �A�j���[�V�����̑J�ڎ��Ԃ̔䗦
			float rate = m_pAnimes[m_blendAnimeNo]->playFrame / m_blendFrame;

			// ����t���[���ɂ����āA�ʁX�̃A�j���[�V�����̒l���Ԃ��邱�ƂŁA
			// �A�j���[�V�����̍������킹����������(�S�Ă̍����Ώ�
			for (int i = 0; i < m_pFBXBone->list.size(); ++i)
			{
				// �e���̊e�p�����[�^����
				m_pBones[i].param.translate.x =
					m_pBlendBones[0][i].translate.x * (1.0f - rate) +
					m_pBlendBones[1][i].translate.x * rate;
				m_pBones[i].param.translate.y =
					m_pBlendBones[0][i].translate.y * (1.0f - rate) +
					m_pBlendBones[1][i].translate.y * rate;
				m_pBones[i].param.translate.z =
					m_pBlendBones[0][i].translate.z * (1.0f - rate) +
					m_pBlendBones[1][i].translate.z * rate;

				m_pBones[i].param.rotation.x =
					m_pBlendBones[0][i].rotation.x * (1.0f - rate) +
					m_pBlendBones[1][i].rotation.x * rate;
				m_pBones[i].param.rotation.y =
					m_pBlendBones[0][i].rotation.y * (1.0f - rate) +
					m_pBlendBones[1][i].rotation.y * rate;
				m_pBones[i].param.rotation.z =
					m_pBlendBones[0][i].rotation.z * (1.0f - rate) +
					m_pBlendBones[1][i].rotation.z * rate;

				m_pBones[i].param.scale.x =
					m_pBlendBones[0][i].scale.x * (1.0f - rate) +
					m_pBlendBones[1][i].scale.x * rate;
				m_pBones[i].param.scale.y =
					m_pBlendBones[0][i].scale.y * (1.0f - rate) +
					m_pBlendBones[1][i].scale.y * rate;
				m_pBones[i].param.scale.z =
					m_pBlendBones[0][i].scale.z * (1.0f - rate) +
					m_pBlendBones[1][i].scale.z * rate;
			}

			// �u�����h�ł͂Q�̃A�j���[�V��������ɍĐ�
			// ���Ă���̂ŁA�J�ڂ��I���������̃A�j���[�V����
			// �݂̂̍Đ��ɖ߂�
			if (rate >= 1.0f)
			{
				m_playAnimeNo = m_blendAnimeNo;
				m_blendAnimeNo = -1;
			}


			//--------------
		}
		else
		{
			// �J�ڂ��Ȃ��̂ŒP���f�[�^���R�s�[
			for (int i = 0; i < m_pFBXBone->list.size(); ++i)
			{
				m_pBones[i].param = m_pBlendBones[0][i];
			}
		}
	}

	// ���̎p���X�V
	CalcBone(m_pFBXBone->root, DirectX::XMMatrixIdentity());
}
void FBXPlayer::Draw(void(*PreCallBack)(int mesh))
{
	// ���f���̕`��
	for (int i = 0; i < m_meshNum; ++i)
	{
		if (PreCallBack != nullptr)
		{
			PreCallBack(i);
		}
		m_pBuffers[i].Draw();
	}

}
void FBXPlayer::DrawBone()
{
	CalcDrawBone(m_pFBXBone->root, DirectX::XMFLOAT3(0, 0, 0));
	DrawLines();
}

DirectX::XMFLOAT4X4 FBXPlayer::GetBone(int index)
{
	if (index < 0 || m_pFBXBone->list.size() <= index)
	{
		return DirectX::XMFLOAT4X4(
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1);
	}

	return m_pBones[index].matrix;
}

std::vector<FBXPlayer::MeshInverse>& FBXPlayer::GetMeshInverse()
{
	return m_meshInverse;
}

void FBXPlayer::Reset()
{
	if (m_pBuffers)
	{
		delete[] m_pBuffers;
		m_pBuffers = nullptr;
	}
	for (int i = 0; i < m_meshInverse.size(); ++i)
	{
		delete[] m_meshInverse[i].pList;
	}
	m_meshInverse.clear();
	if (m_pFBXBone)
	{
		delete m_pFBXBone;
		m_pFBXBone = nullptr;
	}
	if (m_pBones)
	{
		delete[] m_pBones;
		m_pBones = nullptr;
	}
	for (int i = 0; i < MaxAnime; ++i)
	{
		if (m_pAnimes[i])
		{
			delete m_pAnimes[i]->pAnime;
			delete m_pAnimes[i];
			m_pAnimes[i] = nullptr;
		}
	}
	for (int i = 0; i < MaxBlend; ++i)
	{
		if (m_pBlendBones[i])
		{
			delete m_pBlendBones[i];
			m_pBlendBones[i] = nullptr;
		}
	}
}

/// @param index ���̔ԍ�
/// @param parent �e�̎p�����
void FBXPlayer::CalcBone(int index, DirectX::XMMATRIX parent)
{
	//--------------
	// ���̎p���v�Z
	//--------------
	// ���P�̂̎p���s����v�Z
	DirectX::XMMATRIX T =	// �ЂƂO�̍��ɑ΂��Ă̑��΋���
		DirectX::XMMatrixTranslation(
			m_pBones[index].param.translate.x,
			m_pBones[index].param.translate.y,
			m_pBones[index].param.translate.z
		);
	DirectX::XMMATRIX R =
		DirectX::XMMatrixRotationX(DirectX::XMConvertToRadians(m_pBones[index].param.rotation.x)) *
		DirectX::XMMatrixRotationY(DirectX::XMConvertToRadians(m_pBones[index].param.rotation.y)) *
		DirectX::XMMatrixRotationZ(DirectX::XMConvertToRadians(m_pBones[index].param.rotation.z));

	DirectX::XMMATRIX S =
	DirectX::XMMatrixScaling(
		m_pBones[index].param.scale.x,
		m_pBones[index].param.scale.y,
		m_pBones[index].param.scale.z
	);
	// ���̎p�� = �v�Z���������̃��[�J���p�� * ���܂Ŏp�����v�Z���Ă����S�Ă̐e�̎p��
	// ���r�̍��Ȃ� = �r�̍��̃��[�J�� * �S�Ă̍�(��̘r * �㔼�g * ��)�̎p��
	DirectX::XMMATRIX mat = S * R * m_pBones[index].preRotate * T * parent;

	// �v�Z�ς݂̍�����ۑ�
	DirectX::XMStoreFloat4x4(&m_pBones[index].matrix, mat);


	// ���ɑ������̌v�Z
	// ���������v�Z������́A[�e�w�A�l�����w�A���w�A��w�A���w]�ƌv�Z������
	for (int i = 0; i < m_pFBXBone->list[index].children.size(); ++i)
	{
		CalcBone(m_pFBXBone->list[index].children[i], mat);
	}


	//--------------
}
// ����`�悷��֐�
void FBXPlayer::CalcDrawBone(int index, DirectX::XMFLOAT3 parent)
{
	//--------------
	// ���̕`��
	//--------------
	// �e�̈ʒu����L�т��I�_�̈ʒu���v�Z
	DirectX::XMVECTOR vPos = DirectX::XMVectorSet(0, 0, 0, 0);
	DirectX::XMMATRIX matPose =
		DirectX::XMLoadFloat4x4(&m_pBones[index].matrix);
	vPos = DirectX::XMVector3Transform(vPos, matPose);/*���_���W���p���s��ňړ�������*/

	// �v�Z�ł����̂ō��̑�p�Œ�����`��
	DirectX::XMFLOAT3 pos;
	DirectX::XMStoreFloat3(&pos, vPos);
	AddLine(parent, pos);

	// ���������̂ł���Ζ��[�܂Ōv�Z
	for (int i = 0; i < m_pFBXBone->list[index].children.size(); i++)
	{
		CalcDrawBone(
			m_pFBXBone->list[index].children[i],
			// ���̍��̐e�ƂȂ���W�́A���v�Z�������W�̂͂��I
			pos
			);
	}
	//--------------
}

// �A�j���[�V�����̎��Ԍo�߂��v�Z
void FBXPlayer::CalcAnimeFrame(int animeNo, float speed)
{
	if (animeNo < 0 || animeNo >= MaxAnime || !m_pAnimes[animeNo])
	{
		return;
	}

	m_pAnimes[animeNo]->playFrame += speed;
	// ���[�v����
	if (m_pAnimes[animeNo]->loop)
	{
		while (m_pAnimes[animeNo]->playFrame >=
			m_pAnimes[animeNo]->pAnime->totalFrame)
		{
			m_pAnimes[animeNo]->playFrame -=
				m_pAnimes[animeNo]->pAnime->totalFrame;
		}
	}
}
// �A�j���[�V�����̃t���[���ɉ����č��̃|�[�Y���v�Z
void FBXPlayer::CalcAnime(int animeNo, MatrixParam* pBones)
{
	// �Đ��`�F�b�N
	if (animeNo < 0 || animeNo >= MaxAnime || !m_pAnimes[animeNo])
	{
		return;
	}

	//--------------
	// �A�j���[�V�����œ����������̌v�Z�i�u�����h���݂̌v�Z
	//--------------

	// �����Ŏw�肳�ꂽ�A�j���[�V�����̏����擾
	ggfbx::AnimationInfo* pAnime = m_pAnimes[animeNo]->pAnime;

	// �A�j���[�V�����̃f�[�^�̂����A�����Ƃɍׂ����f�[�^���擾
	for (int i = 0; i < pAnime->curveList.size(); ++i)
	{
		// �A�j���[�V�����̍��ƁA���f���̍��̃f�[�^�͌����ɂ͈Ⴄ���߁A
		// ���т���K�v������B�i���݂��鍜���A�j���[�V���������Ȃ���
		// ���������Ȃ�B
		int boneIndex = -1;
		for (int j = 0; j < m_pFBXBone->list.size(); ++j)
		{
			if (pAnime->curveList[i].name == m_pFBXBone->list[j].name)
			{
				boneIndex = j;
				break;
			}
		}
		// �Ή�����A�j���[�V�����Ȃ�
		if (boneIndex == -1)
		{
			continue;
		}

		// �A�j���[�V�����̌v�Z�́A�w�肳�ꂽ�t���[���̂Ƃ���
		// �ǂ�ȃ|�[�Y���Ƃ邩���v�Z����B�|�[�Y�ɂ͈ړ��A��]�A�g�k�̃f�[�^��
		// �܂܂�邽�߁Ax,y,z�R�̐����ƍ��킹�A�X���̃p�����[�^���v�Z����B
		
		// �ϐ������̂܂܂��ƁA�܂Ƃ߂��Ȃ��̂ŁAfloat�^�̃A�h���X�Ƃ��Ă܂Ƃ߂�
		MatrixParam* pBone = &pBones[boneIndex]; // �������ݐ�
		float* value[] = {
			&pBone->translate.x, &pBone->translate.y, &pBone->translate.z,
			&pBone->rotation.x,&pBone->rotation.y,&pBone->rotation.z,
			&pBone->scale.x,&pBone->scale.y,&pBone->scale.z,
		};

		// �p�����[�^�̌v�Z
		for (int j = 0; j < 9; ++j)
		{
			*value[j] = CalcKey(pAnime->curveList[i].keyList[j],
				m_pAnimes[animeNo]->playFrame, *value[j]);
		}

	}

	//--------------
}

// �A�j���[�V�����̃t���[���ɑΉ������p�����v�Z
float FBXPlayer::CalcKey(ggfbx::AnimationInfo::CurveInfo::List& keys, float frame, float defValue)
{
	//--------------
	// �A�j���[�V�����̃L�[�v�Z
	//--------------

	// �g�k��ړ��̓L�[���Ȃ����Ƃ�����̂ŁA��O�������s��
	if (keys.size() == 0)
	{
		return defValue;
	}
	// �L�[��������ł͕�Ԃł��Ȃ��̂ŁA���̂܂܎g�p
	else if (keys.size() == 1)
	{
		return keys[0].value;
	}
	// ��ȏ�ŕ��
	else
	{
		// ��̃t���[���̊Ԃ�⊮����, (i)��(i-1)�̃t���[���𒲂ׂ邽�߁A
		// �����l�͂P����n�߂�i�O���Ɣz��O�ɃA�N�Z�X����
		for (int i = 1; i < keys.size(); i++)
		{
			// �L�[�̊J�n���ԂƏI�����Ԃ̊Ԃɋ��܂�Ă��邩
			float startFrame = keys[i - 1].frame;
			float endFrame = keys[i].frame;
			if (startFrame <= frame && frame < endFrame)/*�����̃t���[�����L�[�̊Ԃɋ��܂�Ă��邩����*/
			{
				// �L�[�̒l
				float startValue = keys[i - 1].value;
				float endValue = keys[i].value;

				// �䗦 = (���ݒn - �J�n�l) / (�ڕW�l - �J�n�l)
				float rate = (frame - startFrame) / (endFrame - startFrame);/*�䗦���v�Z*/

				// ���`��Ԃ̎� = (�ڕW�l - �J�n�l) * �䗦 + �J�n�l
				return (endValue - startValue) * rate + startValue;/*��Ԃ�������*/
			}
		}

		// ��ԍŌ�̃L�[�����x���t���[���̏ꍇ�A
		// �Ō�̃t���[���̒l�����̂܂܎g�p
		return keys[keys.size() - 1].value;
	}



	//--------------
	return 0.0f;
}