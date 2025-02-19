//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-02-11 11:32:37
//

#include <Utility/Math.hpp>

#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/constants.hpp>

glm::vec3 Math::GetNormalizedPerpendicular(glm::vec3 base)
{
    if (abs(base.x) > abs(base.y)) {
        float len = sqrt(base.x * base.x + base.y * base.y);
        return glm::vec3(base.z, 0.0f, -base.x) / len;
    } else {
        float len = sqrt(base.y * base.y + base.z * base.z);
        return glm::vec3(0.0f, base.z, -base.y) / len;
    }
}

bool Math::DecomposeTransform(const glm::mat4& transform, glm::vec3& translation, glm::vec3& rotation, glm::vec3& scale)
{
    // From glm::decompose in matrix_decompose.inl

	using namespace glm;
	using T = float;
	mat4 LocalMatrix(transform);
	// Normalize the matrix.
	if (epsilonEqual(LocalMatrix[3][3], static_cast<float>(0), epsilon<T>()))
		return false;
	// First, isolate perspective.  This is the messiest.
	if (
		epsilonNotEqual(LocalMatrix[0][3], static_cast<T>(0), epsilon<T>()) ||
		epsilonNotEqual(LocalMatrix[1][3], static_cast<T>(0), epsilon<T>()) ||
		epsilonNotEqual(LocalMatrix[2][3], static_cast<T>(0), epsilon<T>()))
	{
		// Clear the perspective partition
		LocalMatrix[0][3] = LocalMatrix[1][3] = LocalMatrix[2][3] = static_cast<T>(0);
		LocalMatrix[3][3] = static_cast<T>(1);
	}
	// Next take care of translation (easy).
	translation = vec3(LocalMatrix[3]);
	LocalMatrix[3] = vec4(0, 0, 0, LocalMatrix[3].w);
	vec3 Row[3], Pdum3;
	// Now get scale and shear.
	for (length_t i = 0; i < 3; ++i)
		for (length_t j = 0; j < 3; ++j)
			Row[i][j] = LocalMatrix[i][j];
	// Compute X scale factor and normalize first row.
	scale.x = length(Row[0]);
	Row[0] = detail::scale(Row[0], static_cast<T>(1));
	scale.y = length(Row[1]);
	Row[1] = detail::scale(Row[1], static_cast<T>(1));
	scale.z = length(Row[2]);
	Row[2] = detail::scale(Row[2], static_cast<T>(1));
	// At this point, the matrix (in rows[]) is orthonormal.
	// Check for a coordinate system flip.  If the determinant
	// is -1, then negate the matrix and the scaling factors.

	rotation.y = asin(-Row[0][2]);
	if (cos(rotation.y) != 0) {
		rotation.x = atan2(Row[1][2], Row[2][2]);
		rotation.z = atan2(Row[0][1], Row[0][0]);
	}
	else {
		rotation.x = atan2(-Row[2][0], Row[1][1]);
		rotation.z = 0;
	}
    rotation = glm::degrees(rotation);
	return true;
}

glm::quat Math::EulerToQuat(glm::vec3 euler)
{
	return glm::quat(glm::radians(euler));
}

glm::vec3 Math::QuatToEuler(glm::quat quat)
{
	glm::vec3 euler = glm::eulerAngles(quat);
	return glm::degrees(euler);
}

glm::vec3 Math::QuatToForward(glm::quat q)
{
	return glm::vec3(
        2.0f * (q.x * q.z + q.w * q.y),
        2.0f * (q.y * q.z - q.w * q.x),
        1.0f - 2.0f * (q.x * q.x + q.y * q.y)
    );
}

glm::vec3 Math::EulerToForward(glm::vec3 deg)
{
	glm::vec3 eulerAnglesRadians = glm::radians(deg);
    glm::mat4 rotationMatrix = glm::yawPitchRoll(
        eulerAnglesRadians.y, // Yaw
        eulerAnglesRadians.x, // Pitch
        eulerAnglesRadians.z  // Roll
    );

    glm::vec4 forward = rotationMatrix * glm::vec4(0, 0, 1, 0);
    return glm::normalize(glm::vec3(forward));
}

Vector<glm::vec4> Math::FrustumCorners(glm::mat4 view, glm::mat4 proj)
{
	glm::mat4 inv = glm::inverse(proj * view);

    Vector<glm::vec4> corners = {
        glm::vec4(-1.0f,  1.0f, 0.0f, 1.0f),
        glm::vec4( 1.0f,  1.0f, 0.0f, 1.0f),
        glm::vec4( 1.0f, -1.0f, 0.0f, 1.0f),
        glm::vec4(-1.0f, -1.0f, 0.0f, 1.0f),
        glm::vec4(-1.0f,  1.0f, 1.0f, 1.0f),
        glm::vec4( 1.0f,  1.0f, 1.0f, 1.0f),
        glm::vec4( 1.0f, -1.0f, 1.0f, 1.0f),
        glm::vec4(-1.0f, -1.0f, 1.0f, 1.0f),
    };

    // To convert from world space to NDC space, multiply by the inverse of the camera matrix (projection * view) then perspective divide
    for (int i = 0; i < 8; i++) {
        glm::vec4 h = inv * corners[i];
        h.x /= h.w;
        h.y /= h.w;
        h.z /= h.w;
        corners[i] = h;
    }
    return corners;
}

Vector<glm::vec4> Math::CascadeCorners(glm::mat4 view, float fov, float aspectRatio, float nearPlane, float farPlane)
{
	return FrustumCorners(view, glm::perspective(fov, aspectRatio, nearPlane, farPlane));
}
