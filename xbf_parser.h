#pragma once
#include "easy_file.h"
#include "xbf_data.h"

class xbf_parser
{
private:
	easy_file m_file;
	xbf_data m_data;

	bool read_node(xbf_node& node);
	bool read_string(std::wstring* s);
public:
	xbf_parser(easy_file&& file);

	enum parse_result {
		result_none = 0x00,
		result_header = 0x01,
		result_metadata = 0x02,
		result_nodes = 0x04,
		result_all = 0x07,
	};
	parse_result parse(std::string& error);
	bool read_nodes_v1();

	const xbf_data& get_data() { return m_data; }
};

