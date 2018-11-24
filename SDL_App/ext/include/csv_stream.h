// The MIT License (MIT)
// capi Fast CSV Streams 0.5.0 Beta
// Copyright (C) 2017, by Wong Shao Voon (shaovoon@yahoo.com)
//
// http://opensource.org/licenses/MIT
//
// version 0.5.0  : First Commit
// version 0.5.1  : Fix Input Stream reading char exception
// version 0.5.2  : Add NChar class and its unit tests. Remove CHAR_AS_ASCII macro.
// version 0.5.3  : Can have an unescaped delimiter which will be enclosed by quote automatically
// version 0.5.3b : Update minicsv.h in Benchmark to 1.8.5
// version 0.5.4  : Add overloaded open functions that take in wchar file parameter (Only available on win32)

#ifndef CSV_STREAMS_H
	#define CSV_STREAMS_H

#include <string>
#include <cstdio>
#include <algorithm>
#include <stdexcept>

#ifdef USE_BOOST_SPIRIT_QI
	#include <boost/spirit/include/qi.hpp>
#endif

#ifdef USE_GOOGLE_DCONV
	#include "double_conversion/double-conversion.h"
#endif

#define NEWLINE '\n'

#ifdef _WIN32
#define MY_FUNC_SIG __FUNCSIG__
#else
#define MY_FUNC_SIG __PRETTY_FUNCTION__
#endif

namespace capi
{
	namespace csv
	{
		struct NChar
		{
			explicit NChar(char& ch_) : ch(ch_) {}
			const char& getChar() const { return ch; }
			char& getChar() { return ch; }
			void setChar(char ch_) { ch = ch_; }
		private:
			char& ch;
		};

#ifdef USE_BOOST_SPIRIT_QI
		template<typename string_type>
		inline bool str_to_value(const string_type& src, double& dest)
		{
			namespace qi = boost::spirit::qi;

			return qi::parse(std::cbegin(src), std::cend(src), qi::double_, dest);
		}

		template<typename string_type>
		inline bool str_to_value(const string_type& src, float& dest)
		{
			namespace qi = boost::spirit::qi;

			return qi::parse(std::cbegin(src), std::cend(src), qi::float_, dest);
		}

		template<typename string_type>
		inline bool str_to_value(const string_type& src, long long& dest)
		{
			namespace qi = boost::spirit::qi;

			return qi::parse(std::cbegin(src), std::cend(src), qi::long_long, dest);
		}

		template<typename string_type>
		inline bool str_to_value(const string_type& src, unsigned long long& dest)
		{
			namespace qi = boost::spirit::qi;

			return qi::parse(std::cbegin(src), std::cend(src), qi::ulong_long, dest);
		}

		template<typename string_type>
		inline bool str_to_value(const string_type& src, int& dest)
		{
			namespace qi = boost::spirit::qi;

			return qi::parse(std::cbegin(src), std::cend(src), qi::int_, dest);
		}

		template<typename string_type>
		inline bool str_to_value(const string_type& src, unsigned int& dest)
		{
			namespace qi = boost::spirit::qi;

			return qi::parse(std::cbegin(src), std::cend(src), qi::uint_, dest);
		}

		template<typename string_type>
		inline bool str_to_value(const string_type& src, short& dest)
		{
			namespace qi = boost::spirit::qi;

			return qi::parse(std::cbegin(src), std::cend(src), qi::short_, dest);
		}

		template<typename string_type>
		inline bool str_to_value(const string_type& src, unsigned short& dest)
		{
			namespace qi = boost::spirit::qi;

			return qi::parse(std::cbegin(src), std::cend(src), qi::ushort_, dest);
		}
#else
#ifdef USE_GOOGLE_DCONV
		template<typename string_type>
		inline bool str_to_value(const string_type& src, double& dest)
		{
			StringToDoubleConverter conv(StringToDoubleConverter::NO_FLAGS, 0.0, NAN, "infinity", "nan");
			int processed_characters_count = 0;
			dest = conv.StringToDouble(src.c_str(), src.size(), &processed_characters_count);

			return true;
		}

		template<typename string_type>
		inline bool str_to_value(const string_type& src, float& dest)
		{
			StringToDoubleConverter conv(StringToDoubleConverter::NO_FLAGS, 0.0, NAN, "infinity", "nan");
			int processed_characters_count = 0;
			dest = conv.StringToFloat(src.c_str(), src.size(), &processed_characters_count);

			return true;
		}
#else
		template<typename string_type>
		inline bool str_to_value(const string_type& src, double& dest)
		{
			try
			{
				dest = std::stod(src);
			}
			catch (std::invalid_argument&)
			{
				return false;
			}
			catch (std::out_of_range&)
			{
				return false;
			}
			return true;
		}

		template<typename string_type>
		inline bool str_to_value(const string_type& src, float& dest)
		{
			try
			{
				dest = std::stof(src);
			}
			catch (std::invalid_argument&)
			{
				return false;
			}
			catch (std::out_of_range&)
			{
				return false;
			}
			return true;
		}
#endif
		template<typename string_type>
		inline bool str_to_value(const string_type& src, long long& dest)
		{
			try
			{
				dest = std::stoll(src);
			}
			catch (std::invalid_argument&)
			{
				return false;
			}
			catch (std::out_of_range&)
			{
				return false;
			}
			return true;
		}

		template<typename string_type>
		inline bool str_to_value(const string_type& src, unsigned long long& dest)
		{
			try
			{
				dest = std::stoull(src);
			}
			catch (std::invalid_argument&)
			{
				return false;
			}
			catch (std::out_of_range&)
			{
				return false;
			}
			return true;
		}

		template<typename string_type>
		inline bool str_to_value(const string_type& src, int& dest)
		{
			try
			{
				dest = std::stoi(src);
			}
			catch (std::invalid_argument&)
			{
				return false;
			}
			catch (std::out_of_range&)
			{
				return false;
			}
			return true;
		}

		template<typename string_type>
		inline bool str_to_value(const string_type& src, unsigned int& dest)
		{
			try
			{
				dest = std::stoul(src);
			}
			catch (std::invalid_argument&)
			{
				return false;
			}
			catch (std::out_of_range&)
			{
				return false;
			}
			return true;
		}

		template<typename string_type>
		inline bool str_to_value(const string_type& src, short& dest)
		{
			try
			{
				int n = std::stoi(src);
				if (n > std::numeric_limits<short>::max())
					return false;
				if (n < std::numeric_limits<short>::min())
					return false;

				dest = static_cast<short>(n);
			}
			catch (std::invalid_argument&)
			{
				return false;
			}
			catch (std::out_of_range&)
			{
				return false;
			}
			return true;
		}

		template<typename string_type>
		inline bool str_to_value(const string_type& src, unsigned short& dest)
		{
			try
			{
				unsigned int n = std::stoul(src);
				if (n > std::numeric_limits<unsigned short>::max())
					return false;

				dest = static_cast<unsigned short>(n);
			}
			catch (std::invalid_argument&)
			{
				return false;
			}
			catch (std::out_of_range&)
			{
				return false;
			}
			return true;
		}

#endif

		template<typename string_type>
		inline bool str_to_value(const string_type& src, char& dest)
		{
			if (src.size() > 0)
				dest = src[0];
			else
				return false;

			return true;
		}

		template<typename string_type>
		inline bool str_to_value(const string_type& src, NChar& dest)
		{
			try
			{
				int n = std::stoi(src);
				if (n > 127)
					return false;
				if (n < -128)
					return false;

				char temp = static_cast<char>(n);
				dest.setChar(temp);
			}
			catch (std::invalid_argument&)
			{
				return false;
			}
			catch (std::out_of_range&)
			{
				return false;
			}
			return true;
		}

		template<typename string_type>
		inline bool str_to_value(const string_type& src, unsigned char& dest)
		{
			try
			{
				unsigned int n = std::stoul(src);
				if (n > 255)
					return false;

				dest = static_cast<unsigned char>(n);
			}
			catch (std::invalid_argument&)
			{
				return false;
			}
			catch (std::out_of_range&)
			{
				return false;
			}
			return true;
		}

		inline std::string const & replace(std::string & src, std::string const & to_find, std::string const & to_replace)
		{
			size_t pos = 0;
			while (std::string::npos != pos)
			{
				pos = src.find(to_find, pos);

				if (std::string::npos != pos)
				{
					src.erase(pos, to_find.size());
					src.insert(pos, to_replace);
					pos += to_replace.size();
				}
			}

			return src;
		}

		class sep // separator class for the stream, so that no need to call set_delimiter
		{
		public:
			sep(const char delimiter_, const std::string& escape_) : delimiter(delimiter_), escape(escape_) {}

			const char get_delimiter() const { return delimiter; }
			const std::string& get_escape() const { return escape; }
		private:
			const char delimiter;
			const std::string escape;
		};

		class ifstream
		{
		public:
			ifstream(const std::string& file = "")
				: input_file_ptr(nullptr)
				, str("")
				, pos(0)
				, delimiter(",")
				, unescape_str("##")
				, trim_quote_on_str(false)
				, trim_quote('\"')
				, trim_quote_str(1, trim_quote)
				, terminate_on_blank_line(true)
				, quote_unescape("&quot;")
				, has_bom(false)
				, first_line_read(false)
				, filename("")
				, line_num(0)
				, token_num(0)
				, allow_blank_line(false)
			{
				open(file);
			}
#ifdef _MSC_VER
			ifstream(const std::wstring& file = L"")
				: input_file_ptr(nullptr)
				, str("")
				, pos(0)
				, delimiter(",")
				, unescape_str("##")
				, trim_quote_on_str(false)
				, trim_quote('\"')
				, trim_quote_str(1, trim_quote)
				, terminate_on_blank_line(true)
				, quote_unescape("&quot;")
				, has_bom(false)
				, first_line_read(false)
				, filename("")
				, line_num(0)
				, token_num(0)
				, allow_blank_line(false)
			{
				open(file);
			}
#endif
			ifstream(const char * file)
				: input_file_ptr(nullptr)
				, str("")
				, pos(0)
				, delimiter(",")
				, unescape_str("##")
				, trim_quote_on_str(false)
				, trim_quote('\"')
				, trim_quote_str(1, trim_quote)
				, terminate_on_blank_line(true)
				, quote_unescape("&quot;")
				, has_bom(false)
				, first_line_read(false)
				, filename("")
				, line_num(0)
				, token_num(0)
				, allow_blank_line(false)
			{
				open(file);
			}
#ifdef _MSC_VER
			ifstream(const wchar_t * file)
				: input_file_ptr(nullptr)
				, str("")
				, pos(0)
				, delimiter(",")
				, unescape_str("##")
				, trim_quote_on_str(false)
				, trim_quote('\"')
				, trim_quote_str(1, trim_quote)
				, terminate_on_blank_line(true)
				, quote_unescape("&quot;")
				, has_bom(false)
				, first_line_read(false)
				, filename("")
				, line_num(0)
				, token_num(0)
				, allow_blank_line(false)
			{
				open(file);
			}
#endif
			~ifstream()
			{
				close();
			}
			void open(const std::string& file)
			{
				if (!file.empty())
					open(file.c_str());
			}
#ifdef _MSC_VER
			void open(const std::wstring& file)
			{
				if (!file.empty())
					open(file.c_str());
			}
#endif
			bool open(const char * file)
			{
				close();
				reset();
				filename = file;

#ifdef _MSC_VER
				fopen_s(&input_file_ptr, file, "rb");
#else
				input_file_ptr = fopen(file, "rb");
#endif
				if (!input_file_ptr)
					return false;

				read_bom();

				return true;
			}
#ifdef _MSC_VER
			bool open(const wchar_t * file)
			{
				close();
				reset();

				_wfopen_s(&input_file_ptr, file, L"rb");
				if (!input_file_ptr)
					return false;

				read_bom();

				return true;
			}
#endif
			void read_bom()
			{
				char tt[3] = { 0, 0, 0 };

				fread(tt, 3, 1, input_file_ptr);

				if (tt[0] == (char)0xEF && tt[1] == (char)0xBB && tt[2] == (char)0xBF) // not the correct BOM, so reset the pos to beginning (file might not have BOM)
					has_bom = true;

				fseek(input_file_ptr, 0, SEEK_SET);
			}
			void reset()
			{
				input_file_ptr = nullptr;
				str = "";
				pos = 0;
				delimiter = ',';
				unescape_str = "##";
				trim_quote_on_str = false;
				trim_quote = '\"';
				trim_quote_str = std::string(1, trim_quote);
				terminate_on_blank_line = true;
				has_bom = false;
				first_line_read = false;
				filename = "";
				line_num = 0;
				token_num = 0;
				allow_blank_line = false;
			}
			void close()
			{
				if (input_file_ptr)
				{
					fclose(input_file_ptr);
					input_file_ptr = nullptr;
				}
			}
			bool is_open()
			{
				return (input_file_ptr != nullptr);
			}
			void enable_trim_quote_on_str(bool enable, char quote, const std::string& unescape = "&quot;")
			{
				trim_quote_on_str = enable;
				trim_quote = quote;
				trim_quote_str = std::string(1, trim_quote);
				quote_unescape = unescape;
			}
			void enable_blank_line(bool enable)
			{
				allow_blank_line = enable;
			}
			// eof is replaced by read_line
			//bool eof() const
			void set_delimiter(char delimiter_, std::string const & unescape_str_)
			{
				delimiter = delimiter_;
				unescape_str = unescape_str_;
			}
			std::string const &  get_delimiter() const
			{
				return delimiter;
			}
			std::string const &  get_unescape_str() const
			{
				return unescape_str;
			}

			void getline()
			{
				char ch = '\0';
				do
				{
					ch = fgetc(input_file_ptr);
					if (ch == '\r' || ch == '\n') continue;
					if (ch != EOF)
					{
						this->str += ch;
					}
					else
						return;

				} while (ch != '\n');
			}

			void skip_line()
			{
				if (!feof(input_file_ptr))
				{
					getline();
					pos = 0;

					if (first_line_read == false)
					{
						first_line_read = true;
					}
				}
			}
			bool read_line()
			{
				this->str = "";
				while (!feof(input_file_ptr))
				{
					getline();
					pos = 0;

					if (first_line_read == false)
					{
						first_line_read = true;
						if (has_bom)
						{
							this->str = this->str.substr(3);
						}
					}

					if (this->str.empty())
					{
						if (terminate_on_blank_line)
							break;
						else if(allow_blank_line==false)
							continue;
					}

					++line_num;
					token_num = 0;
					return true;
				}
				return false;
			}
			const std::string& get_delimited_str()
			{
				token = "";
				char ch = '\0';
				bool within_quote = false;
				do
				{
					if (pos >= this->str.size())
					{
						this->str = "";

						++token_num;
						token = unescape(token);
						return token;
					}

					ch = this->str[pos];
					//if (trim_quote_on_str)
					{
						if (within_quote&& ch == trim_quote && this->str[pos + 1] == trim_quote)
						{
							token += ch;
							pos += 2;
							continue;
						}
						if (within_quote == false && ch == trim_quote && ((pos > 0 && this->str[pos - 1] == delimiter[0]) || pos == 0))
							within_quote = true;
						else if (within_quote && ch == trim_quote)
							within_quote = false;
					}

					++(pos);

					if (ch == delimiter[0] && within_quote == false)
						break;
					if (ch == '\r' || ch == '\n')
						break;

					token += ch;
				} while (true);

				++token_num;
				token = unescape(token);
				return token;
			}
			std::string& unescape(std::string & src)
			{
				src = unescape_str.empty() ? src : replace(src, unescape_str, delimiter);
				//if (trim_quote_on_str)
				{
					if (!src.empty() && (src[0] == trim_quote && src[src.size() - 1] == trim_quote))
					{
						src = src.substr(1, src.size() - 2);
					}

					if (std::string::npos != src.find(quote_unescape, 0))
					{
						src = replace(src, quote_unescape, trim_quote_str);
					}
				}
				return src;
			}
			size_t num_of_delimiter() const
			{
				if (delimiter.size() == 0)
					return 0;

				size_t cnt = 0;
				//if (trim_quote_on_str)
				{
					bool inside_quote = false;
					for (size_t i = 0; i < str.size(); ++i)
					{
						if (str[i] == trim_quote)
							inside_quote = !inside_quote;

						if (!inside_quote)
						{
							if (str[i] == delimiter[0])
								++cnt;
						}
					}
				}
				return cnt;
			}
			std::string get_rest_of_line() const
			{
				return str.substr(pos);
			}
			const std::string& get_line() const
			{
				return str;
			}
			void enable_terminate_on_blank_line(bool enable)
			{
				terminate_on_blank_line = enable;
			}
			bool is_terminate_on_blank_line() const
			{
				return terminate_on_blank_line;
			}
			std::string error_line(const std::string& token_param, const std::string& function_site)
			{
				std::string error = "csv::ifstream conversion error at line no.:";
				error += std::to_string(line_num) + ", filename:";
				error += filename + ", token position:";
				error += std::to_string(token_num) + ", token:";
				error += token_param + ", function:";
				error += function_site;

				return error;
			}

		private:
			std::FILE* input_file_ptr;
			std::string str;
			size_t pos;
			std::string delimiter;
			std::string unescape_str;
			bool trim_quote_on_str;
			char trim_quote;
			std::string trim_quote_str;
			bool terminate_on_blank_line;
			std::string quote_unescape;
			bool has_bom;
			bool first_line_read;
			std::string filename;
			size_t line_num;
			size_t token_num;
			std::string token;
			bool allow_blank_line;
		};

		class ofstream
		{
		public:

			ofstream(const std::string& file = "")
				: output_file_ptr(nullptr)
				, after_newline(true)
				, delimiter(",")
				, escape_str("##")
				, surround_quote_on_str(false)
				, surround_quote('\"')
				, quote_escape("&quot;")
			{
				open(file);
			}
#ifdef _MSC_VER
			ofstream(const std::wstring& file = L"")
				: output_file_ptr(nullptr)
				, after_newline(true)
				, delimiter(",")
				, escape_str("##")
				, surround_quote_on_str(false)
				, surround_quote('\"')
				, quote_escape("&quot;")
			{
				open(file);
			}
#endif
			~ofstream()
			{
				close();
			}
			ofstream(const char * file)
				: output_file_ptr(nullptr)
				, after_newline(true)
				, delimiter(",")
				, escape_str("##")
				, surround_quote_on_str(false)
				, surround_quote('\"')
				, quote_escape("&quot;")
			{
				open(file);
			}
#ifdef _MSC_VER
			ofstream(const wchar_t * file)
				: output_file_ptr(nullptr)
				, after_newline(true)
				, delimiter(",")
				, escape_str("##")
				, surround_quote_on_str(false)
				, surround_quote('\"')
				, quote_escape("&quot;")
			{
				open(file);
			}
#endif
			void open(const std::string& file)
			{
				if (!file.empty())
					open(file.c_str());
			}
#ifdef _MSC_VER
			void open(const std::wstring& file)
			{
				if (!file.empty())
					open(file.c_str());
			}
#endif
			void open(const char * file)
			{
				close();
				reset();
#ifdef _MSC_VER
				fopen_s(&output_file_ptr, file, "wb");
#else
				output_file_ptr = fopen(file, "wb");
#endif
			}
#ifdef _MSC_VER
			void open(const wchar_t * file)
			{
				close();
				reset();
				_wfopen_s(&output_file_ptr, file, L"wb");
			}
#endif
			void reset()
			{
				after_newline = true;
				delimiter = ',';
				escape_str = "##";
				surround_quote_on_str = false;
				surround_quote = '\"';
				quote_escape = "&quot;";
			}
			void flush()
			{
				fflush(output_file_ptr);
			}
			void close()
			{
				if (output_file_ptr)
				{
					fclose(output_file_ptr);
					output_file_ptr = nullptr;
				}
			}
			bool is_open()
			{
				return output_file_ptr != nullptr;
			}
			void enable_surround_quote_on_str(bool enable, char quote, const std::string& escape = "&quot;")
			{
				surround_quote_on_str = enable;
				surround_quote = quote;
				quote_escape = escape;
			}
			void set_delimiter(char delimiter_, std::string const & escape_str_)
			{
				delimiter = delimiter_;
				escape_str = escape_str_;
			}
			std::string const &  get_delimiter() const
			{
				return delimiter;
			}
			std::string const &  get_escape_str() const
			{
				return escape_str;
			}
			void set_after_newline(bool after_newline_)
			{
				after_newline = after_newline_;
			}
			bool get_after_newline() const
			{
				return after_newline;
			}
			void escape_and_output(std::string& src)
			{
				std::string out = ((escape_str.empty()) ? src : replace(src, delimiter, escape_str));
				fwrite(out.data(), 1, out.size(), output_file_ptr);
			}
			void escape_str_and_output(std::string& src)
			{
				src = ((escape_str.empty()) ? src : replace(src, delimiter, escape_str));
				if (surround_quote_on_str || src.find(delimiter) != std::string::npos)
				{
					if (!quote_escape.empty())
					{
						if (src.find_first_of(surround_quote) != std::string::npos)
						{
							src = replace(src, std::string(1, surround_quote), quote_escape);
						}
					}

					fwrite(&surround_quote, 1, 1, output_file_ptr);
					fwrite(src.data(), 1, src.size(), output_file_ptr);
					fwrite(&surround_quote, 1, 1, output_file_ptr);
				}
				else
				{
					fwrite(src.data(), 1, src.size(), output_file_ptr);
				}
			}
			void write_char(char ch)
			{
				fwrite(&ch, 1, 1, output_file_ptr);
			}
		private:
			std::FILE* output_file_ptr;
			bool after_newline;
			std::string delimiter;
			std::string escape_str;
			bool surround_quote_on_str;
			char surround_quote;
			std::string quote_escape;
		};


	} // ns csv
} // ns capi

	template<typename T>
	inline capi::csv::ifstream& operator >> (capi::csv::ifstream& istm, T& val)
	{
		const std::string& src = istm.get_delimited_str();

		if (!capi::csv::str_to_value(src, val))
		{
			throw std::runtime_error(istm.error_line(src, MY_FUNC_SIG).c_str());
		}

		return istm;
	}

	inline capi::csv::ifstream& operator >> (capi::csv::ifstream& istm, capi::csv::NChar val)
	{
		const std::string& src = istm.get_delimited_str();

		if (!capi::csv::str_to_value(src, val))
		{
			throw std::runtime_error(istm.error_line(src, MY_FUNC_SIG).c_str());
		}

		return istm;
	}

	template<>
	inline capi::csv::ifstream& operator >> (capi::csv::ifstream& istm, char& val)
	{
		const std::string& src = istm.get_delimited_str();

		if (src.empty())
		{
			throw std::runtime_error(istm.error_line(src, MY_FUNC_SIG).c_str());
		}

		val = src[0];

		return istm;
	}

	template<>
	inline capi::csv::ifstream& operator >> (capi::csv::ifstream& istm, std::string& val)
	{
		val = istm.get_delimited_str();

		return istm;
	}

	template<>
	inline capi::csv::ifstream& operator >> (capi::csv::ifstream& istm, capi::csv::sep& val)
	{
		istm.set_delimiter(val.get_delimiter(), val.get_escape());

		return istm;
	}

	template<typename T>
	capi::csv::ofstream& operator << (capi::csv::ofstream& ostm, const T& val)
	{
		if (!ostm.get_after_newline() && ostm.get_delimiter().size()>0)
			ostm.write_char(ostm.get_delimiter()[0]);

		std::string temp = std::to_string(val);
		ostm.escape_and_output(temp);

		ostm.set_after_newline(false);

		return ostm;
	}

	template<typename T>
	capi::csv::ofstream& operator << (capi::csv::ofstream& ostm, const T* val)
	{
		if (!ostm.get_after_newline() && ostm.get_delimiter().size()>0)
			ostm.write_char(ostm.get_delimiter()[0]);

		std::string temp = std::to_string(*val);
		ostm.escape_and_output(temp);

		ostm.set_after_newline(false);

		return ostm;
	}

	template<>
	inline capi::csv::ofstream& operator << (capi::csv::ofstream& ostm, const std::string& val)
	{
		if (!ostm.get_after_newline()&& ostm.get_delimiter().size()>0)
			ostm.write_char(ostm.get_delimiter()[0]);

		std::string temp = val;
		ostm.escape_str_and_output(temp);

		ostm.set_after_newline(false);

		return ostm;
	}

	inline capi::csv::ofstream& operator << (capi::csv::ofstream& ostm, const capi::csv::NChar val)
	{
		if (!ostm.get_after_newline() && ostm.get_delimiter().size() > 0)
			ostm.write_char(ostm.get_delimiter()[0]);

		std::string temp = std::to_string((int)val.getChar());
		ostm.escape_str_and_output(temp);

		ostm.set_after_newline(false);

		return ostm;
	}

	template<>
	inline capi::csv::ofstream& operator << (capi::csv::ofstream& ostm, const capi::csv::sep& val)
	{
		ostm.set_delimiter(val.get_delimiter(), val.get_escape());

		return ostm;
	}

	template<>
	inline capi::csv::ofstream& operator << (capi::csv::ofstream& ostm, const char& val)
	{
		if (val == NEWLINE)
		{
#ifdef WIN32
			ostm.write_char('\r');
#endif
			ostm.write_char(NEWLINE);

			ostm.set_after_newline(true);
		}
		else
		{
			if (!ostm.get_after_newline() && ostm.get_delimiter().size() > 0)
				ostm.write_char(ostm.get_delimiter()[0]);

			std::string temp = "";
			temp += val;
			ostm.escape_str_and_output(temp);

			ostm.set_after_newline(false);
		}

		return ostm;
	}
	template<>
	inline capi::csv::ofstream& operator << (capi::csv::ofstream& ostm, const char* val)
	{
		const std::string temp = val;

		ostm << temp;

		return ostm;
	}
namespace capi
{
	namespace csv
	{
		class istringstream
		{
		public:
			istringstream(const char * text)
			{
				set_new_input_string(text);
			}
			istringstream(const std::string& text)
			{
				set_new_input_string(text);
			}

			void set_new_input_string(const std::string& text)
			{
				reset(text);
			}

			void reset(const std::string& text)
			{
				input_str = text;
				input_str_index = 0;
				str = "";
				pos = 0;
				delimiter = ",";
				unescape_str = "##";
				trim_quote_on_str = false;
				trim_quote = '\"';
				trim_quote_str = std::string(1, trim_quote);
				terminate_on_blank_line = true;
				quote_unescape = "&quot;";
				line_num = 0;
				token_num = 0;
				allow_blank_line = false;
			}
			void enable_trim_quote_on_str(bool enable, char quote, const std::string& unescape = "&quot;")
			{
				trim_quote_on_str = enable;
				trim_quote = quote;
				trim_quote_str = std::string(1, trim_quote);
				quote_unescape = unescape;
			}
			void enable_blank_line(bool enable)
			{
				allow_blank_line = enable;
			}
			void set_delimiter(char delimiter_, std::string const & unescape_str_)
			{
				delimiter = delimiter_;
				unescape_str = unescape_str_;
			}
			std::string const & get_delimiter() const
			{
				return delimiter;
			}
			std::string const &  get_unescape_str() const
			{
				return unescape_str;
			}
			void getline()
			{
				char ch = '\0';
				do
				{
					ch = input_str[input_str_index];
					if (ch == '\r' || ch == '\n') continue;
					if (input_str_index < input_str.size())
					{
						this->str += ch;
					}
					else
						return;

				} while (input_str[++input_str_index] != '\n' && input_str_index < input_str.size());
				++input_str_index;
			}

			void skip_line()
			{
				getline();
				pos = 0;
			}
			bool read_line()
			{
				this->str = "";
				while (input_str_index < input_str.size())
				{
					getline();
					pos = 0;

					if (this->str.empty())
					{
						if (terminate_on_blank_line)
							break;
						else if(allow_blank_line==false)
							continue;
					}

					++line_num;
					token_num = 0;
					return true;
				}
				return false;
			}
			const std::string& get_delimited_str()
			{
				token = "";
				char ch = '\0';
				bool within_quote = false;
				do
				{
					if (pos >= this->str.size())
					{
						this->str = "";

						++token_num;
						token = unescape(token);
						return token;
					}

					ch = this->str[pos];
					//if (trim_quote_on_str)
					{
						if (within_quote&& ch == trim_quote && this->str[pos + 1] == trim_quote)
						{
							token += ch;
							pos += 2;
							continue;
						}

						if (within_quote == false && ch == trim_quote && ((pos > 0 && this->str[pos - 1] == delimiter[0]) || pos == 0))
							within_quote = true;
						else if (within_quote && ch == trim_quote)
							within_quote = false;
					}

					++(pos);

					if (ch == delimiter[0] && within_quote == false)
						break;
					if (ch == '\r' || ch == '\n')
						break;

					token += ch;
				} while (true);

				++token_num;
				token = unescape(token);
				return token;
			}

			std::string& unescape(std::string & src)
			{
				src = unescape_str.empty() ? src : replace(src, unescape_str, delimiter);
				//if (trim_quote_on_str)
				{
					if (!src.empty() && (src[0] == trim_quote && src[src.size() - 1] == trim_quote))
					{
						src = src.substr(1, src.size() - 2);
					}

					if (std::string::npos != src.find(quote_unescape, 0))
					{
						src = replace(src, quote_unescape, trim_quote_str);
					}
				}
				return src;
			}

			size_t num_of_delimiter() const
			{
				if (delimiter.size() == 0)
					return 0;

				size_t cnt = 0;
				//if (trim_quote_on_str)
				{
					bool inside_quote = false;
					for (size_t i = 0; i < str.size(); ++i)
					{
						if (str[i] == trim_quote)
							inside_quote = !inside_quote;

						if (!inside_quote)
						{
							if (str[i] == delimiter[0])
								++cnt;
						}
					}
				}
				return cnt;
			}
			std::string get_rest_of_line() const
			{
				return str.substr(pos);
			}
			const std::string& get_line() const
			{
				return str;
			}
			void enable_terminate_on_blank_line(bool enable)
			{
				terminate_on_blank_line = enable;
			}
			bool is_terminate_on_blank_line() const
			{
				return terminate_on_blank_line;
			}
			std::string error_line(const std::string& token_param, const std::string& function_site)
			{
				std::string error = "csv::istringstream conversion error at line no.:";
				error += std::to_string(line_num) + ", token position:";
				error += std::to_string(token_num) + ", token:";
				error += token_param + ", function:";
				error += function_site;

				return error;
			}

		private:
			std::string input_str;
			size_t input_str_index;
			std::string str;
			size_t pos;
			std::string delimiter;
			std::string unescape_str;
			bool trim_quote_on_str;
			char trim_quote;
			std::string trim_quote_str;
			bool terminate_on_blank_line;
			std::string quote_unescape;
			size_t line_num;
			size_t token_num;
			std::string token;
			bool allow_blank_line;
		};

		class icachedfstream
		{
		public:
			icachedfstream(const std::string& file = "")
				: input_str("")
				, input_str_index(0)
				, str("")
				, pos(0)
				, delimiter(",")
				, unescape_str("##")
				, trim_quote_on_str(false)
				, trim_quote('\"')
				, trim_quote_str(1, trim_quote)
				, terminate_on_blank_line(true)
				, quote_unescape("&quot;")
				, line_num(0)
				, token_num(0)
				, allow_blank_line(false)
			{
				open(file);
			}
#ifdef _MSC_VER
			icachedfstream(const std::wstring& file = L"")
				: input_str("")
				, input_str_index(0)
				, str("")
				, pos(0)
				, delimiter(",")
				, unescape_str("##")
				, trim_quote_on_str(false)
				, trim_quote('\"')
				, trim_quote_str(1, trim_quote)
				, terminate_on_blank_line(true)
				, quote_unescape("&quot;")
				, line_num(0)
				, token_num(0)
				, allow_blank_line(false)
			{
				open(file);
			}
#endif
			icachedfstream(const char * file)
				: input_str("")
				, input_str_index(0)
				, str("")
				, pos(0)
				, delimiter(",")
				, unescape_str("##")
				, trim_quote_on_str(false)
				, trim_quote('\"')
				, trim_quote_str(1, trim_quote)
				, terminate_on_blank_line(true)
				, quote_unescape("&quot;")
				, line_num(0)
				, token_num(0)
				, allow_blank_line(false)
			{
				open(file);
			}
#ifdef _MSC_VER
			icachedfstream(const wchar_t * file)
				: input_str("")
				, input_str_index(0)
				, str("")
				, pos(0)
				, delimiter(",")
				, unescape_str("##")
				, trim_quote_on_str(false)
				, trim_quote('\"')
				, trim_quote_str(1, trim_quote)
				, terminate_on_blank_line(true)
				, quote_unescape("&quot;")
				, line_num(0)
				, token_num(0)
				, allow_blank_line(false)
			{
				open(file);
			}
#endif
			void reset()
			{
				input_str = "";
				input_str_index = 0;
				str = "";
				pos = 0;
				delimiter = ",";
				unescape_str = "##";
				trim_quote_on_str = false;
				trim_quote = '\"';
				trim_quote_str = std::string(1, trim_quote);
				terminate_on_blank_line = true;
				quote_unescape = "&quot;";
				line_num = 0;
				token_num = 0;
				allow_blank_line = false;
			}
			void open(const std::string& file)
			{
				if (!file.empty())
					open(file.c_str());
			}
#ifdef _MSC_VER
			void open(const std::wstring& file)
			{
				if (!file.empty())
					open(file.c_str());
			}
#endif
			bool open(const char * file)
			{
				reset();
#ifdef _MSC_VER
				FILE* input_file_ptr = nullptr;
				fopen_s(&input_file_ptr, file, "rb");
#else
				FILE* input_file_ptr = fopen(file, "rb");
#endif
				if (!input_file_ptr)
					return false;

				long size = compute_length(input_file_ptr);

				input_str.resize(size);
				char* p = const_cast<char*>(input_str.c_str());
				std::fread(p, size, 1, input_file_ptr);

				fclose(input_file_ptr);

				return true;
			}
#ifdef _MSC_VER
			bool open(const wchar_t * file)
			{
				reset();
				FILE* input_file_ptr = nullptr;
				_wfopen_s(&input_file_ptr, file, L"rb");
				if (!input_file_ptr)
					return false;

				long size = compute_length(input_file_ptr);

				input_str.resize(size);
				char* p = const_cast<char*>(input_str.c_str());
				std::fread(p, size, 1, input_file_ptr);

				fclose(input_file_ptr);

				return true;
			}
#endif
			void enable_trim_quote_on_str(bool enable, char quote, const std::string& unescape = "&quot;")
			{
				trim_quote_on_str = enable;
				trim_quote = quote;
				trim_quote_str = std::string(1, trim_quote);
				quote_unescape = unescape;
			}
			void set_delimiter(char delimiter_, std::string const & unescape_str_)
			{
				delimiter = delimiter_;
				unescape_str = unescape_str_;
			}
			std::string const & get_delimiter() const
			{
				return delimiter;
			}
			std::string const &  get_unescape_str() const
			{
				return unescape_str;
			}
			void getline()
			{
				char ch = '\0';
				do
				{
					ch = input_str[input_str_index];
					if (ch == '\r' || ch == '\n') continue;
					if (input_str_index < input_str.size())
					{
						this->str += ch;
					}
					else
						return;

				} while (input_str[++input_str_index] != '\n' && input_str_index < input_str.size());
				++input_str_index;
			}

			void skip_line()
			{
				getline();
				pos = 0;
			}
			bool read_line()
			{
				this->str = "";
				while (input_str_index < input_str.size())
				{
					getline();
					pos = 0;

					if (this->str.empty())
					{
						if (terminate_on_blank_line)
							break;
						else if(allow_blank_line==false)
							continue;
					}

					++line_num;
					token_num = 0;
					return true;
				}
				return false;
			}
			const std::string& get_delimited_str()
			{
				token = "";
				char ch = '\0';
				bool within_quote = false;
				do
				{
					if (pos >= this->str.size())
					{
						this->str = "";

						++token_num;
						token = unescape(token);
						return token;
					}

					ch = this->str[pos];
					//if (trim_quote_on_str)
					{
						if (within_quote&& ch == trim_quote && this->str[pos + 1] == trim_quote)
						{
							token += ch;
							pos += 2;
							continue;
						}

						if (within_quote == false && ch == trim_quote && ((pos > 0 && this->str[pos - 1] == delimiter[0]) || pos == 0))
							within_quote = true;
						else if (within_quote && ch == trim_quote)
							within_quote = false;
					}

					++(pos);

					if (ch == delimiter[0] && within_quote == false)
						break;
					if (ch == '\r' || ch == '\n')
						break;

					token += ch;
				} while (true);

				++token_num;
				token = unescape(token);
				return token;
			}

			std::string& unescape(std::string & src)
			{
				src = unescape_str.empty() ? src : replace(src, unescape_str, delimiter);
				//if (trim_quote_on_str)
				{
					if (!src.empty() && (src[0] == trim_quote && src[src.size() - 1] == trim_quote))
					{
						src = src.substr(1, src.size() - 2);
					}

					if (std::string::npos != src.find(quote_unescape, 0))
					{
						src = replace(src, quote_unescape, trim_quote_str);
					}
				}
				return src;
			}

			size_t num_of_delimiter() const
			{
				if (delimiter.size() == 0)
					return 0;

				size_t cnt = 0;
				//if (trim_quote_on_str)
				{
					bool inside_quote = false;
					for (size_t i = 0; i < str.size(); ++i)
					{
						if (str[i] == trim_quote)
							inside_quote = !inside_quote;

						if (!inside_quote)
						{
							if (str[i] == delimiter[0])
								++cnt;
						}
					}
				}
				return cnt;
			}
			std::string get_rest_of_line() const
			{
				return str.substr(pos);
			}
			const std::string& get_line() const
			{
				return str;
			}
			void enable_terminate_on_blank_line(bool enable)
			{
				terminate_on_blank_line = enable;
			}
			void enable_blank_line(bool enable)
			{
				allow_blank_line = enable;
			}
			bool is_terminate_on_blank_line() const
			{
				return terminate_on_blank_line;
			}
			std::string error_line(const std::string& token_param, const std::string& function_site)
			{
				std::string error = "csv::istringfstream conversion error at line no.:";
				error += std::to_string(line_num) + ", token position:";
				error += std::to_string(token_num) + ", token:";
				error += token_param + ", function:";
				error += function_site;

				return error;
			}
			long tellg(std::FILE* input_file_ptr) const
			{
				return std::ftell(input_file_ptr);
			}
			void seekg(std::FILE* input_file_ptr, long file_pos)
			{
				std::fseek(input_file_ptr, file_pos, SEEK_SET);
			}
			void seekg(std::FILE* input_file_ptr, long offset, int way)
			{
				std::fseek(input_file_ptr, offset, way);
			}

		private:
			long compute_length(std::FILE* input_file_ptr)
			{
				seekg(input_file_ptr, 0, SEEK_END);
				long size = tellg(input_file_ptr);
				seekg(input_file_ptr, 0, SEEK_SET);
				return size;
			}

			std::string input_str;
			size_t input_str_index;
			std::string str;
			size_t pos;
			std::string delimiter;
			std::string unescape_str;
			bool trim_quote_on_str;
			char trim_quote;
			std::string trim_quote_str;
			bool terminate_on_blank_line;
			std::string quote_unescape;
			size_t line_num;
			size_t token_num;
			std::string token;
			bool allow_blank_line;
		};

		class ostringstream
		{
		public:

			ostringstream()
			{
				reset();
			}
			void reset()
			{
				output_str = "";
				after_newline = true;
				delimiter = ",";
				escape_str = "##";
				surround_quote_on_str = false;
				surround_quote = '\"';
				quote_escape = "&quot;";
			}
			void enable_surround_quote_on_str(bool enable, char quote, const std::string& escape = "&quot;")
			{
				surround_quote_on_str = enable;
				surround_quote = quote;
				quote_escape = escape;
			}
			void set_delimiter(char delimiter_, std::string const & escape_str_)
			{
				delimiter = delimiter_;
				escape_str = escape_str_;
			}
			std::string const & get_delimiter() const
			{
				return delimiter;
			}
			std::string const &  get_escape_str() const
			{
				return escape_str;
			}
			void set_after_newline(bool after_newline_)
			{
				after_newline = after_newline_;
			}
			bool get_after_newline() const
			{
				return after_newline;
			}
			const std::string& get_text() const
			{
				return output_str;
			}
			void escape_and_output(std::string& src)
			{
				output_str += ((escape_str.empty()) ? src : replace(src, delimiter, escape_str));
			}
			void escape_str_and_output(std::string& src)
			{
				src = ((escape_str.empty()) ? src : replace(src, delimiter, escape_str));
				if (surround_quote_on_str || src.find(delimiter) != std::string::npos)
				{
					if (!quote_escape.empty())
					{
						if (src.find_first_of(surround_quote) != std::string::npos)
						{
							src = replace(src, std::string(1, surround_quote), quote_escape);
						}
					}
					output_str += surround_quote;
					output_str += src;
					output_str += surround_quote;
				}
				else
				{
					output_str += src;
				}
			}
			void write_char(char ch)
			{
				output_str += ch;
			}

		private:
			std::string output_str;
			bool after_newline;
			std::string delimiter;
			std::string escape_str;
			bool surround_quote_on_str;
			char surround_quote;
			std::string quote_escape;
		};


		class ocachedfstream
		{
		public:

			ocachedfstream()
			{
				reset();
			}
			void reset()
			{
				output_str = "";
				after_newline = true;
				delimiter = ",";
				escape_str = "##";
				surround_quote_on_str = false;
				surround_quote = '\"';
				quote_escape = "&quot;";
			}
			void enable_surround_quote_on_str(bool enable, char quote, const std::string& escape = "&quot;")
			{
				surround_quote_on_str = enable;
				surround_quote = quote;
				quote_escape = escape;
			}
			void set_delimiter(char delimiter_, std::string const & escape_str_)
			{
				delimiter = delimiter_;
				escape_str = escape_str_;
			}
			std::string const & get_delimiter() const
			{
				return delimiter;
			}
			std::string const &  get_escape_str() const
			{
				return escape_str;
			}
			void set_after_newline(bool after_newline_)
			{
				after_newline = after_newline_;
			}
			bool get_after_newline() const
			{
				return after_newline;
			}
			const std::string& get_text() const
			{
				return output_str;
			}
			void escape_and_output(std::string& src)
			{
				output_str += ((escape_str.empty()) ? src : replace(src, delimiter, escape_str));
			}
			void escape_str_and_output(std::string& src)
			{
				src = ((escape_str.empty()) ? src : replace(src, delimiter, escape_str));
				if (surround_quote_on_str || src.find(delimiter) != std::string::npos)
				{
					if (!quote_escape.empty())
					{
						if (src.find_first_of(surround_quote) != std::string::npos)
						{
							src = replace(src, std::string(1, surround_quote), quote_escape);
						}
					}
					output_str += surround_quote;
					output_str += src;
					output_str += surround_quote;
				}
				else
				{
					output_str += src;
				}
			}
			void write_char(char ch)
			{
				output_str += ch;
			}

			bool write_to_file(const char* file)
			{
#ifdef _MSC_VER
				FILE* fp = nullptr; 
				fopen_s(&fp, file, "wb");
#else
				FILE* fp = fopen(file, "wb");
#endif
				if (fp)
				{
					size_t size = std::fwrite(output_str.c_str(), output_str.size(), 1, fp);
					std::fflush(fp);
					std::fclose(fp);
					output_str.clear();
					after_newline = true;
					return size == 1u;
				}
				return false;
			}
#ifdef _MSC_VER
			bool write_to_file(const wchar_t* file)
			{
				FILE* fp = nullptr;
				_wfopen_s(&fp, file, L"wb");
				if (fp)
				{
					size_t size = std::fwrite(output_str.c_str(), output_str.size(), 1, fp);
					std::fflush(fp);
					std::fclose(fp);
					output_str.clear();
					after_newline = true;
					return size == 1u;
				}
				return false;
			}
#endif

		private:
			std::string output_str;
			bool after_newline;
			std::string delimiter;
			std::string escape_str;
			bool surround_quote_on_str;
			char surround_quote;
			std::string quote_escape;
		};

	} // ns csv

} // ns capi

template<typename T>
capi::csv::istringstream& operator >> (capi::csv::istringstream& istm, T& val)
{
	const std::string& src = istm.get_delimited_str();

	if (!capi::csv::str_to_value(src, val))
	{
		throw std::runtime_error(istm.error_line(src, MY_FUNC_SIG).c_str());
	}

	return istm;
}

inline capi::csv::istringstream& operator >> (capi::csv::istringstream& istm, capi::csv::NChar val)
{
	const std::string& src = istm.get_delimited_str();

	if (!capi::csv::str_to_value(src, val))
	{
		throw std::runtime_error(istm.error_line(src, MY_FUNC_SIG).c_str());
	}

	return istm;
}

inline capi::csv::istringstream& operator >> (capi::csv::istringstream& istm, char& val)
{
	const std::string& src = istm.get_delimited_str();

	if (src.empty())
	{
		throw std::runtime_error(istm.error_line(src, MY_FUNC_SIG).c_str());
	}

	val = src[0];

	return istm;
}

inline capi::csv::istringstream& operator >> (capi::csv::istringstream& istm, std::string& val)
{
	val = istm.get_delimited_str();

	return istm;
}

inline capi::csv::istringstream& operator >> (capi::csv::istringstream& istm, capi::csv::sep& val)
{
	istm.set_delimiter(val.get_delimiter(), val.get_escape());

	return istm;
}

template<typename T>
capi::csv::icachedfstream& operator >> (capi::csv::icachedfstream& istm, T& val)
{
	const std::string& src = istm.get_delimited_str();

	if (!capi::csv::str_to_value(src, val))
	{
		throw std::runtime_error(istm.error_line(src, MY_FUNC_SIG).c_str());
	}

	return istm;
}

inline capi::csv::icachedfstream& operator >> (capi::csv::icachedfstream& istm, capi::csv::NChar val)
{
	const std::string& src = istm.get_delimited_str();

	if (!capi::csv::str_to_value(src, val))
	{
		throw std::runtime_error(istm.error_line(src, MY_FUNC_SIG).c_str());
	}

	return istm;
}

inline capi::csv::icachedfstream& operator >> (capi::csv::icachedfstream& istm, char& val)
{
	const std::string& src = istm.get_delimited_str();

	if (src.empty())
	{
		throw std::runtime_error(istm.error_line(src, MY_FUNC_SIG).c_str());
	}

	val = src[0];

	return istm;
}

inline capi::csv::icachedfstream& operator >> (capi::csv::icachedfstream& istm, std::string& val)
{
	val = istm.get_delimited_str();

	return istm;
}

inline capi::csv::icachedfstream& operator >> (capi::csv::icachedfstream& istm, capi::csv::sep& val)
{
	istm.set_delimiter(val.get_delimiter(), val.get_escape());

	return istm;
}

template<typename T>
capi::csv::ostringstream& operator << (capi::csv::ostringstream& ostm, const T& val)
{
	if (!ostm.get_after_newline() && ostm.get_delimiter().size()>0)
		ostm.write_char(ostm.get_delimiter()[0]);

	std::string temp = std::to_string(val);
	ostm.escape_and_output(temp);

	ostm.set_after_newline(false);

	return ostm;
}
template<typename T>
capi::csv::ostringstream& operator << (capi::csv::ostringstream& ostm, const T* val)
{
	if (!ostm.get_after_newline() && ostm.get_delimiter().size()>0)
		ostm.write_char(ostm.get_delimiter()[0]);

	std::string temp = std::to_string(*val);
	ostm.escape_and_output(temp);

	ostm.set_after_newline(false);

	return ostm;
}
template<>
inline capi::csv::ostringstream& operator << (capi::csv::ostringstream& ostm, const std::string& val)
{
	if (!ostm.get_after_newline()&& ostm.get_delimiter().size()>0)
		ostm.write_char(ostm.get_delimiter()[0]);

	std::string temp = val;
	ostm.escape_str_and_output(temp);

	ostm.set_after_newline(false);

	return ostm;
}

inline capi::csv::ostringstream& operator << (capi::csv::ostringstream& ostm, const capi::csv::NChar val)
{
	if (!ostm.get_after_newline() && ostm.get_delimiter().size() > 0)
		ostm.write_char(ostm.get_delimiter()[0]);

	std::string temp = std::to_string((int)val.getChar());
	ostm.escape_str_and_output(temp);

	ostm.set_after_newline(false);

	return ostm;
}

template<>
inline capi::csv::ostringstream& operator << (capi::csv::ostringstream& ostm, const capi::csv::sep& val)
{
	ostm.set_delimiter(val.get_delimiter(), val.get_escape());

	return ostm;
}

template<>
inline capi::csv::ostringstream& operator << (capi::csv::ostringstream& ostm, const char& val)
{
	if (val == NEWLINE)
	{
		ostm.write_char(NEWLINE);

		ostm.set_after_newline(true);
	}
	else
	{
		if (!ostm.get_after_newline() && ostm.get_delimiter().size() > 0)
			ostm.write_char(ostm.get_delimiter()[0]);

		std::string temp = "";
		temp += val;
		ostm.escape_str_and_output(temp);

		ostm.set_after_newline(false);
	}

	return ostm;
}
template<>
inline capi::csv::ostringstream& operator << (capi::csv::ostringstream& ostm, const char* val)
{
	const std::string temp = val;

	ostm << temp;

	return ostm;
}

template<typename T>
capi::csv::ocachedfstream& operator << (capi::csv::ocachedfstream& ostm, const T& val)
{
	if (!ostm.get_after_newline() && ostm.get_delimiter().size()>0)
		ostm.write_char(ostm.get_delimiter()[0]);

	std::string temp = std::to_string(val);
	ostm.escape_and_output(temp);

	ostm.set_after_newline(false);

	return ostm;
}

template<typename T>
capi::csv::ocachedfstream& operator << (capi::csv::ocachedfstream& ostm, const T* val)
{
	if (!ostm.get_after_newline() && ostm.get_delimiter().size()>0)
		ostm.write_char(ostm.get_delimiter()[0]);

	std::string temp = std::to_string(*val);
	ostm.escape_and_output(temp);

	ostm.set_after_newline(false);

	return ostm;
}
template<>
inline capi::csv::ocachedfstream& operator << (capi::csv::ocachedfstream& ostm, const std::string& val)
{
	if (!ostm.get_after_newline() && ostm.get_delimiter().size()>0)
		ostm.write_char(ostm.get_delimiter()[0]);

	std::string temp = val;
	ostm.escape_str_and_output(temp);

	ostm.set_after_newline(false);

	return ostm;
}

inline capi::csv::ocachedfstream& operator << (capi::csv::ocachedfstream& ostm, const capi::csv::NChar val)
{
	if (!ostm.get_after_newline() && ostm.get_delimiter().size() > 0)
		ostm.write_char(ostm.get_delimiter()[0]);

	std::string temp = std::to_string((int)val.getChar());
	ostm.escape_str_and_output(temp);

	ostm.set_after_newline(false);

	return ostm;
}

template<>
inline capi::csv::ocachedfstream& operator << (capi::csv::ocachedfstream& ostm, const capi::csv::sep& val)
{
	ostm.set_delimiter(val.get_delimiter(), val.get_escape());

	return ostm;
}

template<>
inline capi::csv::ocachedfstream& operator << (capi::csv::ocachedfstream& ostm, const char& val)
{
	if (val == NEWLINE)
	{
#ifdef WIN32
		ostm.write_char('\r');
#endif
		ostm.write_char(NEWLINE);

		ostm.set_after_newline(true);
	}
	else
	{
		if (!ostm.get_after_newline() && ostm.get_delimiter().size() > 0)
			ostm.write_char(ostm.get_delimiter()[0]);

		std::string temp = "";
		temp += val;
		ostm.escape_str_and_output(temp);

		ostm.set_after_newline(false);
	}

	return ostm;
}
template<>
inline capi::csv::ocachedfstream& operator << (capi::csv::ocachedfstream& ostm, const char* val)
{
	const std::string temp = val;

	ostm << temp;

	return ostm;
}

#endif // CSV_STREAMS_H