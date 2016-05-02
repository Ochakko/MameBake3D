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
 * @breaf コンストラクタ
 * @return なし。
 */
	CEditRange();

/**
 * @fn
 * ~CEditRange
 * @breaf デストラクタ
 * @return なし。
 */
	~CEditRange();

/**
 * @fn
 * Clear
 * @breaf メンバをリセットする。
 * @return 成功したら０。
 */
	int Clear();

/**
 * @fn
 * SetRange
 * @breaf 編集範囲をセットする。
 * @param (std::list<KeyInfo> srcki) IN 編集したキーが記録されたリスト。
 * @param (double srcframe) IN １フレームのみの編集時にはそのフレーム。
 * @return 成功したら０。
 */
	int SetRange( std::list<KeyInfo> srcki, double srcframe );


/**
 * @fn
 * GetRange
 * @breaf 編集範囲情報を取得する。
 * @param (int* numptr) OUT 編集範囲のキーの数。
 * @param (double* startptr) OUT　編集範囲の開始フレーム。
 * @param (double* endptr) OUT 編集範囲の終了フレーム。
 * @return 成功したら０。
 */
	int GetRange( int* numptr, double* startptr, double* endptr );

/**
 * @fn
 * GetRange
 * @breaf 編集範囲情報を取得する。
 * @param (int* numptr) OUT 編集範囲のキーの数。
 * @param (double* startptr) OUT　編集範囲の開始フレーム。
 * @param (double* endptr) OUT 編集範囲の終了フレーム。
 * @param (double* applyptr) OUT 姿勢適用フレーム。
 * @return 成功したら０。
 * @detail 選択範囲の開始位置から何％の位置に姿勢を適用するかの情報から、姿勢適用フレームの計算も行う。
 */
	int GetRange( int* numptr, double* startptr, double* endptr, double* applyptr );


	CEditRange CEditRange::operator= (CEditRange srcrange);
	bool CEditRange::operator== (const CEditRange &er) const { return ((m_startframe == er.m_startframe) && (m_endframe == er.m_endframe) && (m_applyframe == er.m_applyframe)); };
	bool CEditRange::operator!= (const CEditRange &er) const { return !(*this == er); };

private:

/**
* @fn
* InitParams
* @breaf メンバの初期化。
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


public:
	int m_setflag;
	int m_setcnt;
	std::list<KeyInfo> m_ki;//編集範囲のキーの情報
	int m_keynum;
	double m_startframe;
	double m_endframe;
	double m_applyframe;//姿勢適用フレーム。

	static double s_applyrate;//姿勢適用フレームが選択範囲の開始位置から何％のところにあるか。
};

#endif



