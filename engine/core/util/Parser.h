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
			constexpr const char* INVALID_ARRAY_SIZE = "invalid size of array";
			constexpr const char* FAILED_RESTRICTION = "failed restriction for";
		}
	}

	struct NullValidator
	{
	};

	template<typename Type, typename Arg>
	concept Validates = requires(const Type& t, const Arg& arg) {
		{ std::invoke(t, arg) } -> std::same_as<bool>;
		{ t.elaboration(arg) } -> std::same_as<DeferredStringList>;
	};

	template<typename Arg, typename BoolFunc, typename StrFunc>
	struct SingleValidator
	{
		BoolFunc validator;
		StrFunc elaborator;

		bool operator()(const Arg& arg) const { return std::invoke(validator, arg); }
		DeferredStringList elaboration(const Arg& arg) const { return std::invoke(elaborator, arg); }
	};

	template<typename Arg, typename BoolFunc, typename StrFunc>
	SingleValidator<Arg, BoolFunc, StrFunc> make_single_validator(BoolFunc&& validator, StrFunc&& elaborator) requires (Validates<SingleValidator<Arg, BoolFunc, StrFunc>, Arg>)
	{
		return { .validator = std::forward<BoolFunc>(validator), .elaborator = std::forward<StrFunc>(elaborator) };
	}

	template<typename Arg1, typename BoolFunc1, typename StrFunc1, typename Arg2, typename BoolFunc2, typename StrFunc2>
	struct DualValidator
	{
		static_assert(!std::is_same_v<Arg1, Arg2>);

		SingleValidator<Arg1, BoolFunc1, StrFunc1> v1;
		SingleValidator<Arg2, BoolFunc2, StrFunc2> v2;

		bool operator()(const Arg1& arg) const { return std::invoke(v1.validator, arg); }
		DeferredStringList elaboration(const Arg1& arg) const { return std::invoke(v1.elaborator, arg); }
		bool operator()(const Arg2& arg) const { return std::invoke(v2.validator, arg); }
		DeferredStringList elaboration(const Arg2& arg) const { return std::invoke(v2.elaborator, arg); }
	};

	template<typename Arg1, typename BoolFunc1, typename StrFunc1, typename Arg2, typename BoolFunc2, typename StrFunc2>
	DualValidator<Arg1, BoolFunc1, StrFunc1, Arg2, BoolFunc2, StrFunc2> make_dual_validator(SingleValidator<Arg1, BoolFunc1, StrFunc1>&& v1, SingleValidator<Arg2, BoolFunc2, StrFunc2>&& v2)
	{
		return { std::forward<SingleValidator<Arg1, BoolFunc1, StrFunc1>>(v1), std::forward<SingleValidator<Arg2, BoolFunc2, StrFunc2>>(v2) };
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

		template<typename Key, bool Throws, typename Validator>
		struct Accessor
		{
		protected:
			const Parser& parser;
			Key key;
			Validator validator;

		public:
			Accessor(const Parser& parser, Key key, Validator&& validator) : parser(parser), key(key), validator(std::move(validator)) {}

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

			template<bool TypeFallback>
			TOMLArray parse_array(std::source_location location) const
			{
				if (auto value = field())
				{
					TOMLArray obj;
					if (internal::try_parse<TOMLArray>(value, obj))
					{
						if constexpr (Validates<Validator, TOMLArray>)
						{
							if (validator(obj))
								return obj;
							else
								this->report(location, internal::fail_causes::FAILED_RESTRICTION, validator.elaboration(obj));
						}
						else
							return obj;
					}

					if constexpr (!TypeFallback)
						this->report(location);
				}
				else if constexpr (Throws)
					this->report(location);

				return nullptr;
			}
		};

		template<typename Key, typename Predefined, typename Translator, typename Validator>
		struct Defaulted;

		template<typename Key, typename Translator, typename Validator>
		struct Defaulted<Key, void, Translator, Validator> : public Accessor<Key, false, Validator>
		{
			using Accessor<Key, false, Validator>::Accessor;

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

		template<typename Key, typename Predefined, typename Validator>
		struct Defaulted<Key, Predefined, void, Validator> : public Accessor<Key, false, Validator>
		{
			using Accessor<Key, false, Validator>::Accessor;

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

		template<typename Key, typename T, typename Validator>
		struct Defaulted<Key, std::vector<T>, void, Validator> : public Accessor<Key, false, Validator>
		{
			using Accessor<Key, false, Validator>::Accessor;

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
								obj.push_back(std::move(el));
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

		template<typename Key, typename Validator>
		struct Defaulted<Key, void, void, Validator> : public Accessor<Key, false, Validator>
		{
			using Accessor<Key, false, Validator>::Accessor;

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

		template<typename Key, typename Predefined, typename Translator, typename Validator, bool TypeFallback>
		struct Optional;

		template<typename Key, typename Translator, typename Validator, bool TypeFallback>
		struct Optional<Key, void, Translator, Validator, TypeFallback> : public Accessor<Key, false, Validator>
		{
			using Accessor<Key, false, Validator>::Accessor;

			bool operator()(typename Translator::EnumType& obj, std::source_location location = std::source_location::current()) const
			{
				if (auto value = this->field())
				{
					typename Translator::IndexType index;
					if (internal::try_parse<typename Translator::IndexType>(value, index))
					{
						try
						{
							obj = Translator::val(index);
							return true;
						}
						catch (const std::out_of_range&)
						{
							this->report(location, index);
						}
					}
					else if constexpr (!TypeFallback)
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
							return Translator::val(index);
						}
						catch (const std::out_of_range&)
						{
							this->report(location, index);
						}
					}
					else if constexpr (!TypeFallback)
						this->report(location);
				}

				return std::nullopt;
			}
		};

		template<typename Key, typename Predefined, typename Validator, bool TypeFallback>
		struct Optional<Key, Predefined, void, Validator, TypeFallback> : public Accessor<Key, false, Validator>
		{
			using Accessor<Key, false, Validator>::Accessor;

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

					if constexpr (!TypeFallback)
						this->report(location);
				}
				return std::nullopt;
			}
		};

		template<typename Key, typename Validator, bool TypeFallback>
		struct Optional<Key, TOMLArray, void, Validator, TypeFallback> : public Accessor<Key, false, Validator>
		{
			using Accessor<Key, false, Validator>::Accessor;

			TOMLArray operator()(std::source_location location = std::source_location::current()) const
			{
				return this->parse_array<TypeFallback>(location);
			}
		};

		template<typename Key, typename Validator, bool TypeFallback>
		struct Optional<Key, void, void, Validator, TypeFallback> : public Accessor<Key, false, Validator>
		{
			using Accessor<Key, false, Validator>::Accessor;

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

					if constexpr (!TypeFallback)
						this->report(location);
				}	
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

					if constexpr (!TypeFallback)
						this->report(location);
				}
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
								obj.push_back(std::move(el));
							else
								this->report(location, internal::fail_causes::INVALID_ARRAY_ELEMENT);
						}

						return true;
					}

					if constexpr (!TypeFallback)
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

					if constexpr (!TypeFallback)
						this->report(location);
				}
				return false;
			}

			std::optional<Parser> subparser() const;
		};

		template<typename Key, typename Predefined, typename Translator, typename Validator>
		struct Required;

		template<typename Key, typename Translator, typename Validator>
		struct Required<Key, void, Translator, Validator> : public Accessor<Key, true, Validator>
		{
			using Accessor<Key, true, Validator>::Accessor;

			typename Translator::EnumType operator()(std::source_location location = std::source_location::current()) const
			{
				typename Translator::IndexType index;
				if (internal::try_parse<typename Translator::IndexType>(this->field(), index))
				{
					try
					{
						return Translator::val(index);
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

		template<typename Key, typename Predefined, typename Validator>
		struct Required<Key, Predefined, void, Validator> : public Accessor<Key, true, Validator>
		{
			using Accessor<Key, true, Validator>::Accessor;

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
					{
						if constexpr (Validates<Validator, Predefined>)
						{
							if (this->validator(obj))
								return obj;
							else
								this->report(location, internal::fail_causes::CANNOT_PARSE, this->validator.elaboration(obj));
						}
						else
							return obj;
					}
				}

				this->report(location);

				throw Error(ErrorCode::UnreachableCode);
			}
		};

		template<typename Key, typename Validator>
		struct Required<Key, TOMLArray, void, Validator> : public Accessor<Key, true, Validator>
		{
			using Accessor<Key, true, Validator>::Accessor;

			TOMLArray operator()(std::source_location location = std::source_location::current()) const
			{
				return this->parse_array<false>(location);
			}
		};

		template<typename Key, typename T, size_t N, typename Validator>
		struct Required<Key, std::array<T, N>, void, Validator> : public Accessor<Key, true, Validator>
		{
			static_assert(N > 1);

			using Accessor<Key, true, Validator>::Accessor;

			std::array<T, N> operator()(std::source_location location = std::source_location::current()) const
			{
				TOMLArray arr;
				if (internal::try_parse<TOMLArray>(this->field(), arr))
				{
					if (arr->size() != N)
						this->report(location, internal::fail_causes::INVALID_ARRAY_SIZE,
							DeferredStringList{ "expected (", std::to_string(N), ") but parsed (", std::to_string(arr->size()), ")" });

					std::array<T, N> obj;
					for (size_t i = 0; i < N; ++i)
					{
						T el;
						if (internal::try_parse<T>((TOMLNode)*arr->get(i), el))
						{
							if constexpr (Validates<Validator, T>)
							{
								if (this->validator(el))
									obj.at(i) = std::move(el);
								else
									this->report(location, internal::fail_causes::INVALID_ARRAY_ELEMENT, this->validator.elaboration(el));
							}
							else
								obj.at(i) = std::move(el);
						}
						else
							this->report(location, internal::fail_causes::INVALID_ARRAY_ELEMENT);
					}

					return obj;
				}

				this->report(location);

				throw Error(ErrorCode::UnreachableCode);
			}
		};

		template<typename Key, typename Validator>
		struct Required<Key, void, void, Validator> : public Accessor<Key, true, Validator>
		{
			using Accessor<Key, true, Validator>::Accessor;

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

			template<typename Validator = NullValidator, typename Key>
			Defaulted<Key, void, Translator, Validator> defaulted(Key key, Validator&& validator = {}) const
				{ return Defaulted<Key, void, Translator, Validator>(parser, key, std::move(validator)); }

			template<bool TypeFallback = false, typename Validator = NullValidator, typename Key>
			Optional<Key, void, Translator, Validator, TypeFallback> optional(Key key, Validator&& validator = {}) const
				{ return Optional<Key, void, Translator, Validator, TypeFallback>(parser, key, std::move(validator)); }

			template<typename Validator = NullValidator, typename Key>
			Required<Key, void, Translator, Validator> required(Key key, Validator&& validator = {}) const
				{ return Required<Key, void, Translator, Validator>(parser, key, std::move(validator)); }
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

		template<typename T = void, typename Validator = NullValidator, typename Key>
		Defaulted<Key, T, void, Validator> defaulted(Key key, Validator&& validator = {}) const
			{ return Defaulted<Key, T, void, Validator>(*this, key, std::move(validator)); }

		template<typename T = void, bool TypeFallback = false, typename Validator = NullValidator, typename Key>
		Optional<Key, T, void, Validator, TypeFallback> optional(Key key, Validator&& validator = {}) const
			{ return Optional<Key, T, void, Validator, TypeFallback>(*this, key, std::move(validator)); }
		
		template<typename T = void, typename Validator = NullValidator, typename Key>
		Required<Key, T, void, Validator> required(Key key, Validator&& validator = {}) const
			{ return Required<Key, T, void, Validator>(*this, key, std::move(validator)); }

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

	template<typename Key, typename Validator, bool TypeFallback>
	std::optional<Parser> Parser::Optional<Key, void, void, Validator, TypeFallback>::subparser() const
	{
		if (auto value = this->field())
			return Parser(value, this->parser.log_suffix, this->parser.error_code, this->parser.fatal);
		else
			return std::nullopt;
	}
}
