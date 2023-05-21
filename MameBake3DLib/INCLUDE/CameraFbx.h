#ifndef CAMERAFBXH
#define CAMERAFBXH

#include <map>
#include <string>

#include <Coef.h>
#include <ChaVecCalc.h>
#include <MotionPoint.h>


#include <fbxsdk.h>



class CCameraFbx
{
public:
	CCameraFbx();
	~CCameraFbx();
	int Clear();
	int SetFbxCamera(FbxNode* pnode);


	CCameraFbx operator= (CCameraFbx srcrange);
	bool operator== (const CCameraFbx &cmp) const {
		return (
			(m_position == cmp.m_position) &&
			(m_dirvec == cmp.m_dirvec) &&
			(m_upVector == cmp.m_upVector) && 
			(m_aspectHeight == cmp.m_aspectHeight) && 
			(m_aspectWidth == cmp.m_aspectWidth) &&
			(m_nearZ == cmp.m_nearZ) &&
			(m_farZ == cmp.m_farZ) &&
			(m_aspectRatio == cmp.m_aspectRatio) &&
			//(m_inch_mm == cmp.m_inch_mm) &&
			(m_filmHeight == cmp.m_filmHeight) &&
			(m_focalLength == cmp.m_focalLength) &&
			//(m_filmHeight_mm == cmp.m_filmHeight_mm) &&
			(m_fovY == cmp.m_fovY)// && 
			//(m_fovY_Degree == cmp.m_fovY_Degree)
			); 
	};
	bool operator!= (const CCameraFbx &cmp) const { 
		return !(*this == cmp); 
	};


private:
	int InitParams();
	int DestroyObjs();

public:
	bool IsLoaded()
	{
		return m_loadedflag;
	}
	ChaMatrix GetWorldMat()
	{
		return m_worldmat;
	}


	ChaVector3 GetPosition()
	{
		return m_position;
	}
	ChaVector3 GetTargetPos()
	{
		_ASSERT(0);//���݁@���ݒ�
		return m_targetpos;
	}
	ChaVector3 GetDirVec()
	{
		return m_dirvec;
	}

	ChaVector3 GetUpVector()
	{
		return m_upVector;
	}
	double GetAspectHeight()
	{
		return m_aspectHeight; // �A�X�y�N�g��
	}
	double GetAspectWidth()
	{
		return m_aspectWidth;  // �A�X�y�N�g��
	}
	double GetNearPlane()
	{
		return m_nearZ;     // near���ʋ���
	}
	double GetFarPlane()
	{
		return m_farZ;      // far���ʋ���
	}
	double GetAspectRatio()
	{
		return m_aspectRatio; // �A�X�y�N�g��
	}
	double GetFilmHeight()
	{
		return m_filmHeight;   // �t�B�������i�C���`�j
	}
	double GetFocalLength()
	{
		return m_focalLength; // ���ŋ����i�~���j
	}
	double GetFilmHeightMM()
	{
		return m_filmHeight_mm;
	}
	double GetFovY()
	{
		return m_fovY;
	}
	double GetFovYDegree()
	{
		return m_fovY_Degree;
	}


private:
	FbxNode* m_pnode;
	FbxCamera* m_pcamera;
	ChaMatrix m_worldmat;

	bool m_loadedflag;

	ChaVector3 m_position;
	ChaVector3 m_targetpos;
	ChaVector3 m_dirvec;

	ChaVector3 m_upVector;     // �A�b�v�x�N�g��
	double m_aspectHeight; // �A�X�y�N�g��
	double m_aspectWidth;  // �A�X�y�N�g��
	double m_nearZ;     // near���ʋ���
	double m_farZ;      // far���ʋ���
	double m_aspectRatio; // �A�X�y�N�g��
	double m_inch_mm;    // �C���`���~��
	double m_filmHeight;   // �t�B�������i�C���`�j
	double m_focalLength; // ���ŋ����i�~���j
	double m_filmHeight_mm;
	double m_fovY;
	double m_fovY_Degree;

};

#endif



