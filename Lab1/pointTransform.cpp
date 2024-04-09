#include "pointTransform.h"
#include"entity.hpp"


PointTransform::PointTransform(Entity& owner)
	:Component(ComponentConstructorArgs(PointTransform)),
	position{}
{
}

MyMat PointTransform::Get()
{
	return MyMTrans(position);
}
