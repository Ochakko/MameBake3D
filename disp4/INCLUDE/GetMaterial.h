#ifndef GETMATERIALH
#define GETMATERIALH

#include <mqomaterial.h>
#include <map>
using namespace std;

CMQOMaterial* GetMaterialFromNo( map<int,CMQOMaterial*>& srcmat, int matno );

#endif