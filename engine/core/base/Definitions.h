namespace oly
{
	// TODO v7 definition enums should use explicit values to be order-independent

	enum class TransformModifierType
	{
		None,
		Shear,
		Pivot,
		Offset
	};

	enum class StorageMode
	{
		Discard,
		Keep
	};
}
