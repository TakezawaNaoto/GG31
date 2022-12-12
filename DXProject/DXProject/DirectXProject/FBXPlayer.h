#ifndef __FBX_PLAYER_H__
#define __FBX_PLAYER_H__

#include "FBXLoader.h"
#include <DirectXMath.h>
#include "DXBuffer.h"

class FBXPlayer
{
public:
	struct FBXVertex
	{
		DirectX::XMFLOAT3 pos;
		DirectX::XMFLOAT4 color;
		DirectX::XMFLOAT2 uv;
		DirectX::XMFLOAT3 normal;
		// メッシュとボーンを結びつけるためのデータ
		float weight[4]; // それぞれの骨からどの程度影響を受けるか
		long index[4]; // どの骨に引っ付いて動くか
	};
	/* スキンメッシュアニメーション
	骨を動かした結果をモデルの頂点に適用する
	そのまま適用してしまうと、骨が原点から移動してきた分、表示しようと
	している頂点も移動してしまう。この問題を解決するには一度頂点を
	原点に移動させてから骨の移動を適用する必要がある。
	その際、FBXモデルは一つのメッシュ(例えば頭だけ)をアニメーションさせる
	時、影響を受ける必要最低限の骨の情報しか持たない(頭を動かす時に
	足の骨の情報はいらない)
	そのため、各メッシュごとにどの骨の影響を受けるのかをまとめた情報が
	必要になる
	*/
	struct MeshInverse
	{
		struct List
		{
			int boneIndex;
			// もともとのモデルはTポーズを取っている
			// そのTポーズの移動情報を打ち消し、新たにアニメーションの
			// 行列を適用することで、頂点が正しいアニメーションの位置に
			// 移動する
			DirectX::XMMATRIX invMat; // Tポーズの移動を打ち消す逆行列
		};
		List* pList;
		int num;
	};

	struct MatrixParam
	{
		DirectX::XMFLOAT3 translate;
		DirectX::XMFLOAT3 rotation;
		DirectX::XMFLOAT3 scale;
	};
	struct BoneInfo
	{
		DirectX::XMFLOAT4X4 matrix; // 骨の姿勢
		DirectX::XMMATRIX preRotate; // あらかじめ回転している量
		MatrixParam param; // デフォルトの骨の姿勢
	};
	struct AnimeInfo
	{
		float playFrame; // 現在再生中のフレーム
		bool loop; // 繰り返し再生するかどうか
		float speed; // アニメーションの再生スピード
		ggfbx::AnimationInfo* pAnime; // アニメの情報
	};

public:
	FBXPlayer();
	~FBXPlayer();

	bool Load(const char* fileName);
	// 戻り値で配列のどの位置にアニメーションの情報を格納したか返す
	// 読み込めなかった時は-1を返す
	int LoadAnime(const char* fileName);

	// アニメーションの再生指定
	void PlayAnime(int animeNo, bool isLoop = false);
	// 遷移ｱﾆﾒｰｼｮﾝの指定
	void PlayBlendAnime(int blendNo, float time, bool isLoop = false);
	// アニメーション再生判定
	bool IsPlayAnime(int animeNo = -1);

	// ぶちゃっけUpdate
	// 映像と同じイメージで早送り、コマ送りなどを考えたときに、
	// １フレームでの処理数が一定ではないので、現在の再生ｽﾋﾟｰﾄﾞで
	// どれだけ処理するかで考えた→結果stepという関数名
	void Step(float speed = 1.0f);


	/*関数ポインタ
	   実行したい関数がどこに配置されているかを示すポインタ
	   変数のポインタと用途は変わらない
	   関数名の前に*を付けて()で囲うと宣言できる
	   戻り値、引数を呼び出したい関数と揃えないといけない
	 描画ごとに別々の処理を呼び出したいので、引数に関数ポインタを設定 
	*/
	void Draw(void(*PreCallBack)(int mesh) = nullptr);
	// デバッグ用
// #ifdef _DEBUG
	void DrawBone();
// #endif

	DirectX::XMFLOAT4X4 GetBone(int index);
	std::vector<MeshInverse>& GetMeshInverse();

private:
	// 全てのメモリを安全に解放するための関数
	void Reset();
	// モデルの骨がどんなポーズを取っているか計算する関数
	void CalcBone(int index, DirectX::XMMATRIX parent);
	// 骨を描画する関数
	void CalcDrawBone(int index, DirectX::XMFLOAT3 parent);
	// アニメーションの時間経過を計算
	void CalcAnimeFrame(int animeNo, float speed);
	// アニメーションのフレームに応じて骨のポーズを計算
	void CalcAnime(int animeNo, MatrixParam* pBones);
	// アニメーションのフレームに対応した姿勢を計算
	float CalcKey(ggfbx::AnimationInfo::CurveInfo::List& keys, float frame, float defValue);

private:
	// モデルの表示
	DXBuffer* m_pBuffers;
	int m_meshNum;
	std::vector<MeshInverse> m_meshInverse;


	//--- 骨の情報
	// 骨一本一本の位置や回転の情報、
	// どの骨がどこに繋がってるか、骨の名称が含まれる
	// また、どこから計算を始めるのかといった情報を含む
	ggfbx::BoneListInfo* m_pFBXBone; // FBXの骨情報
	BoneInfo* m_pBones; // プログラム実行中の骨の情報
	const static int MaxBlend = 2;
	MatrixParam* m_pBlendBones[MaxBlend]; // 遷移用の骨情報
	//--- アニメーションの情報
	const static int MaxAnime = 10;
	AnimeInfo* m_pAnimes[MaxAnime];
	int m_playAnimeNo; // 現在再生中のアニメ番号

	int m_blendAnimeNo; // 遷移中のアニメ番号
	float m_blendFrame; // 遷移時間
};

#endif
