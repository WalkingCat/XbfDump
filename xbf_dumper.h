#pragma once
#include "xbf_data.h"

class xbf_dumper
{
private:
	const xbf_data& m_data;

	std::wstring get_node_value(const xbf_node_value* node);
	std::wstring get_node_text(const xbf_node* node);

	struct xaml_node {
		std::wstring name;
		std::vector<std::tuple<std::wstring, std::wstring>> namespaces;
		std::vector<std::tuple<std::wstring, std::wstring>> attributes;
		std::vector<xaml_node> children;
	};
	xaml_node build_xaml_node(size_t& i);
	void dump_xaml_node(const xaml_node& node, uint32_t indent);
public:
	xbf_dumper(const xbf_data& data);
	void dump_header();
	void dump_tables();
	void dump_nodes();
	void dump_xaml();
};

