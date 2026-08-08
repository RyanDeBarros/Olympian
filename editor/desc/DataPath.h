#pragma once

#include <functional>
#include <ostream>
#include <optional>
#include <span>
#include <typeindex>
#include <vector>

// TODO v9.3 move DataPath to imtk

namespace oly::editor
{
	struct DataPathStep
	{
		int v;

		constexpr DataPathStep(int v) : v(v) {}
		
		constexpr bool operator==(const DataPathStep& o) const { return v == o.v; }
		constexpr bool operator==(int o) const { return v == o; }

		constexpr DataPathStep& operator++() { ++v; return *this; }
		constexpr DataPathStep operator++(int) { DataPathStep s = *this; ++v; return s; }
		constexpr DataPathStep operator+(int x) const { return DataPathStep(v + x); }
		constexpr DataPathStep& operator--() { --v; return *this; }
		constexpr DataPathStep operator--(int) { DataPathStep s = *this; --v; return s; }
		constexpr DataPathStep operator-(int x) const { return DataPathStep(v - x); }
	};

	class DataPath;

	class DataPathSource
	{
		friend class DataPath;
		std::vector<DataPathStep> _path;

	public:
		DataPathSource() = default;
		DataPathSource(DataPath path);

		DataPathSource operator/(DataPathStep step) const;
		DataPathSource& operator/=(DataPathStep step);
	};

	class DataPath
	{
		friend class DataPathSource;
		std::span<const DataPathStep> _path;

	public:
		DataPath() = default;
		DataPath(const DataPathSource& source);
		DataPath& operator=(const DataPathSource& source);

		DataPathSource Copy() const;
		DataPathSource operator/(DataPathStep step) const;

		bool Empty() const;
		DataPathStep Step() const;
		DataPath Next() const;

		friend std::ostream& operator<<(std::ostream& os, DataPath path);
	};

	class DataPathLink
	{
		class Node
		{
			friend class DataPathLink;

			std::shared_ptr<Node> parent;
			std::optional<DataPathStep> step;

			DataPathSource ComputePath() const;
		};

		std::shared_ptr<Node> node;

	public:
		DataPathLink();
		DataPathLink(DataPathLink& parent, DataPathStep step);

		DataPathLink(const DataPathLink& o) = delete;
		DataPathLink(DataPathLink&& o) noexcept = default;

		DataPathLink& operator=(const DataPathLink& o) = delete;
		DataPathLink& operator=(DataPathLink&& o) noexcept = default;

		std::optional<DataPathStep> Step() const;
		void SetStep(DataPathStep step);
		DataPathLink Share() const;
		DataPathSource ComputePath() const;
	};

#define DATA_PATH_SUBLINK(subpath) DataPathLink(this->link, subpath)
}
