#ifndef EDITRANGEH
#define EDITRANGEH

#include <list>
#include <orgwindow.h>

class CEditRange
{
public:

/**
 * @fn
 * CEditRange
 * @breaf �R���X�g���N�^
 * @return �Ȃ��B
 */
	CEditRange();

/**
 * @fn
 * ~CEditRange
 * @breaf �f�X�g���N�^
 * @return �Ȃ��B
 */
	~CEditRange();

/**
 * @fn
 * Clear
 * @breaf �����o�����Z�b�g����B
 * @return ����������O�B
 */
	int Clear();

/**
 * @fn
 * SetRange
 * @breaf �ҏW�͈͂��Z�b�g����B
 * @param (std::list<KeyInfo> srcki) IN �ҏW�����L�[���L�^���ꂽ���X�g�B
 * @param (double srcframe) IN �P�t���[���݂̂̕ҏW���ɂ͂��̃t���[���B
 * @return ����������O�B
 */
	int SetRange( std::list<KeyInfo> srcki, double srcframe );


/**
 * @fn
 * GetRange
 * @breaf �ҏW�͈͏����擾����B
 * @param (int* numptr) OUT �ҏW�͈͂̃L�[�̐��B
 * @param (double* startptr) OUT�@�ҏW�͈͂̊J�n�t���[���B
 * @param (double* endptr) OUT �ҏW�͈͂̏I���t���[���B
 * @return ����������O�B
 */
	int GetRange( int* numptr, double* startptr, double* endptr );

/**
 * @fn
 * GetRange
 * @breaf �ҏW�͈͏����擾����B
 * @param (int* numptr) OUT �ҏW�͈͂̃L�[�̐��B
 * @param (double* startptr) OUT�@�ҏW�͈͂̊J�n�t���[���B
 * @param (double* endptr) OUT �ҏW�͈͂̏I���t���[���B
 * @param (double* applyptr) OUT �p���K�p�t���[���B
 * @return ����������O�B
 * @detail �I��͈͂̊J�n�ʒu���牽���̈ʒu�Ɏp����K�p���邩�̏�񂩂�A�p���K�p�t���[���̌v�Z���s���B
 */
	int GetRange( int* numptr, double* startptr, double* endptr, double* applyptr );


	CEditRange CEditRange::operator= (CEditRange srcrange);
	bool CEditRange::operator== (const CEditRange &er) const { return ((m_startframe == er.m_startframe) && (m_endframe == er.m_endframe) && (m_applyframe == er.m_applyframe)); };
	bool CEditRange::operator!= (const CEditRange &er) const { return !(*this == er); };

private:

/**
* @fn
* InitParams
* @breaf �����o�̏������B
* @return �O�B
*/
	int InitParams();

/**
 * @fn
 * DestroyObjs
 * @breaf �A���P�[�g�������������J������B
 * @return �O�B
 */
	int DestroyObjs();


public:
	int m_setflag;
	int m_setcnt;
	std::list<KeyInfo> m_ki;//�ҏW�͈͂̃L�[�̏��
	int m_keynum;
	double m_startframe;
	double m_endframe;
	double m_applyframe;//�p���K�p�t���[���B

	static double s_applyrate;//�p���K�p�t���[�����I��͈͂̊J�n�ʒu���牽���̂Ƃ���ɂ��邩�B
};

#endif



