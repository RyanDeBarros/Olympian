#pragma once

#include "desc/DataPath.h"
#include "desc/FieldSetAction.h"

#include "util/TypeErasedBox.h"

namespace oly::editor
{
	struct IDoubleDescriptor
	{
		virtual ~IDoubleDescriptor() = default;
		virtual void* PathGet(DataPath path, std::type_index type) = 0;
		virtual void PrintPath(std::ostream& os, DataPath path) const = 0;
		virtual bool DrawFinalize() = 0;
		virtual bool QueryDirty() = 0;
		virtual TypeErasedBox CopyScratch() const = 0;
		virtual std::unique_ptr<UndoAction> ScratchUndoAction(TypeErasedBox original) const = 0;
	};

	template<typename Descriptor>
	struct DoubleDescriptor : public IDoubleDescriptor
	{
		Descriptor scratch;
		Descriptor disk;

		DoubleDescriptor() = default;
		DoubleDescriptor(Descriptor scratch, Descriptor disk) : scratch(std::move(scratch)), disk(std::move(disk)) {}

		void* PathGet(DataPath path, std::type_index type) override
		{
			return scratch.PathGet(path, type);
		}

		void PrintPath(std::ostream& os, DataPath path) const override
		{
			scratch.PrintPath(os, path);
		}

		bool DrawFinalize() override
		{
			return scratch.DrawFinalize(DataPath());
		}

		bool QueryDirty() override
		{
			return scratch.QueryDirty(disk);
		}

		TypeErasedBox CopyScratch() const
		{
			return TypeErasedBox(scratch);
		}

		std::unique_ptr<UndoAction> ScratchUndoAction(TypeErasedBox original) const override
		{
			if (auto og = original.consume_unique<Descriptor>())
				return std::make_unique<FieldSetAction<Descriptor, false>>(DataPath(), std::move(*og), scratch);
			else
				return nullptr;
		}

		void WriteToDisk()
		{
			disk = scratch;
		}

		void LoadFromDisk()
		{
			scratch = disk;
		}
	};
}
