#include "JsonTextPatch.h"

#include <algorithm>
#include <cctype>
#include <charconv>
#include <cmath>
#include <cstdint>
#include <limits>
#include <string_view>
#include <unordered_map>

using Json = nlohmann::json;

namespace JsonTextPatch
{
	namespace
	{
		constexpr size_t MaxLineColumns = 140;   // new content stays on one line while its line fits
		constexpr size_t TabColumns = 4;

		// A value of the authored text: what it is and where. Object members keep their keys and where those start.
		struct Node
		{
			enum class Kind : uint8_t { Scalar, Object, Array };

			Kind kind = Kind::Scalar;
			size_t begin = 0;
			size_t end = 0;
			std::vector<std::string> keys;       // objects
			std::vector<size_t> keyBegins;       // objects: offset of each member's key
			std::vector<Node> children;          // member values (objects) or elements (arrays)

			const Node* Find(const std::string& key, size_t* index) const
			{
				for (size_t i = 0; i < keys.size(); ++i)
				{
					if (keys[i] == key)
					{
						*index = i;
						return &children[i];
					}
				}
				return nullptr;
			}
		};

		// Strict enough to locate values; the patched text is validated by a real load afterwards.
		class Parser
		{
		public:
			explicit Parser(const std::string& source) : text(source) {}

			bool Parse(Node& root, std::string& error)
			{
				SkipSpace();
				if (!ParseValue(root))
				{
					error = message + " at offset " + std::to_string(pos);
					return false;
				}
				SkipSpace();
				if (pos != text.size())
				{
					error = "unexpected text after the document at offset " + std::to_string(pos);
					return false;
				}
				return true;
			}

			std::unordered_map<std::string, size_t> keyRank;   // order in which keys first appear in the file

		private:
			const std::string& text;
			size_t pos = 0;
			std::string message;

			bool Fail(const char* what)
			{
				message = what;
				return false;
			}

			bool At(char c) const { return pos < text.size() && text[pos] == c; }

			// Whitespace and comments (the data files may carry // and /* */ comments, ParseDataJson allows them).
			void SkipSpace()
			{
				while (pos < text.size())
				{
					const char c = text[pos];
					if (c == ' ' || c == '\t' || c == '\n' || c == '\r')
					{
						++pos;
					}
					else if (c == '/' && pos + 1 < text.size() && text[pos + 1] == '/')
					{
						const size_t newline = text.find('\n', pos);
						pos = newline == std::string::npos ? text.size() : newline + 1;
					}
					else if (c == '/' && pos + 1 < text.size() && text[pos + 1] == '*')
					{
						const size_t close = text.find("*/", pos + 2);
						pos = close == std::string::npos ? text.size() : close + 2;
					}
					else
					{
						break;
					}
				}
			}

			bool ParseValue(Node& node)
			{
				if (pos >= text.size())
					return Fail("unexpected end of the document");

				node.begin = pos;
				bool ok = false;
				if (At('{'))
					ok = ParseObject(node);
				else if (At('['))
					ok = ParseArray(node);
				else if (At('"'))
					ok = ParseString(nullptr);
				else
					ok = ParseLiteral();
				node.end = pos;
				return ok;
			}

			bool ParseObject(Node& node)
			{
				node.kind = Node::Kind::Object;
				++pos;
				SkipSpace();
				if (At('}'))
				{
					++pos;
					return true;
				}

				while (true)
				{
					SkipSpace();
					if (!At('"'))
						return Fail("expected a key");
					const size_t keyBegin = pos;
					std::string key;
					if (!ParseString(&key))
						return false;
					SkipSpace();
					if (!At(':'))
						return Fail("expected ':'");
					++pos;
					SkipSpace();

					Node child;
					if (!ParseValue(child))
						return false;
					keyRank.emplace(key, keyRank.size());
					node.keys.push_back(std::move(key));
					node.keyBegins.push_back(keyBegin);
					node.children.push_back(std::move(child));

					SkipSpace();
					if (At(','))
					{
						++pos;
						continue;
					}
					if (At('}'))
					{
						++pos;
						return true;
					}
					return Fail("expected ',' or '}'");
				}
			}

			bool ParseArray(Node& node)
			{
				node.kind = Node::Kind::Array;
				++pos;
				SkipSpace();
				if (At(']'))
				{
					++pos;
					return true;
				}

				while (true)
				{
					SkipSpace();
					Node child;
					if (!ParseValue(child))
						return false;
					node.children.push_back(std::move(child));

					SkipSpace();
					if (At(','))
					{
						++pos;
						continue;
					}
					if (At(']'))
					{
						++pos;
						return true;
					}
					return Fail("expected ',' or ']'");
				}
			}

			// Decodes into `out` when given (keys); otherwise only skips.
			bool ParseString(std::string* out)
			{
				++pos;   // opening quote
				while (pos < text.size())
				{
					const char c = text[pos++];
					if (c == '"')
						return true;
					if (c != '\\')
					{
						if (out)
							*out += c;
						continue;
					}
					if (pos >= text.size())
						break;

					const char escaped = text[pos++];
					if (escaped == 'u')
					{
						unsigned code = 0;
						const auto hex = std::from_chars(text.data() + pos, text.data() + std::min(pos + 4, text.size()), code, 16);
						if (hex.ec != std::errc() || hex.ptr != text.data() + pos + 4)
							return Fail("bad \\u escape");
						pos += 4;
						if (out)
							AppendUtf8(*out, code);
						continue;
					}
					if (!out)
						continue;
					switch (escaped)
					{
					case 'n': *out += '\n'; break;
					case 't': *out += '\t'; break;
					case 'r': *out += '\r'; break;
					case 'b': *out += '\b'; break;
					case 'f': *out += '\f'; break;
					default:  *out += escaped; break;   // \" \\ \/
					}
				}
				return Fail("unterminated string");
			}

			static void AppendUtf8(std::string& out, unsigned code)
			{
				if (code < 0x80)
				{
					out += static_cast<char>(code);
				}
				else if (code < 0x800)
				{
					out += static_cast<char>(0xC0 | (code >> 6));
					out += static_cast<char>(0x80 | (code & 0x3F));
				}
				else
				{
					out += static_cast<char>(0xE0 | (code >> 12));
					out += static_cast<char>(0x80 | ((code >> 6) & 0x3F));
					out += static_cast<char>(0x80 | (code & 0x3F));
				}
			}

			bool ParseLiteral()
			{
				const size_t start = pos;
				while (pos < text.size() && (std::isalnum(static_cast<unsigned char>(text[pos])) || text[pos] == '-' || text[pos] == '+' || text[pos] == '.'))
					++pos;
				return pos != start || Fail("unexpected character");
			}
		};

		class Patcher
		{
		public:
			Patcher(const std::string& source, const std::unordered_map<std::string, size_t>& rank, const Minimize& minimizeFn)
				: text(source), keyRank(rank), minimize(minimizeFn)
			{
			}

			void Walk(const Json& base, const Json& edited, const Node& node, Path& path)
			{
				if (base == edited)
					return;

				if (base.is_object() && edited.is_object() && node.kind == Node::Kind::Object)
				{
					WalkObject(base, edited, node, path);
					return;
				}

				if (base.is_array() && edited.is_array() && node.kind == Node::Kind::Array
					&& base.size() == edited.size() && node.children.size() == base.size())
				{
					for (size_t i = 0; i < base.size(); ++i)
					{
						path.push_back(std::to_string(i));
						Walk(base[i], edited[i], node.children[i], path);
						path.pop_back();
					}
					return;
				}

				if (edited.is_primitive())
					Replace(node, FormatScalar(edited, std::string_view(text).substr(node.begin, node.end - node.begin)));
				else
					Replace(node, Format(minimize(path, edited), LineIndent(node.begin)));
			}

			bool Result(std::string& out, std::string& error)
			{
				// Back to front; at one offset the later edit goes first, so the earlier one ends up in front of it.
				std::sort(edits.begin(), edits.end(), [](const Edit& a, const Edit& b)
				{
					return a.begin != b.begin ? a.begin > b.begin : a.order > b.order;
				});

				out = text;
				size_t limit = std::numeric_limits<size_t>::max();
				for (const Edit& edit : edits)
				{
					if (edit.end > limit)
					{
						error = "overlapping edits at offset " + std::to_string(edit.begin);
						return false;
					}
					out.replace(edit.begin, edit.end - edit.begin, edit.text);
					limit = edit.begin;
				}
				return true;
			}

		private:
			struct Edit
			{
				size_t begin = 0;
				size_t end = 0;
				std::string text;
				size_t order = 0;
			};

			const std::string& text;
			const std::unordered_map<std::string, size_t>& keyRank;
			const Minimize& minimize;
			std::vector<Edit> edits;

			void Replace(const Node& node, std::string replacement)
			{
				edits.push_back({ node.begin, node.end, std::move(replacement), edits.size() });
			}

			void Insert(size_t at, std::string insertion)
			{
				edits.push_back({ at, at, std::move(insertion), edits.size() });
			}

			void WalkObject(const Json& base, const Json& edited, const Node& node, Path& path)
			{
				if (node.keys.empty())
				{
					Replace(node, Format(ChangedSubtree(base, edited, path), LineIndent(node.begin)));
					return;
				}

				for (auto it = edited.begin(); it != edited.end(); ++it)
				{
					const std::string& key = it.key();
					const auto inBase = base.find(key);
					if (inBase != base.end() && *inBase == it.value())
						continue;

					path.push_back(key);
					size_t index = 0;
					if (const Node* child = node.Find(key, &index))
					{
						if (inBase != base.end())
							Walk(*inBase, it.value(), *child, path);
						else
							Replace(*child, Format(minimize(path, it.value()), LineIndent(node.keyBegins[index])));
					}
					else
					{
						InsertMember(node, key, inBase != base.end() ? ChangedSubtree(*inBase, it.value(), path) : minimize(path, it.value()));
					}
					path.pop_back();
				}

				// Members gone from the data: cut runs of them with their separators (never overlapping).
				std::vector<bool> removed(node.keys.size(), false);
				for (auto it = base.begin(); it != base.end(); ++it)
				{
					size_t index = 0;
					if (!edited.contains(it.key()) && node.Find(it.key(), &index))
						removed[index] = true;
				}
				if (std::all_of(removed.begin(), removed.end(), [](bool r) { return r; }))
				{
					Replace(node, "{}");
					return;
				}
				for (size_t i = 0; i < removed.size();)
				{
					if (!removed[i])
					{
						++i;
						continue;
					}
					size_t last = i;
					while (last + 1 < removed.size() && removed[last + 1])
						++last;
					if (i > 0)
						edits.push_back({ node.children[i - 1].end, node.children[last].end, std::string(), edits.size() });
					else
						edits.push_back({ node.keyBegins[0], node.keyBegins[last + 1], std::string(), edits.size() });
					i = last + 1;
				}
			}

			// Only what changed: a missing object written for an edit gets the edited fields, the rest stays default.
			Json ChangedSubtree(const Json& base, const Json& edited, Path& path)
			{
				if (!base.is_object() || !edited.is_object())
					return minimize(path, edited);

				Json out = Json::object();
				for (auto it = edited.begin(); it != edited.end(); ++it)
				{
					const auto inBase = base.find(it.key());
					if (inBase != base.end() && *inBase == it.value())
						continue;
					path.push_back(it.key());
					out[it.key()] = inBase != base.end() ? ChangedSubtree(*inBase, it.value(), path) : minimize(path, it.value());
					path.pop_back();
				}
				return out;
			}

			void InsertMember(const Node& node, const std::string& key, const Json& value)
			{
				const std::string keyText = Json(key).dump();
				const size_t last = node.keys.size() - 1;
				const size_t at = node.children[last].end;
				const bool multiLine = text.find('\n', node.begin) < node.keyBegins[0];
				if (multiLine)
				{
					const std::string indent = LineIndent(node.keyBegins[last]);
					Insert(at, ",\n" + indent + keyText + ": " + Format(value, indent));
				}
				else
				{
					Insert(at, ", " + keyText + ": " + Format(value, LineIndent(node.begin)));
				}
			}

			std::string LineIndent(size_t offset) const
			{
				const size_t newline = offset == 0 ? std::string::npos : text.rfind('\n', offset - 1);
				const size_t lineStart = newline == std::string::npos ? 0 : newline + 1;
				size_t i = lineStart;
				while (i < offset && (text[i] == '\t' || text[i] == ' '))
					++i;
				return text.substr(lineStart, i - lineStart);
			}

			static size_t Columns(const std::string& indent)
			{
				size_t columns = 0;
				for (char c : indent)
					columns += c == '\t' ? TabColumns : 1;
				return columns;
			}

			static std::string FormatScalar(const Json& value, std::string_view previous)
			{
				if (value.is_number_float())
					return FormatNumber(value.get<double>(), previous.find_first_of(".eE") != std::string_view::npos);
				return value.dump();   // integers, booleans, escaped strings, null
			}

			std::vector<std::string> OrderedKeys(const Json& object) const
			{
				std::vector<std::string> keys;
				for (auto it = object.begin(); it != object.end(); ++it)
					keys.push_back(it.key());
				auto rank = [this](const std::string& key)
				{
					const auto found = keyRank.find(key);
					return found == keyRank.end() ? std::numeric_limits<size_t>::max() : found->second;
				};
				std::stable_sort(keys.begin(), keys.end(), [&](const std::string& a, const std::string& b) { return rank(a) < rank(b); });
				return keys;
			}

			// One line: { "a": 1, "b": 2 }, ["x", "y"], [ { ... }, { ... } ].
			std::string FormatLine(const Json& value) const
			{
				if (value.is_object())
				{
					if (value.empty())
						return "{}";
					std::string out = "{ ";
					bool first = true;
					for (const std::string& key : OrderedKeys(value))
					{
						out += (first ? "" : ", ") + Json(key).dump() + ": " + FormatLine(value[key]);
						first = false;
					}
					return out + " }";
				}
				if (value.is_array())
				{
					if (value.empty())
						return "[]";
					const bool scalars = std::none_of(value.begin(), value.end(), [](const Json& e) { return e.is_structured(); });
					std::string out = scalars ? "[" : "[ ";
					for (size_t i = 0; i < value.size(); ++i)
						out += (i ? ", " : "") + FormatLine(value[i]);
					return out + (scalars ? "]" : " ]");
				}
				return FormatScalar(value, std::string_view());
			}

			// One line while it fits, otherwise one member / element per line, tab-indented below `indent`.
			std::string Format(const Json& value, const std::string& indent) const
			{
				const std::string line = FormatLine(value);
				constexpr size_t KeyAllowance = 24;   // the key in front of the value on its line
				if (!value.is_structured() || value.empty() || Columns(indent) + KeyAllowance + line.size() <= MaxLineColumns)
					return line;

				const std::string inner = indent + "\t";
				std::string out = value.is_object() ? "{\n" : "[\n";
				if (value.is_object())
				{
					bool first = true;
					for (const std::string& key : OrderedKeys(value))
					{
						out += (first ? "" : ",\n") + inner + Json(key).dump() + ": " + Format(value[key], inner);
						first = false;
					}
				}
				else
				{
					for (size_t i = 0; i < value.size(); ++i)
						out += (i ? ",\n" : "") + inner + Format(value[i], inner);
				}
				return out + "\n" + indent + (value.is_object() ? "}" : "]");
			}
		};
	}

	bool Apply(const std::string& text, const Json& base, const Json& edited, const Minimize& minimize, std::string& out, std::string& error)
	{
		Parser parser(text);
		Node root;
		if (!parser.Parse(root, error))
		{
			error = "cannot read the document: " + error;
			return false;
		}

		Patcher patcher(text, parser.keyRank, minimize);
		Path path;
		patcher.Walk(base, edited, root, path);
		return patcher.Result(out, error);
	}

	void Merge(Json& target, const Json& base, const Json& edited)
	{
		if (base == edited)
			return;

		if (base.is_object() && edited.is_object() && target.is_object())
		{
			for (auto it = edited.begin(); it != edited.end(); ++it)
			{
				const auto inBase = base.find(it.key());
				if (inBase == base.end())
					target[it.key()] = it.value();
				else if (*inBase != it.value())
				{
					if (target.contains(it.key()))
						Merge(target[it.key()], *inBase, it.value());
					else
						target[it.key()] = it.value();
				}
			}
			for (auto it = base.begin(); it != base.end(); ++it)
				if (!edited.contains(it.key()))
					target.erase(it.key());
			return;
		}

		if (base.is_array() && edited.is_array() && target.is_array() && base.size() == edited.size() && target.size() == base.size())
		{
			for (size_t i = 0; i < base.size(); ++i)
				Merge(target[i], base[i], edited[i]);
			return;
		}

		target = edited;
	}

	std::string FormatNumber(double value, bool forceDecimal)
	{
		char buffer[64];
		std::to_chars_result result;
		const float single = static_cast<float>(value);
		if (std::isfinite(value) && static_cast<double>(single) == value)
			result = std::to_chars(buffer, buffer + sizeof(buffer), single);   // data fields are floats: 0.4f -> "0.4"
		else
			result = std::to_chars(buffer, buffer + sizeof(buffer), value);

		std::string text(buffer, result.ptr);
		if (forceDecimal && text.find_first_of(".eE") == std::string::npos)
			text += ".0";
		return text;
	}
}
