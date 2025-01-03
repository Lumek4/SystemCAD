#include<nlohmann/json.hpp>
#include"saving.h"

using namespace nlohmann;
using value_t = nlohmann::detail::value_t;
static const std::array geometryTypesArray =
{
	(std::string)"torus",
	(std::string)"bezierC0",
	(std::string)"bezierC2",
	(std::string)"interpolatedC2",
	(std::string)"bezierSurfaceC0",
	(std::string)"bezierSurfaceC2",
	(std::string)"gregoryPatch"
};
nlohmann::detail::other_error my_error(int id, const std::string& message)
{
	return nlohmann::detail::other_error::create(id, message, nullptr);
}

#define CHECKTYPE(key, checked_type) \
	do{if(object.at(#key).type()!=value_t::checked_type)throw my_error(__LINE__, "test");}while(false)
static bool validateV2(json& object)
{
	if (object.type() != value_t::object)
		throw my_error(__LINE__, "test");
	CHECKTYPE(x, number_float);
	CHECKTYPE(y, number_float);
	return true;
}
static bool validateV2u(json& object)
{
	if (object.type() != value_t::object)
		throw my_error(__LINE__, "test");
	CHECKTYPE(x, number_unsigned);
	CHECKTYPE(y, number_unsigned);
	return true;
}
static bool validateV3(json& object)
{
	if (object.type() != value_t::object)
		throw my_error(__LINE__, "test");
	CHECKTYPE(x, number_float);
	CHECKTYPE(y, number_float);
	CHECKTYPE(z, number_float);
	return true;
}
static bool validateIdArray(json& arr)
{
	if (arr.type() != value_t::array)
		throw my_error(__LINE__, "test");
	for (auto it = arr.begin(); it < arr.end(); it++)
	{
		json& object = (*it);
		CHECKTYPE(id, number_unsigned);
	}
	return true;
}
static bool validatePatchArray(json& arr)
{
	if (arr.type() != value_t::array)
		throw my_error(__LINE__, "test");
	for (auto it = arr.begin(); it < arr.end(); it++)
	{
		json& object = (*it);
		CHECKTYPE(objectType, string);
		CHECKTYPE(id, number_unsigned);
		CHECKTYPE(name, string);
		if (!validateIdArray(object.at("controlPoints")))
			throw my_error(__LINE__, "test");
		if (!validateV2u(object.at("samples")))
			throw my_error(__LINE__, "test");
	}
	return true;
}
GEOMETRY_TYPE getGeometryType(const std::string& v)
{
	int t = -1;
	for (int i = 0; i < geometryTypesArray.size(); i++)
		if (geometryTypesArray[i].compare(v) == 0)
		{
			t = i; break;
		}
	return (GEOMETRY_TYPE)t;
}
bool validate(json& file)
{
	if (file.type() != value_t::object)
		throw my_error(__LINE__, "test");

	try
	{
		json& geometry = file.at("geometry"),
			& points = file.at("points");
		if (geometry.type() != value_t::array ||
			points.type() != value_t::array)
			throw my_error(__LINE__, "test");

		for (auto it = points.begin(); it < points.end(); it++)
		{
			json& object = (*it);
			if (object.type() != value_t::object)
				throw my_error(__LINE__, "test");
			CHECKTYPE(id, number_unsigned);
			CHECKTYPE(name, string);
			json& position = object.at("position");
			if (!validateV3(position))
				throw my_error(__LINE__, "test");
		}
		for (auto it = geometry.begin(); it < geometry.end(); it++)
		{
			json& object = (*it);
			if (object.type() != value_t::object)
				throw my_error(__LINE__, "test");

			CHECKTYPE(objectType, string);
			std::string* type = object.at("objectType").get<std::string*>();
			GEOMETRY_TYPE t = getGeometryType(*type);
			CHECKTYPE(id, number_unsigned);
			CHECKTYPE(name, string);
			switch (t)
			{
			case GEOMETRY_TYPE::INVALID:
				throw my_error(__LINE__, "test");
			case GEOMETRY_TYPE::torus:
				CHECKTYPE(smallRadius, number_float);
				CHECKTYPE(largeRadius, number_float);
				if (!validateV3(object.at("position")))
					throw my_error(__LINE__, "test");
				if (!validateV3(object.at("rotation")))
					throw my_error(__LINE__, "test");
				if (!validateV3(object.at("scale")))
					throw my_error(__LINE__, "test");
				if (!validateV2u(object.at("samples")))
					throw my_error(__LINE__, "test");
				break;
			case GEOMETRY_TYPE::bezierC0:
				if (!validateIdArray(object.at("controlPoints")))
					throw my_error(__LINE__, "test");
				break;
			case GEOMETRY_TYPE::bezierC2:
				if (!validateIdArray(object.at("deBoorPoints")))
					throw my_error(__LINE__, "test");
				break;
			case GEOMETRY_TYPE::interpolatedC2:
				if (!validateIdArray(object.at("controlPoints")))
					throw my_error(__LINE__, "test");
				break;
			case GEOMETRY_TYPE::bezierSurfaceC0:
				if (!validatePatchArray(object.at("patches")))
					throw my_error(__LINE__, "test");
				{
					json& parameterWrapped = object.at("parameterWrapped");
					if (parameterWrapped.at("u").type() != value_t::boolean ||
						parameterWrapped.at("v").type() != value_t::boolean)
						throw my_error(__LINE__, "test");
				}
				if (!validateV2u(object.at("size")))
					throw my_error(__LINE__, "test");
				break;
			case GEOMETRY_TYPE::bezierSurfaceC2:
				if (!validatePatchArray(object.at("patches")))
					throw my_error(__LINE__, "test");
				{
					json& parameterWrapped = object.at("parameterWrapped");
					if (parameterWrapped.at("u").type() != value_t::boolean ||
						parameterWrapped.at("v").type() != value_t::boolean)
						throw my_error(__LINE__, "test");
				}
				if (!validateV2u(object.at("size")))
					throw my_error(__LINE__, "test");
				break;
			case GEOMETRY_TYPE::gregoryPatch:
				if (!validateIdArray(object.at("neighbors")))
					throw my_error(__LINE__, "test");
				break;
			}
		}

	}
	catch (nlohmann::detail::out_of_range e)
	{
		throw my_error(__LINE__, "test");
	}
	return true;
}