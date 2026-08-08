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
		virtual bool QueryDirty() = 0;
		virtual TypeErasedBox CopyScratch() const = 0;
		virtual std::unique_ptr<UndoAction> ScratchUndoAction(TypeErasedBox original) const = 0;
		virtual bool ScratchUndoActionQuery(TypeErasedBox original, std::unique_ptr<UndoAction>& action) const = 0;
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

		bool QueryDirty() override
		{
			return scratch.QueryDirty(disk);
		}

		TypeErasedBox CopyScratch() const
		{
			Descriptor copy;
			copy.CopyData(scratch);
			return TypeErasedBox(std::move(copy));
		}

		std::unique_ptr<UndoAction> ScratchUndoAction(TypeErasedBox original) const override
		{
			if (auto og = original.consume_unique<Descriptor>())
			{
				Descriptor newest;
				newest.CopyData(scratch);
				return std::make_unique<DescriptorSetAction<Descriptor, void>>(DataPath(), std::move(*og), std::move(newest));
			}
			else
				return nullptr;
		}

		bool ScratchUndoActionQuery(TypeErasedBox original, std::unique_ptr<UndoAction>& action) const override
		{
			if (auto og = original.as<Descriptor>())
			{
				if (scratch.QueryDirty(*og))
				{
					if (auto og = original.consume_unique<Descriptor>())
					{
						Descriptor newest;
						newest.CopyData(scratch);
						action = std::make_unique<DescriptorSetAction<Descriptor, void>>(DataPath(), std::move(*og), std::move(newest));
					}
					else
						action = nullptr;

					return true;
				}
			}

			return false;
		}

		void WriteToDisk()
		{
			disk.CopyData(scratch);
		}

		void LoadFromDisk()
		{
			scratch.CopyData(disk);
		}

		void ResetScratch()
		{
			scratch = Descriptor();
		}
	};
}
