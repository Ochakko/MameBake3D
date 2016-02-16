#ifndef BVHELEMH
#define BVHELEMH

#include <coef.h>
#include <D3DX9.h>
#include <fbxsdk.h>

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

	//D3DXVECTOR3* GetRotate(){
	//	return rotate;
	//};

	float GetRotate(int srcframeno, int srcaxis)
	{
		if ((srcframeno >= 0) && (srcframeno < framenum)){
			if ((srcaxis >= ROTAXIS_X) && (srcaxis <= ROTAXIS_Z)){
				switch (srcaxis){
				case ROTAXIS_X:
					return (rotate + srcframeno)->x;
					break;
				case ROTAXIS_Y:
					return (rotate + srcframeno)->y;
					break;
				case ROTAXIS_Z:
					return (rotate + srcframeno)->z;
					break;
				default:
					_ASSERT(0);
					return 0.0f;
					break;
				}
			}
			else{
				_ASSERT(0);
				return 0.0f;
			}
		}
		else{
			_ASSERT(0);
			return 0.0f;
		}
	};

	int GetTrans(int srcframeno, D3DXVECTOR3* dsttra)
	{
		if ((srcframeno >= 0) && (srcframeno < framenum)){
			*dsttra = *(trans + srcframeno);
		}
		else{
			return -1;
		}
		return 0;
	}

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

	EFbxRotationOrder GetFBXRotationOrder(){
		if (rotorder[0] == ROTAXIS_X){
			if (rotorder[1] == ROTAXIS_Y){
				//return eEulerXYZ;
				return eEulerZYX;
			}
			else if (rotorder[1] == ROTAXIS_Z){
				//return eEulerXZY;
				return eEulerYZX;
			}
			else{
				_ASSERT(0);
				return eEulerZXY;
			}
		}
		else if (rotorder[0] == ROTAXIS_Y){
			if (rotorder[1] == ROTAXIS_X){
				//return eEulerYXZ;
				return eEulerZXY;
			}
			else if (rotorder[1] == ROTAXIS_Z){
				//return eEulerYZX;
				return eEulerXZY;
			}
			else{
				_ASSERT(0);
				return eEulerZXY;
			}
		}
		else if (rotorder[0] == ROTAXIS_Z){
			if (rotorder[1] == ROTAXIS_X){
				//return eEulerZXY;
				return eEulerYXZ;
			}
			else if (rotorder[1] == ROTAXIS_Y){
				//return eEulerZYX;
				return eEulerXYZ;
			}
			else{
				_ASSERT(0);
				return eEulerZXY;
			}
		}
		else{
			_ASSERT(0);
			return eEulerZXY;
		}
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