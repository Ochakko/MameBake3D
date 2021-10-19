#ifndef RETARGETH
#define RETARGETH

class CModel;
class CBone;

#include <map>
#include <ChaVecCalc.h>

namespace MameBake3DLibRetarget {

#ifdef RETARGETCPP

	int Retarget(CModel* srcmodel, CModel* srcbvhmodel, ChaMatrix smatVP, std::map<CBone*, CBone*>& sconvbonemap, int (*srcAddMotionFunc)(WCHAR* wfilename, double srcmotleng), int (*srcInitCurMotionFunc)(int selectflag, double expandmotion));

#else

	extern int Retarget(CModel* srcmodel, CModel* srcbvhmodel, ChaMatrix smatVP, std::map<CBone*, CBone*>& sconvbonemap, int (*srcAddMotionFunc)(WCHAR* wfilename, double srcmotleng), int (*srcInitCurMotionFunc)(int selectflag, double expandmotion));

#endif

}

#endif#pragma once
