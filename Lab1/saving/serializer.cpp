#include "serializer.h"

Entity* Deserializer::Point(DirectX::XMFLOAT3 position, const std::string& name)
{
	auto* e = Entity::New();
	auto* t = e->AddComponent<PointTransform>();
	t->Translate(position);
	e->AddComponent<PointRenderer>();
	e->SetName(name.c_str());
	return e;
}
