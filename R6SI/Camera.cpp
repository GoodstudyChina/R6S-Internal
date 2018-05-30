#include "Camera.h"
#include "Offsets.h"
#include "Memory.h"
#include "Helpers.h"

namespace Engine
{
	Vector3& Camera::GetViewRight()
	{
		if (!Memory::IsValidPtr<Camera>(this))
			return *(new Vector3());

		return *Memory::Ptr<Vector3*>(this, OFFSET_CAMERA_VIEWRIGHT);
	}

	Vector3& Camera::GetViewUp()
	{
		if (!Memory::IsValidPtr<Camera>(this))
			return *(new Vector3());

		return *Memory::Ptr<Vector3*>(this, OFFSET_CAMERA_VIEWUP);
	}

	Vector3& Camera::GetViewForward()
	{
		if (!Memory::IsValidPtr<Camera>(this))
			return *(new Vector3());

		return *Memory::Ptr<Vector3*>(this, OFFSET_CAMERA_VIEFORWARD);
	}

	Vector3& Camera::GetViewTranslation()
	{
		if (!Memory::IsValidPtr<Camera>(this))
			return *(new Vector3());

		return *Memory::Ptr<Vector3*>(this, OFFSET_CAMERA_VIEWTRANSLATION);
	}

	float Camera::GetViewFovX()
	{
		if (!Memory::IsValidPtr<Camera>(this))
			return 0.0f;

		return *Memory::Ptr<float*>(this, OFFSET_CAMERA_VIEWFOVX);
	}

	float Camera::GetViewFovY()
	{
		if (!Memory::IsValidPtr<Camera>(this))
			return 0.0f;

		return *Memory::Ptr<float*>(this, OFFSET_CAMERA_VIEWFOVY);
	}

	Vector3 Camera::WorldToScreen(Vector3& position)
	{
		if (!Memory::IsValidPtr<Camera>(this))
			return *(new Vector3());

		Vector3 temp = position - GetViewTranslation();

		float x = temp.Dot(GetViewRight());
		float y = temp.Dot(GetViewUp());
		float z = temp.Dot(GetViewForward() * -1);

		int horizontal = 1920;
		int vertical = 1080;

		Helpers::GetResolution(horizontal, vertical);

		return *(new Vector3(
			(horizontal / 2) * (1 + x / GetViewFovX() / z),
			(vertical / 2) * (1 - y / GetViewFovY() / z),
			z));
	}

}