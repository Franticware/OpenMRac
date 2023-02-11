#ifndef GLM1_H
#define GLM1_H

#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/ext/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale
#include <glm/ext/matrix_clip_space.hpp> // glm::perspective
#include <glm/ext/scalar_constants.hpp> // glm::pi
#include <glm/gtc/type_ptr.hpp> // glm::value_ptr

namespace glm
{
inline void assign1(float* fa, const glm::vec2& v) { fa[0] = v[0]; fa[1] = v[1]; }
inline void assign1(float* fa, const glm::vec3& v) { fa[0] = v[0]; fa[1] = v[1]; fa[2] = v[2]; }
inline void assign1(float* fa, const glm::vec4& v) { fa[0] = v[0]; fa[1] = v[1]; fa[2] = v[2]; fa[3] = v[3]; }
}

#endif
