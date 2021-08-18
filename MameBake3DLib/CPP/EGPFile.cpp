#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <malloc.h>
#include <memory.h>

#include <windows.h>

#define EGPFILECPP
#include <EGPFile.h>
//#include <fbxfilesdk/kfbxtransformation.h>

#define	DBGH
#include <dbg.h>

#include <crtdbg.h>
#include <coef.h>

#include <FBXBone.h>
#include <Model.h>
#include <MQOObject.h>
#include <Bone.h>
#include <PolyMesh4.h>
#include <MQOMaterial.h>
#include <MQOFace.h>
#include <InfBone.h>
#include <MotionPoint.h>
//#include <MorphKey.h>

#include <bvhelem.h>
#include <bvhfile.h>


#include <map>
using namespace std;

typedef struct tag_animinfo
{
	int motid;
	int orgindex;
	int maxframe;
	char* engmotname;
	FbxAnimLayer* animlayer;
}ANIMINFO;




typedef struct tag_egpheader
{
	char magicstr[32];//EvaluateGlobalPosition
	char version[16];
	char fbxdate[256];
	int animno;
	int jointnum;
	int framenum;
	int reserved;
}EGPHEADER;

typedef struct tag_egpjointheader
{
	int jointindex;
	int framenum;
	char jointname[256];
}EGPJOINTHEADER;

typedef struct tag_egpelem
{
	int jointindex;
	int frameno;
	FbxAMatrix egp;
}EGPELEM;

//about 800MB
#define EGPBUFLENG 800000000

static CModel* s_model = 0;

static int WriteEGPFileHeader(EGPHEADER* dstegph, HANDLE file, CModel* srcmodel, char* fbxdate, int srcanimno);
static bool ValidateEGPFile(EGPHEADER* dstegph, char* srcbuf, DWORD bufleng, CModel* pmodel, char* fbxdate, int srcanimno);

int CreateEGPBuf()
{
	g_egpbuf = (char*)malloc(sizeof(char) * (unsigned int)EGPBUFLENG);
	if (!g_egpbuf) {
		return 1;
	}

	return 0;
}
void DestroyEGPBuf()
{
	if (g_egpbuf) {
		free(g_egpbuf);
		g_egpbuf = 0;
	}
}



int WriteEGPFile(CModel* pmodel, WCHAR* pfilename, char* fbxdate, int animno)
{
	s_model = pmodel;


	CallF( pmodel->MakeEnglishName(), return 1 );

	WCHAR outfilename[MAX_PATH] = { 0L };
	swprintf_s(outfilename, MAX_PATH, L"%s_anim%d.egp", pfilename, animno);


	HANDLE hfile = CreateFile(outfilename, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_ALWAYS,
		FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (hfile == INVALID_HANDLE_VALUE) {
		DbgOut(L"EGPFile : WriteEGPFile : file open error !!!\n");
		_ASSERT(0);
		return 1;
	}


	EGPHEADER egpheader;
	ZeroMemory(&egpheader, sizeof(EGPHEADER));

	int result;
	result = WriteEGPFileHeader(&egpheader, hfile, pmodel, fbxdate, animno);
	if (result != 0) {
		_ASSERT(0);
		return 1;
	}
	
	int jointindex;
	int frameno;
	for (jointindex = 0; jointindex < egpheader.jointnum; jointindex++) {
		CBone* curbone = pmodel->GetBoneByZeroBaseIndex(jointindex);
		if (!curbone) {
			_ASSERT(0);
			return 1;
		}

		//typedef struct tag_egpjointheader
		//{
		//	int jointindex;
		//	int framenum;
		//	char jointname[256];
		//}EGPJOINTHEADER;

		EGPJOINTHEADER jointheader;
		ZeroMemory(&jointheader, sizeof(EGPJOINTHEADER));
		jointheader.jointindex = jointindex;
		jointheader.framenum = egpheader.framenum;
		strcpy_s(jointheader.jointname, 256, curbone->GetBoneName());
		DWORD writesize1 = 0;
		WriteFile(hfile, &jointheader, sizeof(EGPJOINTHEADER), &writesize1, NULL);
		if (writesize1 != sizeof(EGPJOINTHEADER)) {
			_ASSERT(0);
			return 1;
		}

		for (frameno = 0; frameno < egpheader.framenum; frameno++) {
			FbxAMatrix curegp;
			curegp = curbone->GetlClusterGlobalCurrentPosition(frameno);

			//typedef struct tag_egpelem
			//{
			//	int jointindex;
			//	int frameno;
			//	ChaMatrix egp;
			//}EGPELEM;

			EGPELEM egpelem;
			ZeroMemory(&egpelem, sizeof(EGPELEM));
			egpelem.jointindex = jointindex;
			egpelem.frameno = frameno;
			egpelem.egp = curegp;
			DWORD writesize2 = 0;
			WriteFile(hfile, &egpelem, sizeof(EGPELEM), &writesize2, NULL);
			if (writesize2 != sizeof(EGPELEM)) {
				_ASSERT(0);
				return 1;
			}
		}
	}

	CloseHandle(hfile);


	return 0;
}

int WriteEGPFileHeader(EGPHEADER* dstegph, HANDLE hfile, CModel* srcmodel, char* fbxdate, int srcanimno)
{
	if ((hfile == INVALID_HANDLE_VALUE) || !srcmodel || !fbxdate || !dstegph) {
		return 1;	
	}

	int animnum;
	animnum = srcmodel->GetMotInfoSize();
	if ((srcanimno < 0) || (srcanimno >= animnum)) {
		return 1;
	}

	int curmotid;
	curmotid = srcanimno + 1;
	MOTINFO* curmi;
	curmi = srcmodel->GetMotInfo(curmotid);
	if (!curmi) {
		return 1;
	}
	int framenum;
	framenum = curmi->frameleng;
	if (framenum <= 0) {
		return 1;
	}


	int jointnum;
	jointnum = srcmodel->GetBoneListSize();
	if (jointnum <= 0) {
		return 1;
	}

	//typedef struct tag_egpheader
	//{
	//	char magicstr[32];//EvaluateGlobalPosition
	//	char version[16];
	//	char fbxdate[256];
	//	int animno;
	//	int jointnum;
	//	int framenum;
	//	int reserved;
	//}EGPHEADER;


	ZeroMemory(dstegph, sizeof(EGPHEADER));

	strcpy_s(dstegph->magicstr, 32, "EvaluateGlobalPosition");
	strcpy_s(dstegph->version, 16, "0.0.0.1");
	strcpy_s(dstegph->fbxdate, 256, fbxdate);
	dstegph->animno = srcanimno;
	dstegph->jointnum = jointnum;
	dstegph->framenum = framenum;
	dstegph->reserved = 0;

	DWORD wleng = 0;
	WriteFile(hfile, dstegph, sizeof(EGPHEADER), &wleng, NULL);
	if (wleng != sizeof(EGPHEADER)) {
		return 1;
	}

	return 0;
}



bool ValidateEGPFile(EGPHEADER* dstegph, char* srcbuf, DWORD bufleng, CModel* pmodel, char* fbxdate, int srcanimno)
{
	if (!dstegph || !srcbuf || (bufleng <= 0) || !pmodel) {
		return false;
	}

	if (bufleng <= sizeof(EGPHEADER)) {
		return false;
	}

	MoveMemory(dstegph, srcbuf, sizeof(EGPHEADER));

	//typedef struct tag_egpheader
	//{
	//	char magicstr[32];//EvaluateGlobalPosition
	//	char version[16];
	//	char fbxdate[256];
	//	int animno;
	//	int jointnum;
	//	int framenum;
	//	int reserved;
	//}EGPHEADER;

	int magicstrlen;
	magicstrlen = strlen(dstegph->magicstr);
	if ((magicstrlen <= 0) || (magicstrlen >= 32)) {
		return false;
	}
	int cmp;
	cmp = strcmp(dstegph->magicstr, "EvaluateGlobalPosition");
	if (cmp != 0) {
		return false;
	}

	int versionstrlen;
	versionstrlen = strlen(dstegph->version);
	if ((versionstrlen <= 0) || (versionstrlen >= 16)) {
		return false;
	}
	int cmp2;
	cmp2 = strcmp(dstegph->version, "0.0.0.1");
	if (cmp2 != 0) {
		return false;
	}
	
	int fbxdatelen;
	fbxdatelen = strlen(dstegph->fbxdate);
	if ((fbxdatelen <= 0) || (fbxdatelen >= 256)) {
		return false;
	}
	int cmp3;
	cmp3 = strcmp(dstegph->fbxdate, fbxdate);
	if (cmp3 != 0) {
		return false;
	}


	int curmotid;
	curmotid = srcanimno + 1;
	MOTINFO* curmi;
	curmi = pmodel->GetMotInfo(curmotid);
	if (!curmi) {
		return 1;
	}
	int framenum;
	framenum = curmi->frameleng;
	if (framenum <= 0) {
		return 1;
	}
	int jointnum;
	jointnum = pmodel->GetBoneListSize();
	if (jointnum <= 0) {
		return 1;
	}


	if (dstegph->animno != srcanimno) {
		return false;
	}
	if (dstegph->framenum != framenum) {
		return false;
	}
	if (dstegph->reserved != 0) {
		return false;
	}

	DWORD buffersize = 0;
	buffersize = sizeof(EGPHEADER) +
		jointnum * sizeof(EGPJOINTHEADER) +
		jointnum * framenum * sizeof(EGPELEM);

	if ((bufleng < 0) || (bufleng != buffersize)) {
		return false;
	}

	return true;
}


bool LoadEGPFile(CModel* pmodel, WCHAR* pfilename, char* fbxdate, int animno)
{

	WCHAR infilename[MAX_PATH] = { 0L };
	swprintf_s(infilename, MAX_PATH, L"%s_anim%d.egp", pfilename, animno);


	HANDLE hfile;
	hfile = CreateFile(infilename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
		FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (hfile == INVALID_HANDLE_VALUE) {
		//_ASSERT(0);
		return false;
	}


	DWORD sizehigh;
	DWORD bufleng;
	bufleng = GetFileSize(hfile, &sizehigh);
	if (bufleng <= 0) {
		_ASSERT(0);
		return false;
	}
	if (sizehigh != 0) {
		_ASSERT(0);
		return false;
	}
	//char* newbuf;
	//newbuf = (char*)malloc(sizeof(char) * bufleng);//bufleng + 1
	//if (!newbuf) {
	//	_ASSERT(0);
	//	return false;
	//}
	if (!g_egpbuf || bufleng >= EGPBUFLENG) {
		_ASSERT(0);
		return false;
	}

	ZeroMemory(g_egpbuf, sizeof(char) * bufleng);
	DWORD rleng, readleng;
	rleng = bufleng;
	BOOL bsuccess;
	bsuccess = ReadFile(hfile, (void*)g_egpbuf, rleng, &readleng, NULL);
	if (!bsuccess || (rleng != readleng)) {
		_ASSERT(0);
		CloseHandle(hfile);
		return false;
	}

	EGPHEADER egpheader;
	ZeroMemory(&egpheader, sizeof(egpheader));
	bool isvalid;
	isvalid = ValidateEGPFile(&egpheader, g_egpbuf, bufleng, pmodel, fbxdate, animno);
	if (!isvalid) {
		//_ASSERT(0);
		CloseHandle(hfile);
		return false;
	}

	unsigned int jointindex;
	unsigned int frameno;
	for (jointindex = 0; jointindex < egpheader.jointnum; jointindex++) {
		DWORD curheaderpos;
		curheaderpos = sizeof(EGPHEADER) + jointindex * sizeof(EGPJOINTHEADER) + jointindex * egpheader.framenum * sizeof(EGPELEM);
		if (curheaderpos >= (bufleng - sizeof(EGPJOINTHEADER))) {
			CloseHandle(hfile);
			return false;
		}
		EGPJOINTHEADER jointheader;
		ZeroMemory(&jointheader, sizeof(EGPJOINTHEADER));
		MoveMemory(&jointheader, g_egpbuf + curheaderpos, sizeof(EGPJOINTHEADER));

		if (jointheader.jointindex != jointindex) {
			CloseHandle(hfile);
			return false;
		}

		int jointnamelen;
		jointnamelen = strlen(jointheader.jointname);
		if ((jointnamelen <= 0) || (jointnamelen >= 256)) {
			CloseHandle(hfile);
			return false;
		}
		CBone* curbone = 0;
		curbone = pmodel->GetBoneByName(jointheader.jointname);
		if (!curbone) {
			CloseHandle(hfile);
			return false;
		}

		//typedef struct tag_egpjointheader
		//{
		//	int jointindex;
		//	int framenum;
		//	char jointname[256];
		//}EGPJOINTHEADER;

		//curbone->veclClusterGlobalCurrentPosition.clear();

		
		size_t veccursize = curbone->veclClusterGlobalCurrentPosition.size();
		if (veccursize < egpheader.framenum) {
			curbone->veclClusterGlobalCurrentPosition.resize(egpheader.framenum);//!!!!!!!!!!!!
		}

		for (frameno = 0; frameno < egpheader.framenum; frameno++) {
			DWORD egpbufpos;
			egpbufpos = curheaderpos + sizeof(EGPJOINTHEADER) + frameno * sizeof(EGPELEM);
			if ((egpbufpos <= 0) || (egpbufpos > (bufleng - sizeof(EGPELEM)))) {
				CloseHandle(hfile);
				return false;
			}

			EGPELEM egpelem;
			ZeroMemory(&egpelem, sizeof(EGPELEM));
			MoveMemory(&egpelem, g_egpbuf + egpbufpos, sizeof(EGPELEM));

			//typedef struct tag_egpelem
			//{
			//	int jointindex;
			//	int frameno;
			//	FbxAMatrix egp;
			//}EGPELEM;

			if (egpelem.jointindex != jointindex) {
				CloseHandle(hfile);
				return false;
			}

			if (egpelem.frameno != frameno) {
				CloseHandle(hfile);
				return false;
			}

			FbxAMatrix curegp;
			curegp = egpelem.egp;
			//curbone->veclClusterGlobalCurrentPosition.push_back(curegp);
			curbone->veclClusterGlobalCurrentPosition[frameno] = curegp;//VS‚Ìpush_back‚Í’x‚¢‚ç‚µ‚¢‚Ì‚Å
		}
	}

	CloseHandle(hfile);

	return true;
}
