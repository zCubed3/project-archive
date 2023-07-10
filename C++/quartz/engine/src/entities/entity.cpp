#include "entity.hpp"

#include <rendering/base/model.hpp>
#include <glm/gtx/quaternion.hpp>

void Entity::Draw(Renderer* renderer) 
{
	if (model != nullptr && material != nullptr)
		model->Draw(renderer, material, GetMatrix());
}

glm::mat4 Entity::GetMatrix() 
{
	glm::mat4 returnVal = glm::identity<glm::mat4>();

	returnVal = glm::translate(returnVal, position);
	returnVal *= glm::toMat4(rotation);
	returnVal = glm::scale(returnVal, scale);

	return returnVal;
}