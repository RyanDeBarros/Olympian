#pragma once

#include "external/TOML.h"
#include "external/GL.h"
#include "core/util/DeferredStringParam.h"
#include "core/util/LogLevel.h"

#include <array>
#include <cstdint>
#include <string>
#include <algorithm>
#include <source_location>

namespace oly::assets
{
	namespace internal
	{
		template<typename Key>
		constexpr std::string translate_key(Key key)
		{
			const auto value = static_cast<underlying_or_self_t<Key>>(key);
			constexpr size_t KeySize = OLYMPIAN_ENGINE_ASSET_KEY_SIZE;
			std::array<char, KeySize> bytes;

			for (size_t i = 0; i < KeySize; ++i)
				bytes[KeySize - 1 - i] = char((value >> (i * KeySize)) & 0xFF);

			return std::string(bytes.begin(), std::find(bytes.begin(), bytes.end(), '\0'));
		}
	}

	struct NoKey
	{
	};

	constexpr NoKey NO_KEY{};

	template<typename Key>
	constexpr std::string key_string(Key key)
	{
		if constexpr (std::is_same_v<Key, NoKey>)
			return "";
		else
			return internal::translate_key(key);
	}

	template<typename T>
	struct PartialView
	{
		T& val;

		const T& operator*() const { return val; }
		T& operator*() { return val; }
		const T* operator->() const { return &val; }
		T* operator->() { return &val; }
	};

	namespace internal
	{
		template<typename T>
		bool try_parse(TOMLNode node, T& obj);

		template<typename T>
		bool try_parse(TOMLNode node, PartialView<T> obj);

		extern void log_context_at_level(LogLevel level, const DeferredStringParam& msg, std::source_location location);

		namespace fail_causes
		{
			constexpr const char* CANNOT_PARSE = "cannot parse";
			constexpr const char* INVALID_ARRAY_ELEMENT = "skipping invalid array element in";
			constexpr const char* FAILED_RESTRICTION = "failed restriction for";
		}
	}

	class Parser
	{
		TOMLNode node;
		DeferredStringParam log_suffix;
		ErrorCode error_code;
		bool fatal;

		// TODO v7 refactor common parsing logic

		template<typename Key>
		DeferredStringList build_message(Key key, const StringParam& cause = internal::fail_causes::CANNOT_PARSE, DeferredStringList&& elaboration = {}) const
		{
			DeferredStringList list{ cause.transfer(), " " };
			
			if constexpr (std::is_same_v<Key, NoKey>)
				list << "field";
			else
				list << key_string(key) << " field";
			
			if (!elaboration.empty())
				list << " " << std::move(elaboration);

			if (!log_suffix.empty())
				list << " " << log_suffix;

			return list;
		}

		template<typename Key>
		void log_at_level(LogLevel level, std::source_location location, Key key,
			const StringParam& cause = internal::fail_causes::CANNOT_PARSE, DeferredStringList&& elaboration = {}) const
		{
			if ((int)level >= (int)LogLevel::Error && fatal)
				level = LogLevel::Fatal;
			internal::log_context_at_level(level, build_message(key, cause.transfer(), std::move(elaboration)), location);
		}

		template<typename Key, typename Index>
		void log_at_level(LogLevel level, std::source_location location, Key key, Index e) const
		{
			if ((int)level >= (int)LogLevel::Error && fatal)
				level = LogLevel::Fatal;
			internal::log_context_at_level(level, build_message(key, DeferredStringList{ "unrecognized enum (", std::to_string(e), ") in"}.str()), location);
		}

		template<typename Key, bool Throws>
		struct Accessor
		{
		protected:
			const Parser& parser;
			Key key;

		public:
			Accessor(const Parser& parser, Key key) : parser(parser), key(key) {}

		protected:
			void report(std::source_location location, const StringParam& cause = internal::fail_causes::CANNOT_PARSE, DeferredStringList&& elaboration = {}) const
			{
				parser.log_at_level(Throws ? LogLevel::Error : LogLevel::Warning, location, key, cause.transfer(), std::move(elaboration));
				if constexpr (Throws)
					throw Error(parser.error_code);
			}

			template<typename Index>
			void report(std::source_location location, Index e)
			{
				parser.log_at_level(Throws ? LogLevel::Error : LogLevel::Warning, location, key, e);
				if constexpr (Throws)
					throw Error(parser.error_code);
			}

			TOMLNode field() const
			{
				return parser.field(key);
			}
		};

		template<typename Key, typename Predefined, typename Translator>
		struct Defaulted;

		template<typename Key, typename Translator>
		struct Defaulted<Key, void, Translator> : public Accessor<Key, false>
		{
			using Accessor<Key, false>::Accessor;

			typename Translator::EnumType operator()(typename Translator::EnumType def = Translator::val(), std::source_location location = std::source_location::current()) const
			{
				if (auto value = this->field())
				{
					typename Translator::IndexType index;
					if (internal::try_parse<typename Translator::IndexType>(value, index))
					{
						try
						{
							return Translator::val(index, def);
						}
						catch (const std::out_of_range&)
						{
							this->report(location, index);
						}
					}
					else
						this->report(location);
				}

				return std::move(def);
			}
		};

		template<typename Key, typename Predefined>
		struct Defaulted<Key, Predefined, void> : public Accessor<Key, false>
		{
			using Accessor<Key, false>::Accessor;

			static_assert(!std::is_same_v<Predefined, TOMLNode>);
			static_assert(!std::is_same_v<Predefined, TOMLArray>);

			Predefined operator()(Predefined def = Predefined(), std::source_location location = std::source_location::current()) const
			{
				if (auto value = this->field())
				{
					if (!internal::try_parse<Predefined>(value, def))
						this->report(location);
				}

				return std::move(def);
			}
		};

		template<typename Key, typename T>
		struct Defaulted<Key, std::vector<T>, void> : public Accessor<Key, false>
		{
			using Accessor<Key, false>::Accessor;

			std::vector<T> operator()(std::source_location location = std::source_location::current()) const
			{
				if (auto value = this->field())
				{
					TOMLArray arr;
					if (internal::try_parse<TOMLArray>(value, arr))
					{
						std::vector<T> obj;
						obj.reserve(arr->size());
						for (size_t i = 0; i < arr->size(); ++i)
						{
							T el;
							if (internal::try_parse<T>((TOMLNode)*arr->get(i), el))
								obj.push_back(el);
							else
								this->report(location, internal::fail_causes::INVALID_ARRAY_ELEMENT);
						}

						return obj;
					}

					this->report(location);
				}
				return {};
			}
		};

		template<typename Key>
		struct Defaulted<Key, void, void> : public Accessor<Key, false>
		{
			using Accessor<Key, false>::Accessor;

			template<typename T>
			T operator()(T def, std::source_location location = std::source_location::current()) const
			{
				static_assert(!std::is_same_v<T, TOMLNode>);
				static_assert(!std::is_same_v<T, TOMLArray>);

				if (auto value = this->field())
				{
					if (!internal::try_parse<T>(value, def))
						this->report(location);
				}

				return std::move(def);
			}
		};

		template<typename Key, typename Predefined, typename Translator>
		struct Optional;

		template<typename Key, typename Translator>
		struct Optional<Key, void, Translator> : public Accessor<Key, false>
		{
			using Accessor<Key, false>::Accessor;

			bool operator()(typename Translator::EnumType& obj, std::source_location location = std::source_location::current()) const
			{
				if (auto value = this->field())
				{
					typename Translator::IndexType index;
					if (internal::try_parse<typename Translator::IndexType>(value, index))
					{
						try
						{
							obj = std::move(Translator::val(index));
							return true;
						}
						catch (const std::out_of_range&)
						{
							this->report(location, index);
						}
					}
					else
						this->report(location);
				}

				return false;
			}

			std::optional<typename Translator::EnumType> operator()(std::source_location location = std::source_location::current()) const
			{
				if (auto value = this->field())
				{
					typename Translator::IndexType index;
					if (internal::try_parse<typename Translator::IndexType>(value, index))
					{
						try
						{
							return std::move(Translator::val(index));
						}
						catch (const std::out_of_range&)
						{
							this->report(location, index);
						}
					}
					else
						this->report(location);
				}

				return std::nullopt;
			}
		};

		template<typename Key, typename Predefined>
		struct Optional<Key, Predefined, void> : public Accessor<Key, false>
		{
			using Accessor<Key, false>::Accessor;

			std::optional<Predefined> operator()(std::source_location location = std::source_location::current()) const
			{
				if (auto value = this->field())
				{
					if constexpr (std::is_same_v<Predefined, TOMLNode>)
					{
						return value;
					}
					else
					{
						Predefined obj;
						if (internal::try_parse<Predefined>(value, obj))
							return obj;
					}

					this->report(location);
				}
				return std::nullopt;
			}
		};

		template<typename Key>
		struct Optional<Key, TOMLArray, void> : public Accessor<Key, false>
		{
			using Accessor<Key, false>::Accessor;

			TOMLArray operator()(std::source_location location = std::source_location::current()) const
			{
				if (auto value = this->field())
				{
					TOMLArray obj;
					if (internal::try_parse<TOMLArray>(value, obj))
						return obj;

					this->report(location);
				}
				return nullptr;
			}
		};

		template<typename Key>
		struct Optional<Key, void, void> : public Accessor<Key, false>
		{
			using Accessor<Key, false>::Accessor;

			template<typename T>
			bool operator()(T& obj, std::source_location location = std::source_location::current()) const
			{
				if (auto value = this->field())
				{
					if constexpr (std::is_same_v<T, TOMLNode>)
					{
						obj = std::move(value);
						return true;
					}
					else
					{
						if (internal::try_parse<T>(value, obj))
							return true;
					}

					this->report(location);
					return false;
				}
				else
					return false;
			}

			template<typename T>
			bool operator()(std::optional<T>& obj, std::source_location location = std::source_location::current()) const
			{
				obj = std::nullopt;
				if (auto value = this->field())
				{
					T o;
					if (internal::try_parse<T>(value, o))
					{
						obj = std::move(o);
						return true;
					}

					this->report(location);
					return false;
				}
				else
					return false;
			}

			template<typename T>
			bool operator()(std::vector<T>& obj, std::source_location location = std::source_location::current()) const
			{
				if (auto value = this->field())
				{
					TOMLArray arr;
					if (internal::try_parse<TOMLArray>(value, arr))
					{
						obj.clear();
						obj.reserve(arr->size());
						for (size_t i = 0; i < arr->size(); ++i)
						{
							T el;
							if (internal::try_parse<T>((TOMLNode)*arr->get(i), el))
								obj.push_back(el);
							else
								this->report(location, internal::fail_causes::INVALID_ARRAY_ELEMENT);
						}

						return true;
					}

					this->report(location);
				}
				return false;
			}

			bool operator()(TOMLNode& obj) const
			{
				if (auto value = this->field())
				{
					obj = std::move(value);
					return true;
				}
				else
					return false;
			}

			bool operator()(std::optional<TOMLNode>& obj) const
			{
				obj = std::nullopt;
				if (auto value = this->field())
				{
					obj = std::move(value);
					return true;
				}
				else
					return false;
			}

			template<typename T>
			bool operator()(PartialView<T> obj, std::source_location location = std::source_location::current()) const
			{
				if (auto value = this->field())
				{
					if (internal::try_parse<T>(value, obj))
						return true;

					this->report(location);
					return false;
				}
				else
					return false;
			}
		};

		template<typename Key, typename Predefined, typename Translator>
		struct Required;

		template<typename Key, typename Translator>
		struct Required<Key, void, Translator> : public Accessor<Key, true>
		{
			using Accessor<Key, true>::Accessor;

			typename Translator::EnumType operator()(std::source_location location = std::source_location::current()) const
			{
				typename Translator::IndexType index;
				if (internal::try_parse<typename Translator::IndexType>(this->field(), index))
				{
					try
					{
						return std::move(Translator::val(index));
					}
					catch (const std::out_of_range&)
					{
						this->report(location, index);
					}
				}
				else
					this->report(location);

				throw Error(ErrorCode::UnreachableCode);
			}
		};

		template<typename Key, typename Predefined>
		struct Required<Key, Predefined, void> : public Accessor<Key, true>
		{
			using Accessor<Key, true>::Accessor;

			Predefined operator()(std::source_location location = std::source_location::current()) const
			{
				if constexpr (std::is_same_v<Predefined, TOMLNode>)
				{
					if (auto value = this->field())
						return std::move(value);
				}
				else
				{
					Predefined obj;
					if (internal::try_parse<Predefined>(this->field(), obj))
						return obj;
				}

				this->report(location);

				throw Error(ErrorCode::UnreachableCode);
			}
		};

		template<typename Key>
		struct Required<Key, TOMLArray, void> : public Accessor<Key, true>
		{
			using Accessor<Key, true>::Accessor;

			TOMLArray operator()(std::source_location location = std::source_location::current()) const
			{
				TOMLArray obj;
				if (internal::try_parse<TOMLArray>(this->field(), obj))
					return obj;

				this->report(location);

				throw Error(ErrorCode::UnreachableCode);
			}

			struct Restriction
			{
				size_t min_size = 0;
				size_t max_size = nmax<size_t>();
			};

			TOMLArray operator()(Restriction restriction, std::source_location location = std::source_location::current()) const
			{
				TOMLArray obj;
				if (internal::try_parse<TOMLArray>(this->field(), obj))
				{
					if (obj->size() >= restriction.min_size && obj->size() <= restriction.max_size)
						return obj;
					else
					{
						this->report(location, internal::fail_causes::FAILED_RESTRICTION,
							DeferredStringList{ "-> out of range [", std::to_string(restriction.min_size), ", ", std::to_string(restriction.max_size), "]"});
					}
				}

				this->report(location);

				throw Error(ErrorCode::UnreachableCode);
			}
		};

		template<typename Key>
		struct Required<Key, void, void> : public Accessor<Key, true>
		{
			using Accessor<Key, true>::Accessor;

			template<typename T>
			void operator()(T& obj, std::source_location location = std::source_location::current()) const
			{
				if constexpr (std::is_same_v<T, TOMLNode>)
				{
					if (auto value = this->field())
					{
						obj = std::move(value);
						return;
					}
				}
				else
				{
					if (internal::try_parse<T>(this->field(), obj))
						return;
				}

				this->report(location);

				throw Error(ErrorCode::UnreachableCode);
			}
		};

		template<typename Translator>
		class TranslateBroker
		{
			const Parser& parser;

		public:
			TranslateBroker(const Parser& parser) : parser(parser) {}

			template<typename Key>
			Defaulted<Key, void, Translator> defaulted(Key key) const { return Defaulted<Key, void, Translator>(parser, key); }

			template<typename Key>
			Optional<Key, void, Translator> optional(Key key) const { return Optional<Key, void, Translator>(parser, key); }

			template<typename Key>
			Required<Key, void, Translator> required(Key key) const { return Required<Key, void, Translator>(parser, key); }
		};

	public:
		explicit Parser(TOMLNode node, const DeferredStringParam& log_suffix = {}, ErrorCode error_code = ErrorCode::LoadAsset, bool fatal = false)
			: node(node), log_suffix(log_suffix), error_code(error_code), fatal(fatal) {}

		Parser(TOMLNode node, DeferredStringParam&& log_suffix, ErrorCode error_code = ErrorCode::LoadAsset, bool fatal = false)
			: node(node), log_suffix(std::move(log_suffix)), error_code(error_code), fatal(fatal) {}

		// TODO v7 once CTOMLNode is added, use const toml::parse_result& instead
		explicit Parser(toml::parse_result& toml, const DeferredStringParam& log_suffix = {}, ErrorCode error_code = ErrorCode::LoadAsset, bool fatal = false)
			: node((TOMLNode)toml), log_suffix(log_suffix), error_code(error_code), fatal(fatal) {}

		Parser(toml::parse_result& toml, DeferredStringParam&& log_suffix, ErrorCode error_code = ErrorCode::LoadAsset, bool fatal = false)
			: node((TOMLNode)toml), log_suffix(std::move(log_suffix)), error_code(error_code), fatal(fatal) {}

		template<typename T = void, typename Key>
		Defaulted<Key, T, void> defaulted(Key key) const { return Defaulted<Key, T, void>(*this, key); }

		template<typename T = void, typename Key>
		Optional<Key, T, void> optional(Key key) const { return Optional<Key, T, void>(*this, key); }
		
		template<typename T = void, typename Key>
		Required<Key, T, void> required(Key key) const { return Required<Key, T, void>(*this, key); }

		template<typename Translator>
		TranslateBroker<Translator> translate() const { return TranslateBroker<Translator>(*this); }

		template<typename Key>
		TOMLNode field(Key key) const
		{
			if constexpr (std::is_same_v<Key, NoKey>)
				return node;
			else
				return node[internal::translate_key(key)];
		}
	};
}
