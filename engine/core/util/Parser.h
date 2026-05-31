#pragma once

#include "external/TOML.h"
#include "external/GL.h"
#include "core/util/DeferredStringParam.h"
#include "core/util/LogLevel.h"

#include "assets/TranslateKey.h"

#include <array>
#include <cstdint>
#include <string>
#include <algorithm>
#include <source_location>

namespace oly::assets
{
	extern detail::Key NO_KEY;

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

		template<typename T> requires (std::is_enum_v<T>)
		bool try_parse(TOMLNode node, T& obj)
		{
			int value = static_cast<int>(obj);
			if (!try_parse(node, value))
				return false;
			obj = static_cast<T>(value);
			return true;
		}

		extern void log_context_at_level(LogLevel level, const DeferredStringParam& msg, std::source_location location);

		namespace fail_causes
		{
			constexpr const char* CANNOT_PARSE = "cannot parse";
			constexpr const char* MISSING_FIELD = "missing";
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

		DeferredStringList build_message(detail::Key key, const StringParam& cause = internal::fail_causes::CANNOT_PARSE, DeferredStringList&& elaboration = {}) const
		{
			DeferredStringList list{ cause.transfer(), " " };
			
			if (key != NO_KEY)
				list << detail::decode_key(key) << " field";
			else
				list << "field";
			
			if (!elaboration.empty())
				list << " " << std::move(elaboration);

			if (!log_suffix.empty())
				list << " " << log_suffix;

			return list;
		}

		void log_at_level(LogLevel level, std::source_location location, detail::Key key,
			const StringParam& cause = internal::fail_causes::CANNOT_PARSE, DeferredStringList&& elaboration = {}) const
		{
			if ((int)level >= (int)LogLevel::Error && fatal)
				level = LogLevel::Fatal;
			internal::log_context_at_level(level, build_message(key, cause.transfer(), std::move(elaboration)), location);
		}

		template<typename Index>
		void log_at_level(LogLevel level, std::source_location location, detail::Key key, Index e) const
		{
			if ((int)level >= (int)LogLevel::Error && fatal)
				level = LogLevel::Fatal;
			internal::log_context_at_level(level, build_message(key, DeferredStringList{ "unrecognized enum (", std::to_string(e), ") in"}.str()), location);
		}

	public:
		template<bool Throws, typename Validator>
		struct Accessor
		{
		protected:
			const Parser& parser;
			detail::Key key;
			Validator validator;

		public:
			Accessor(const Parser& parser, detail::Key key, Validator&& validator) : parser(parser), key(key), validator(std::move(validator)) {}

		protected:
			template<bool Throw = Throws>
			void report(std::source_location location, const StringParam& cause, DeferredStringList&& elaboration = {}) const
			{
				parser.log_at_level(Throws ? LogLevel::Error : LogLevel::Warning, location, key, cause.transfer(), std::move(elaboration));
				if constexpr (Throws)
					throw Error(parser.error_code);
			}

			template<typename Index, bool Throw = Throws>
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

			template<typename T>
			void validate(const T& obj, std::source_location location) const
			{
				if constexpr (Validates<Validator, T>)
				{
					if (!validator(obj))
						report<true>(location, internal::fail_causes::CANNOT_PARSE, validator.elaboration(obj));
				}
			}

			std::optional<TOMLNode> optional_node(std::source_location location) const
			{
				if (auto value = field())
				{
					validate(value, location);
					return value;
				}
				else if constexpr (Throws)
					report(location, internal::fail_causes::MISSING_FIELD);
				return std::nullopt;
			}

			template<bool TypeFallback>
			TOMLArray parse_array(std::source_location location) const
			{
				if (auto value = field())
				{
					TOMLArray obj;
					if (internal::try_parse<TOMLArray>(value, obj))
					{
						validate(obj, location);
						return obj;
					}
					else if constexpr (!TypeFallback)
						report(location, internal::fail_causes::CANNOT_PARSE);
				}
				else if constexpr (Throws)
					report(location, internal::fail_causes::MISSING_FIELD);

				return nullptr;
			}

			template<bool TypeFallback, typename Translator>
			std::optional<typename Translator::EnumType> parse_enum(std::source_location location, typename Translator::EnumType def = Translator::val()) const
			{
				if (auto value = field())
				{
					typename Translator::IndexType index;
					if (internal::try_parse<typename Translator::IndexType>(value, index))
					{
						try
						{
							typename Translator::EnumType e = Translator::val(index, def);
							validate(e, location);
							return e;
						}
						catch (const std::out_of_range&)
						{
							report(location, index);
						}
					}
					else if constexpr (!TypeFallback)
						report(location, internal::fail_causes::CANNOT_PARSE);
				}
				else if constexpr (Throws)
					report(location, internal::fail_causes::MISSING_FIELD);

				return std::nullopt;
			}

			template<bool TypeFallback, typename T, typename U = T>
			bool parse_value(T& def, std::source_location location) const
			{
				static_assert(!std::is_same_v<T, TOMLNode>);
				static_assert(!std::is_same_v<T, TOMLArray>);

				if (auto value = this->field())
				{
					if (internal::try_parse<U>(value, def))
					{
						validate(def, location);
						return true;
					}
					else if constexpr (!TypeFallback)
						this->report(location, internal::fail_causes::CANNOT_PARSE);
				}
				else if constexpr (Throws)
					report(location, internal::fail_causes::MISSING_FIELD);

				return false;
			}

			template<bool TypeFallback, typename T>
			std::optional<T> parse_value(std::source_location location) const
			{
				T obj = T();
				return parse_value<TypeFallback, T>(obj, location) ? std::make_optional<T>(std::move(obj)) : std::nullopt;
			}

			template<typename T, typename Array>
			bool fill_array(TOMLArray arr, Array& array, std::source_location location) const
			{
				for (size_t i = 0; i < arr->size(); ++i)
				{
					T el;
					if (internal::try_parse<T>((TOMLNode)*arr->get(i), el))
					{
						validate(el, location);
						array[i] = std::move(el);
					}
					else
					{
						this->report(location, internal::fail_causes::INVALID_ARRAY_ELEMENT);
						return false;
					}
				}

				return true;
			}

			template<bool TypeFallback, bool Dynamic, typename T, typename Array>
			Array build_list(std::source_location location) const
			{
				if (auto arr = parse_array<TypeFallback>(location))
				{
					Array array{};

					if constexpr (Dynamic)
						array.resize(arr->size());
					else
					{
						if (arr->size() != array.size())
							report(location, internal::fail_causes::INVALID_ARRAY_SIZE,
								DeferredStringList{ "expected (", std::to_string(array.size()), ") but parsed (", std::to_string(arr->size()), ")" });
					}

					if (fill_array<T>(arr, array, location))
						return std::move(array);
				}

				return {};
			}

			template<bool TypeFallback, typename T>
			std::vector<T> build_vector(std::source_location location) const
			{
				return build_list<TypeFallback, true, T, std::vector<T>>(location);
			}

			template<bool TypeFallback, typename T, size_t N>
			std::array<T, N> build_array(std::source_location location) const
			{
				return build_list<TypeFallback, false, T, std::array<T, N>>(location);
			}
		};

		template<typename Predefined, typename Translator, typename Validator>
		struct Defaulted;

		template<typename Translator, typename Validator>
		struct Defaulted<void, Translator, Validator> : public Accessor<false, Validator>
		{
			using Accessor<false, Validator>::Accessor;

			typename Translator::EnumType operator()(typename Translator::EnumType def = Translator::val(), std::source_location location = std::source_location::current()) const
			{
				return get_from_optional(this->parse_enum<true, Translator>(location, def), std::move(def));
			}
		};

		template<typename Predefined, typename Validator>
		struct Defaulted<Predefined, void, Validator> : public Accessor<false, Validator>
		{
			using Accessor<false, Validator>::Accessor;

			Predefined operator()(Predefined&& def = Predefined(), std::source_location location = std::source_location::current()) const
			{
				this->parse_value<false>(def, location);
				return std::move(def);
			}
		};

		template<typename T, typename Validator>
		struct Defaulted<std::vector<T>, void, Validator> : public Accessor<false, Validator>
		{
			using Accessor<false, Validator>::Accessor;

			std::vector<T> operator()(std::source_location location = std::source_location::current()) const
			{
				return this->build_vector<false, T>(location);
			}
		};

		template<typename Validator>
		struct Defaulted<void, void, Validator> : public Accessor<false, Validator>
		{
			using Accessor<false, Validator>::Accessor;

			template<typename T>
			T operator()(T&& def = T(), std::source_location location = std::source_location::current()) const
			{
				this->parse_value<false>(def, location);
				return std::move(def);
			}
		};

		template<typename Predefined, typename Translator, typename Validator, bool TypeFallback>
		struct Optional;

		template<typename Translator, typename Validator, bool TypeFallback>
		struct Optional<void, Translator, Validator, TypeFallback> : public Accessor<false, Validator>
		{
			using Accessor<false, Validator>::Accessor;

			bool operator()(typename Translator::EnumType& obj, std::source_location location = std::source_location::current()) const
			{
				return set_from_optional(obj, this->parse_enum<TypeFallback, Translator>(location));
			}

			std::optional<typename Translator::EnumType> operator()(std::source_location location = std::source_location::current()) const
			{
				return this->parse_enum<TypeFallback, Translator>(location);
			}
		};

		template<typename Predefined, typename Validator, bool TypeFallback>
		struct Optional<Predefined, void, Validator, TypeFallback> : public Accessor<false, Validator>
		{
			using Accessor<false, Validator>::Accessor;

			std::optional<Predefined> operator()(std::source_location location = std::source_location::current()) const
			{
				return this->parse_value<TypeFallback, Predefined>(location);
			}
		};

		template<typename Validator, bool TypeFallback>
		struct Optional<TOMLNode, void, Validator, TypeFallback> : public Accessor<false, Validator>
		{
			using Accessor<false, Validator>::Accessor;

			std::optional<TOMLNode> operator()(std::source_location location = std::source_location::current()) const
			{
				return this->optional_node(location);
			}
		};

		template<typename Validator, bool TypeFallback>
		struct Optional<TOMLArray, void, Validator, TypeFallback> : public Accessor<false, Validator>
		{
			using Accessor<false, Validator>::Accessor;

			TOMLArray operator()(std::source_location location = std::source_location::current()) const
			{
				return this->parse_array<TypeFallback>(location);
			}
		};

		template<typename Validator, bool TypeFallback>
		struct Optional<void, void, Validator, TypeFallback> : public Accessor<false, Validator>
		{
			using Accessor<false, Validator>::Accessor;

			template<typename T>
			bool operator()(T& obj, std::source_location location = std::source_location::current()) const
			{
				return this->parse_value<TypeFallback>(obj, location);
			}

			template<typename T>
			bool operator()(std::optional<T>& obj, std::source_location location = std::source_location::current()) const
			{
				return (obj = this->parse_value<TypeFallback, T>(location)).has_value();
			}

			template<typename T>
			bool operator()(PartialView<T> obj, std::source_location location = std::source_location::current()) const
			{
				return this->parse_value<TypeFallback, PartialView<T>, T>(obj, location);
			}

			template<typename T>
			bool operator()(std::vector<T>& obj, std::source_location location = std::source_location::current()) const
			{
				return !(obj = this->build_vector<TypeFallback, T>()).empty();
			}

			bool operator()(TOMLNode& obj, std::source_location location = std::source_location::current()) const
			{
				return set_from_optional(obj, this->optional_node(location));
			}

			bool operator()(std::optional<TOMLNode>& obj, std::source_location location = std::source_location::current()) const
			{
				return (obj = this->optional_node(location)).has_value();
			}

			std::optional<Parser> subparser() const;
		};

		template<typename Predefined, typename Translator, typename Validator>
		struct Required;

		template<typename Translator, typename Validator>
		struct Required<void, Translator, Validator> : public Accessor<true, Validator>
		{
			using Accessor<true, Validator>::Accessor;

			typename Translator::EnumType operator()(std::source_location location = std::source_location::current()) const
			{
				return *this->parse_enum<false, Translator>(location);
			}
		};

		template<typename Predefined, typename Validator>
		struct Required<Predefined, void, Validator> : public Accessor<true, Validator>
		{
			using Accessor<true, Validator>::Accessor;

			Predefined operator()(std::source_location location = std::source_location::current()) const
			{
				return *this->parse_value<false, Predefined>(location);
			}
		};

		template<typename Validator>
		struct Required<TOMLNode, void, Validator> : public Accessor<true, Validator>
		{
			using Accessor<true, Validator>::Accessor;

			TOMLNode operator()(std::source_location location = std::source_location::current()) const
			{
				return *this->optional_node(location);
			}
		};

		template<typename Validator>
		struct Required<TOMLArray, void, Validator> : public Accessor<true, Validator>
		{
			using Accessor<true, Validator>::Accessor;

			TOMLArray operator()(std::source_location location = std::source_location::current()) const
			{
				return this->parse_array<false>(location);
			}
		};

		template<typename T, size_t N, typename Validator>
		struct Required<std::array<T, N>, void, Validator> : public Accessor<true, Validator>
		{
			static_assert(N > 1);

			using Accessor<true, Validator>::Accessor;

			std::array<T, N> operator()(std::source_location location = std::source_location::current()) const
			{
				return this->build_array<false, T, N>(location);
			}
		};

		template<typename Validator>
		struct Required<void, void, Validator> : public Accessor<true, Validator>
		{
			using Accessor<true, Validator>::Accessor;

			template<typename T>
			void operator()(T& obj, std::source_location location = std::source_location::current()) const
			{
				this->parse_value<false>(obj, location);
			}

			void operator()(TOMLNode& obj, std::source_location location = std::source_location::current()) const
			{
				obj = *this->optional_node(location);
			}
		};

		explicit Parser(TOMLNode node, const DeferredStringParam& log_suffix = {}, ErrorCode error_code = ErrorCode::LoadAsset, bool fatal = false)
			: node(node), log_suffix(log_suffix), error_code(error_code), fatal(fatal) {}

		Parser(TOMLNode node, DeferredStringParam&& log_suffix, ErrorCode error_code = ErrorCode::LoadAsset, bool fatal = false)
			: node(node), log_suffix(std::move(log_suffix)), error_code(error_code), fatal(fatal) {}

		explicit Parser(const toml::parse_result& toml, const DeferredStringParam& log_suffix = {}, ErrorCode error_code = ErrorCode::LoadAsset, bool fatal = false)
			: node((TOMLNode)toml), log_suffix(log_suffix), error_code(error_code), fatal(fatal) {}

		Parser(const toml::parse_result& toml, DeferredStringParam&& log_suffix, ErrorCode error_code = ErrorCode::LoadAsset, bool fatal = false)
			: node((TOMLNode)toml), log_suffix(std::move(log_suffix)), error_code(error_code), fatal(fatal) {}

		template<typename T = void, typename Validator = NullValidator>
		Defaulted<T, void, Validator> defaulted(detail::Key key, Validator&& validator = {}) const
			{ return Defaulted<T, void, Validator>(*this, key, std::move(validator)); }

		template<typename T = void, bool TypeFallback = false, typename Validator = NullValidator>
		Optional<T, void, Validator, TypeFallback> optional(detail::Key key, Validator&& validator = {}) const
			{ return Optional<T, void, Validator, TypeFallback>(*this, key, std::move(validator)); }
		
		template<typename T = void, typename Validator = NullValidator>
		Required<T, void, Validator> required(detail::Key key, Validator&& validator = {}) const
			{ return Required<T, void, Validator>(*this, key, std::move(validator)); }

		TOMLNode field(detail::Key key) const
		{
			if (key != NO_KEY)
				return node[detail::decode_key(key)];
			else
				return node;
		}
	};

	template<typename Validator, bool TypeFallback>
	std::optional<Parser> Parser::Optional<void, void, Validator, TypeFallback>::subparser() const
	{
		if (auto value = this->field())
			return Parser(value, this->parser.log_suffix, this->parser.error_code, this->parser.fatal);
		else
			return std::nullopt;
	}
}
