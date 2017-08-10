#ifndef CONSTRAINTELEMH
#define CONSTRAINTELEMH

class CBtObject;
class CBone;
class btConeTwistConstraint;

typedef struct tag_constraintelem
{
	CBtObject* childbto;
	CBone* centerbone;
	btConeTwistConstraint* constraint;
} CONSTRAINTELEM;


#endif
