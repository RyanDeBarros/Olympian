#include "SubMaterialComponents.h"

namespace oly::physics
{
	void AngularSnapping::set_uniformly_spaced_without_threshold(const size_t angles, float angle_offset)
	{
		snaps.clear();
		const float multiple = glm::two_pi<float>() / angles;
		for (size_t i = 0; i < angles; ++i)
			snaps.insert(i * multiple + angle_offset);
	}

	void AngularSnapping::set_uniformly_spaced(const size_t angles, float angle_offset)
	{
		snaps.clear();
		const float multiple = glm::two_pi<float>() / angles;
		for (size_t i = 0; i < angles; ++i)
			snaps.insert(i * multiple + angle_offset);
		angle_threshold = glm::pi<float>() / angles;
	}
}
