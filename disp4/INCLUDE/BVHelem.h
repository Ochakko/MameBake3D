#ifndef BVHELEMH
#define BVHELEMH

#include <coef.h>
#include <D3DX9.h>

class CQuaternion;

class CBVHElem
{
public:
	CBVHElem();
	~CBVHElem();

	int SetName( char* srcname );
	int SetIsRoot( int srcisroot );
	int SetOffset( char* srcchar );
	int SetChanels( char* srcchar );

	int CreateMotionObjs( int srcframes );

	int SetMotionParams( int srcframeno, float* srcfloat );

	int DbgOutBVHElem( int srcdepth, int outmotionflag );
	int SetPosition();

	int Mult( float srcmult );
	int ConvertRotate2Q();
	int CheckNotAlNumName( char** ppdstname );

	int CalcDiffTra( int frameno, D3DXVECTOR3* pdifftra );

private:
	int InitParams();
	int DestroyObjs();

	char* GetDigit( char* srcstr, int* dstint, int* dstsetflag );
	char* GetFloat( char* srcstr, float* dstfloat, int* dstsetflag );
	char* GetChanelType( char* srcstr, int* dstint, int* dstsetflag );

public:
	//accesser
	CBVHElem* GetParent(){
		return parent;
	};
	void SetParent( CBVHElem* srcval ){
		parent = srcval;
	};

	CBVHElem* GetChild(){
		return child;
	};
	void SetChild( CBVHElem* srcval ){
		child = srcval;
	};

	CBVHElem* GetBrother(){
		return brother;
	};
	void SetBrother( CBVHElem* srcval ){
		brother = srcval;
	};

	int GetChanelNum(){
		return chanelnum;
	};

	CQuaternion* GetTreeQ(){
		return treeq;
	};

	CQuaternion* GetQPtr(){
		return qptr;
	};

	int GetSameNameBoneSeri(){
		return samenameboneseri;
	};

	D3DXVECTOR3 GetPosition(){
		return position;
	};

	int GetFrameNum(){
		return framenum;
	}

	D3DXVECTOR3* GetRotate(){
		return rotate;
	};

	const char* GetName(){
		return name;
	};

	int GetBoneNum(){
		return bonenum;
	};
	int GetBroNo(){
		return brono;
	};
	int SetBoneNum( int srcbonenum ){
		bonenum = srcbonenum;
		return 0;
	};
	int SetBroNo( int srcbrono ){
		brono = srcbrono;
		return 0;
	};

private:
	int isroot;

	int describeno;
	int serialno;
	int mqono;

	char name[ PATH_LENG ];
	D3DXVECTOR3 offset;
	int chanelnum;
	int chanels[ CHANEL_MAX ];

	int framenum;
	float frametime;

	D3DXVECTOR3* trans;
	D3DXVECTOR3* rotate;
	CQuaternion* qptr;
	CQuaternion* transpose;//転置後のクォータニオン
	CQuaternion* treeq;//親の影響を考慮したクォータニオン

	D3DXVECTOR3* partransptr;
	D3DXVECTOR3 position;

	int samenameboneseri;//sigに、同じ名前のボーンがあった場合にselem->serialnoをセット、無かったら-1。

	int rotordercnt;
	int rotorder[ROTAXIS_MAX];

	CBVHElem* parent;
	CBVHElem* child;
	CBVHElem* brother;

	int bonenum;
	int brono;

};

#endif