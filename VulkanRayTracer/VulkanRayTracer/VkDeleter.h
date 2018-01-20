#pragma once
#include <functional>

/// <summary>
/// Wraps a vulkan structure & automatically calls it's free function at the end of the objects lifetime.
/// </summary>

template <typename T>
class VKDeleter
{
public:
	VKDeleter() : VKDeleter([](T, VkAllocationCallbacks*) {}) {}

	VKDeleter(std::function<void(T, VkAllocationCallbacks*)> deletef)
	{
		this->deleter = [=](T obj) { deletef(obj, nullptr); };
	}

	VKDeleter(const VKDeleter<VkInstance>& instance, std::function<void(VkInstance, T, VkAllocationCallbacks*)> deletef)
	{
		this->deleter = [&instance, deletef](T obj) { deletef(instance, obj, nullptr); };
	}

	VKDeleter(const VKDeleter<VkDevice>& device, std::function<void(VkDevice, T, VkAllocationCallbacks*)> deletef)
	{
		this->deleter = [&device, deletef](T obj) { deletef(device, obj, nullptr); };
	}


	~VKDeleter()
	{
		CleanUp();
	}


	const T* operator &() const
	{
		return &object;
	}

	T* Replace()
	{
		CleanUp();
		return &object;
	}

	operator T() const
	{
		return object;
	}

	void operator=(T rhs)
	{
		if (rhs != object)
		{
			CleanUp();
			object = rhs;
		}
	}

	template<typename V>
	bool operator==(V rhs)
	{
		return object == T(rhs);
	}


private:
	T object{ VK_NULL_HANDLE };
	std::function<void(T)> deleter;

	void CleanUp() {
		if (object != VK_NULL_HANDLE)
		{
			deleter(object);
		}
		object = VK_NULL_HANDLE;
	}
};