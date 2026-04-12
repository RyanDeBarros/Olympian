namespace oly
{
	enum class TransformModifierType
	{
		None = 0x0,
		Shear = 0x1,
		Pivot = 0x2,
		Offset = 0x3
	};

	enum class StorageMode
	{
		Discard = 0x0,
		Keep = 0x1
	};
}
