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
	}

	class Parser
	{
		TOMLNode node;
		DeferredStringParam log_suffix;
		ErrorCode error_code;
		bool fatal;

		// TODO v7 refactor common parsing logic
		// TODO v7 simplify logging

		template<typename Key>
		DeferredStringList get_message(Key key) const
		{
			if constexpr (std::is_same_v<Key, NoKey>)
				return DeferredStringList{ "cannot parse field", log_suffix.empty() ? "" : " " } << log_suffix;
			else
				return DeferredStringList{ "cannot parse ", key_string(key), " field", log_suffix.empty() ? "" : " " } << log_suffix;
		}

		template<typename Key>
		DeferredStringList get_message(Key key, DeferredStringList&& restriction_msg) const
		{
			if constexpr (std::is_same_v<Key, NoKey>)
				return DeferredStringList{ "cannot parse field " } << std::move(restriction_msg) << DeferredStringList{ log_suffix.empty() ? "" : " " } << log_suffix;
			else
				return DeferredStringList{ "cannot parse ", key_string(key), " field " } << std::move(restriction_msg) << DeferredStringList{ log_suffix.empty() ? "" : " " } << log_suffix;
		}

		template<typename Key>
		void log_warning(Key key, std::source_location location) const
		{
			internal::log_context_at_level(LogLevel::Warning, get_message(key), location);
		}

		template<typename Key>
		void log_warning(Key key, DeferredStringList&& restriction_msg, std::source_location location) const
		{
			internal::log_context_at_level(LogLevel::Warning, get_message(key, std::move(restriction_msg)), location);
		}

		template<typename Key>
		void log_error(Key key, std::source_location location) const
		{
			internal::log_context_at_level(fatal ? LogLevel::Fatal : LogLevel::Error, get_message(key), location);
		}

		template<typename Key>
		void log_error(Key key, DeferredStringList&& restriction_msg, std::source_location location) const
		{
			internal::log_context_at_level(fatal ? LogLevel::Fatal : LogLevel::Error, get_message(key, std::move(restriction_msg)), location);
		}

		template<typename Key, typename Index>
		DeferredStringList get_message(Key key, Index e) const
		{
			if constexpr (std::is_same_v<Key, NoKey>)
				return DeferredStringList{ "unrecognized enum (", std::to_string(e), ")", log_suffix.empty() ? "" : " " } << log_suffix;
			else
				return DeferredStringList{ "unrecognized ", key_string(key), " enum (", std::to_string(e), ")", log_suffix.empty() ? "" : " " } << log_suffix;
		}

		template<typename Key, typename Index>
		void log_warning(Key key, Index e, std::source_location location) const
		{
			internal::log_context_at_level(LogLevel::Warning, get_message(key, e), location);
		}

		template<typename Key, typename Index>
		void log_error(Key key, Index e, std::source_location location) const
		{
			internal::log_context_at_level(fatal ? LogLevel::Fatal : LogLevel::Error, get_message(key, e), location);
		}

		template<typename Key, typename Predefined, typename Translator>
		class Defaulted;

		template<typename Key, typename Translator>
		class Defaulted<Key, void, Translator>
		{
			const Parser& parser;
			Key key;

		public:
			Defaulted(const Parser& parser, Key key) : parser(parser), key(key) {}

			typename Translator::EnumType operator()(typename Translator::EnumType def = Translator::val(), std::source_location location = std::source_location::current()) const
			{
				if (auto value = parser.field(key))
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
							parser.log_warning(key, index, location);
						}
					}
					else
						parser.log_warning(key, location);
				}

				return std::move(def);
			}
		};

		template<typename Key, typename Predefined>
		class Defaulted<Key, Predefined, void>
		{
			static_assert(!std::is_same_v<Predefined, TOMLNode>);
			static_assert(!std::is_same_v<Predefined, TOMLArray>);

			const Parser& parser;
			Key key;

		public:
			Defaulted(const Parser& parser, Key key) : parser(parser), key(key) {}

			Predefined operator()(Predefined def = Predefined(), std::source_location location = std::source_location::current()) const
			{
				if (auto value = parser.field(key))
				{
					if (!internal::try_parse<Predefined>(value, def))
						parser.log_warning(key, location);
				}

				return std::move(def);
			}
		};

		template<typename Key>
		class Defaulted<Key, void, void>
		{
			const Parser& parser;
			Key key;

		public:
			Defaulted(const Parser& parser, Key key) : parser(parser), key(key) {}

			template<typename T>
			T operator()(T def, std::source_location location = std::source_location::current()) const
			{
				static_assert(!std::is_same_v<T, TOMLNode>);
				static_assert(!std::is_same_v<T, TOMLArray>);

				if (auto value = parser.field(key))
				{
					if (!internal::try_parse<T>(value, def))
						parser.log_warning(key, location);
				}

				return std::move(def);
			}
		};

		template<typename Key, typename Predefined, typename Translator>
		struct Optional;

		template<typename Key, typename Translator>
		struct Optional<Key, void, Translator>
		{
			const Parser& parser;
			Key key;

		public:
			Optional(const Parser& parser, Key key) : parser(parser), key(key) {}

			bool operator()(typename Translator::EnumType& obj, std::source_location location = std::source_location::current()) const
			{
				if (auto value = parser.field(key))
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
							parser.log_warning(key, index, location);
						}
					}
					else
						parser.log_warning(key, location);
				}

				return false;
			}

			std::optional<typename Translator::EnumType> operator()(std::source_location location = std::source_location::current()) const
			{
				if (auto value = parser.field(key))
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
							parser.log_warning(key, index, location);
						}
					}
					else
						parser.log_warning(key, location);
				}

				return std::nullopt;
			}
		};

		template<typename Key, typename Predefined>
		struct Optional<Key, Predefined, void>
		{
			const Parser& parser;
			Key key;

		public:
			Optional(const Parser& parser, Key key) : parser(parser), key(key) {}

			std::optional<Predefined> operator()(std::source_location location = std::source_location::current()) const
			{
				if (auto value = parser.field(key))
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

					parser.log_warning(key, location);
				}
				return std::nullopt;
			}
		};

		template<typename Key>
		struct Optional<Key, TOMLArray, void>
		{
			const Parser& parser;
			Key key;

		public:
			Optional(const Parser& parser, Key key) : parser(parser), key(key) {}

			TOMLArray operator()(std::source_location location = std::source_location::current()) const
			{
				if (auto value = parser.field(key))
				{
					TOMLArray obj;
					if (internal::try_parse<TOMLArray>(value, obj))
						return obj;

					parser.log_warning(key, location);
				}
				return nullptr;
			}
		};

		template<typename Key>
		struct Optional<Key, void, void>
		{
			const Parser& parser;
			Key key;

		public:
			Optional(const Parser& parser, Key key) : parser(parser), key(key) {}

			template<typename T>
			bool operator()(T& obj, std::source_location location = std::source_location::current()) const
			{
				if (auto value = parser.field(key))
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

					parser.log_warning(key, location);
					return false;
				}
				else
					return false;
			}

			template<typename T>
			bool operator()(std::optional<T>& obj, std::source_location location = std::source_location::current()) const
			{
				obj = std::nullopt;
				if (auto value = parser.field(key))
				{
					T o;
					if (internal::try_parse<T>(value, o))
					{
						obj = std::move(o);
						return true;
					}

					parser.log_warning(key, location);
					return false;
				}
				else
					return false;
			}

			bool operator()(TOMLNode& obj) const
			{
				if (auto value = parser.field(key))
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
				if (auto value = parser.field(key))
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
				if (auto value = parser.field(key))
				{
					if (internal::try_parse<T>(value, obj))
						return true;
					else
					{
						parser.log_warning(key, location);
						return false;
					}
				}
				else
					return false;
			}
		};

		template<typename Key, typename Predefined, typename Translator>
		struct Required;

		template<typename Key, typename Translator>
		struct Required<Key, void, Translator>
		{
			const Parser& parser;
			Key key;

		public:
			Required(const Parser& parser, Key key) : parser(parser), key(key) {}
			
			typename Translator::EnumType operator()(std::source_location location = std::source_location::current()) const
			{
				typename Translator::IndexType index;
				if (internal::try_parse<typename Translator::IndexType>(parser.field(key), index))
				{
					try
					{
						return std::move(Translator::val(index));
					}
					catch (const std::out_of_range&)
					{
						parser.log_error(key, index, location);
					}
				}
				else
					parser.log_error(key, location);
				throw Error(parser.error_code);
			}
		};

		template<typename Key, typename Predefined>
		struct Required<Key, Predefined, void>
		{
			const Parser& parser;
			Key key;

		public:
			Required(const Parser& parser, Key key) : parser(parser), key(key) {}

			Predefined operator()(std::source_location location = std::source_location::current()) const
			{
				if constexpr (std::is_same_v<Predefined, TOMLNode>)
				{
					if (auto value = parser.field(key))
						return std::move(value);
				}
				else
				{
					Predefined obj;
					if (internal::try_parse<Predefined>(parser.field(key), obj))
						return obj;
				}

				parser.log_error(key, location);
				throw Error(parser.error_code);
			}
		};

		template<typename Key>
		struct Required<Key, TOMLArray, void>
		{
			const Parser& parser;
			Key key;

		public:
			Required(const Parser& parser, Key key) : parser(parser), key(key) {}

			TOMLArray operator()(std::source_location location = std::source_location::current()) const
			{
				TOMLArray obj;
				if (internal::try_parse<TOMLArray>(parser.field(key), obj))
					return obj;

				parser.log_error(key, location);
				throw Error(parser.error_code);
			}

			struct Restriction
			{
				size_t min_size = 0;
				size_t max_size = nmax<size_t>();
			};

			TOMLArray operator()(Restriction restriction, std::source_location location = std::source_location::current()) const
			{
				TOMLArray obj;
				if (internal::try_parse<TOMLArray>(parser.field(key), obj))
				{
					if (obj->size() >= restriction.min_size && obj->size() <= restriction.max_size)
						return obj;
					else
					{
						parser.log_error(key, DeferredStringList{ "-> out of range [", std::to_string(restriction.min_size), ", ", std::to_string(restriction.max_size), "]"}, location);
					}
				}

				parser.log_error(key, location);
				throw Error(parser.error_code);
			}
		};

		template<typename Key>
		struct Required<Key, void, void>
		{
			const Parser& parser;
			Key key;

		public:
			Required(const Parser& parser, Key key) : parser(parser), key(key) {}

			template<typename T>
			void operator()(T& obj, std::source_location location = std::source_location::current()) const
			{
				if constexpr (std::is_same_v<T, TOMLNode>)
				{
					if (auto value = parser.field(key))
					{
						obj = std::move(value);
						return;
					}
				}
				else
				{
					if (internal::try_parse<T>(parser.field(key), obj))
						return;
				}

				parser.log_error(key, location);
				throw Error(parser.error_code);
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
