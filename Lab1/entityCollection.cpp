#include "entityCollection.hpp"
#include<algorithm>

bool EntityCollection::Remove(Entity* v)
{
	{
		auto it = find(v->containingCollections.begin(), v->containingCollections.end(), this);
		if (it != v->containingCollections.end())
			v->containingCollections.erase(it);
	}
	{
		auto& l = Get();
		auto it = find(l.begin(), l.end(), v);
		if (it != l.end())
		{
			onRemove.Notify(it - l.begin());
			it = find(l.begin(), l.end(), v);
			if (it != l.end())
			{
				l.erase(it);
				return true;
			}
		}
		return false;
	}
}
bool EntityCollection::Add(Entity* v)
{
	if (!v || Contains(v))
		return false;
	Get().push_back(v);
	v->containingCollections.push_back(this);
	onAdd.Notify();
	return true;
}
void EntityCollection::TransferRelations(Entity* from, Entity* to)
{
	DirectX::XMINT2 xy;
	auto& l = Get();

	xy.x = std::find(l.begin(), l.end(), from) - l.begin();
	if (xy.x == l.size())
		return;

	xy.y = std::find(l.begin(), l.end(), to) - l.begin();
	if (xy.y == l.size())
		Add(to);


	onTransferRelations.Notify(xy);
}
