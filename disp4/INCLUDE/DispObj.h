#ifndef DISPOBJH
#define DISPOBJH

#include <D3DX9.h>

class CMQOMaterial;
class CPolyMesh3;
class CPolyMesh4;
class CExtLine;

class CDispObj
{
public:

/**
 * @fn
 * CDispObj
 * @breaf コンストラクタ。
 * @return なし。
 */
	CDispObj();

/**
 * @fn
 * ~CDispObj
 * @breaf デストラクタ。
 * @return なし。
 */
	~CDispObj();

/**
 * @fn
 * CreateDispObj
 * @breaf メタセコイアからのデータ用の表示用オブジェクトを作成する。
 * @param (LPDIRECT3DDEVICE9 pdev) IN Direct3Dのデバイス。
 * @param (CPolyMesh3* pm3) IN 形状データへのポインタ。
 * @param (int hasbone) IN ボーン変形有の場合は１、無しの場合は０。MameBake3Dではメタセコイアデータはボーン無し。
 * @return 成功したら０。
 */
	int CreateDispObj( LPDIRECT3DDEVICE9 pdev, CPolyMesh3* pm3, int hasbone );


/**
 * @fn
 * CreateDispObj
 * @breaf FBXからのデータ用の表示用オブジェクトを作成する。
 * @param (LPDIRECT3DDEVICE9 pdev) IN Direct3Dのデバイス。
 * @param (CPolyMesh4* pm4) IN 形状データへのポインタ。
 * @param (int hasbone) IN ボーン変形有の場合は１、無しの場合は０。 * @return 戻り値の説明
 * @return 成功したら０。
 */
	int CreateDispObj( LPDIRECT3DDEVICE9 pdev, CPolyMesh4* pm4, int hasbone );

/**
 * @fn
 * CreateDispObj
 * @breaf 線分データ用の表示用オブジェクトを作成する。
 * @param (LPDIRECT3DDEVICE9 pdev) IN Direct3Dのデバイス。
 * @param (CExtLine* extline) IN 線の形状データへのポインタ。
 * @return 成功したら０。
 */
	int CreateDispObj( LPDIRECT3DDEVICE9 pdev, CExtLine* extline );


/**
 * @fn
 * RenderNormal
 * @breaf 通常のベーシックな方法で３D描画をする。
 * @param (CMQOMaterial* rmaterial) IN 材質へのポインタ。
 * @param (int lightflag) IN 照光処理するときは１、しないときは０。
 * @param (D3DXVECTOR4 diffusemult) IN ディフューズ色に掛け算する比率。
 * @return 成功したら０。
 */
	int RenderNormal( CMQOMaterial* rmaterial, int lightflag, D3DXVECTOR4 diffusemult );

/**
 * @fn
 * RenderNormalPM3
 * @breaf 通常のベーシックな方法でメタセコイアデータを３D表示する。
 * @param (int lightflag) IN 照光処理するときは１、しないときは０。
 * @param (D3DXVECTOR4 diffusemult) IN ディフューズ色に掛け算する比率。
 * @return 成功したら０。
 * @detail FBXデータは１オブジェクトにつき１マテリアル(材質)だが、メタセコイアデータは１オブジェクトに複数マテリアルが設定されていることが多い。
 */
	int RenderNormalPM3( int lightflag, D3DXVECTOR4 diffusemult );

/**
 * @fn
 * RenderLine
 * @breaf 線分データを表示する。
 * @param (D3DXVECTOR4 diffusemult) IN ディフューズ色に掛け算する比率。
 * @return 成功したら０。
 */
	int RenderLine( D3DXVECTOR4 diffusemult );

/**
 * @fn
 * CopyDispV
 * @breaf FBXの頂点データを表示バッファにコピーする。
 * @param (CPolyMesh4* pm4) IN FBXの形状データ。
 * @return 成功したら０。
 */
	int CopyDispV( CPolyMesh4* pm4 );

/**
 * @fn
 * CopyDispV
 * @breaf メタセコイアの頂点データを表示バッファにコピーする。
 * @param (CPolyMesh3* pm3) IN メタセコイアの形状データ。
 * @param (引数名) 引数の説明
 * @return 成功したら０。
 */
	int CopyDispV( CPolyMesh3* pm3 );

private:

/**
 * @fn
 * InitParams
 * @breaf メンバを初期化する。
 * @return ０。
 */
	int InitParams();

/**
 * @fn
 * DestroyObjs
 * @breaf アロケートしたメモリを開放する。
 * @return ０。
 */
	int DestroyObjs();

/**
 * @fn
 * CreateDecl
 * @breaf 表示用頂点データのフォーマット(宣言)を作成する。
 * @return 成功したら０。
 */
	int CreateDecl();

/**
 * @fn
 * CreateVBandIB
 * @breaf ３D表示用の頂点バッファとインデックスバッファを作成する。
 * @return 成功したら０。
 */
	int CreateVBandIB();

/**
 * @fn
 * CreateVBandIBLine
 * @breaf 線分用の頂点バッファとインデックスバッファを作成する。
 * @return 成功したら０。
 */
	int CreateVBandIBLine();

private:
	int m_hasbone;//ボーン変形用のオブジェクトであるとき１、それ以外の時は０。

	LPDIRECT3DDEVICE9 m_pdev;//外部メモリ、Direct3Dのデバイス。
	CPolyMesh3* m_pm3;//外部メモリ、メタセコイアファイルから作成した３Dデータ。
	CPolyMesh4* m_pm4;//外部メモリ、FBXファイルから作成した３Dデータ。
	CExtLine* m_extline;//外部メモリ、線データ。

	IDirect3DVertexDeclaration9* m_declbone;//ボーン変形有３Dデータのフォーマット。
	IDirect3DVertexDeclaration9* m_declnobone;//ボーン変形無し３Dデータのフォーマット。
	IDirect3DVertexDeclaration9* m_declline;//線データのフォーマット。

    LPDIRECT3DVERTEXBUFFER9 m_VB;//表示用頂点バッファ。
	LPDIRECT3DVERTEXBUFFER9 m_InfB;//表示用ボーン影響度バッファ。
	LPDIRECT3DINDEXBUFFER9 m_IB;//表示用三角のインデックスバッファ。
};



#endif