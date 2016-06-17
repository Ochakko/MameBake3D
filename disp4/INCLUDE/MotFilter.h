#ifndef MOTFILTERH
#define MOTFILTERH


class CMotFilter
{
public:
	CMotFilter();
	~CMotFilter();

	int Filter(CModel* srcmodel, int srcmotid, int srcstartframe, int srcendframe);

private:
	void InitParams();
	void DestroyObjs();

	int GetFilterType();
	int Combi(int N, int rp);

	void FilterReq(CModel* srcmodel, CBone* curbone, int srcmotid, int srcstartframe, int srcendframe);

private:
	int m_filtertype;
	int m_filtersize;

	D3DXVECTOR3* m_eul;
	D3DXVECTOR3* m_smootheul;
	D3DXVECTOR3* m_tra;
	D3DXVECTOR3* m_smoothtra;

};


#endif
