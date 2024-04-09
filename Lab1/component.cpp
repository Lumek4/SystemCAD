#include "component.h"
#include<vector>
#include<cassert>
#include"entity.hpp"
#include"catalogue.hpp"
#include"all_components.h"

Component::Component(Entity& owner, CatalogueBase& catalogue)
	:owner(owner), catalogue(catalogue)
{
	catalogue.Register(this);
}

Component::~Component()
{
	catalogue.Unregister(this);
}

