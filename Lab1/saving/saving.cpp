#include "saving.h"
#include<nlohmann/json.hpp>
#include<fstream>
#include<array>
#include"serializer.h"
using namespace nlohmann;
using value_t = nlohmann::detail::value_t;
using namespace DirectX;

extern bool validate(nlohmann::json& file);

SaveResult save(const CadApplication& app, const char* filepath)
{
	return SaveResult::Success;
}

SaveResult load(CadApplication& app, const char* filepath)
{
	std::ifstream f(filepath);
	if (!f.is_open())
		return SaveResult::NoFile;
	
	json file = json::parse(f);
	try
	{
		validate(file);
	}
	catch (nlohmann::detail::other_error e)
	{
		return SaveResult::InvalidFile;
	}
	Entity::Clear();
	app.mainFolder = Entity::New()->AddComponent<Folder>();

	json& points = file["points"];
	std::map<unsigned, Entity*> ids;
	for (auto it = points.begin(); it < points.end(); it++)
	{
		json& point = *it;
		Entity* e = Deserializer::Point(
			XMFLOAT3{
				point["position"]["x"],
				point["position"]["y"],
				point["position"]["z"]
			},
			point["name"]
		);
		app.mainFolder->Add(e);
		ids[point["id"]] = e;
	}
	json& geometry = file["geometry"];


	return SaveResult::Success;
}
