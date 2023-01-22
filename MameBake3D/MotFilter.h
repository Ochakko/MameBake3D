#ifndef MOTFILTERH
#define MOTFILTERH


class CMotFilter
{
public:
	CMotFilter();
	~CMotFilter();

	int Filter(CModel* srcmodel, CBone* srcbone, int srcopekind, int srcmotid, int srcstartframe, int srcendframe);

private:
	void InitParams();
	void DestroyObjs();

	int GetFilterType();
	int Combi(int N, int rp);

	void FilterReq(CModel* srcmodel, CBone* curbone, int srcmotid, int srcstartframe, int srcendframe);
	int FilterFunc(CModel* srcmodel, CBone* curbone, int srcmotid, int srcstartframe, int srcendframe);

private:
	int m_filtertype;
	int m_filtersize;

	ChaVector3* m_eul;
	ChaVector3* m_smootheul;
	ChaVector3* m_tra;
	ChaVector3* m_smoothtra;

};


#endif
