#include "stdafx.h"
#include "xbf_dumper.h"
#include <sstream>
#include <iomanip>

xbf_dumper::xbf_dumper(const xbf_data& data) : m_data(data) {}

void xbf_dumper::dump_header()
{
	auto& header = m_data.header;
	printf_s("\tmagic: %08x %s\n", header.magicNumber, &header.magicNumber);
	printf_s("\tmetadataSize: %u\n", header.metadataSize);
	printf_s("\tnodeSize: %u\n", header.nodeSize);
	printf_s("\tmajorFileVersion: %u\n", header.majorFileVersion);
	printf_s("\tminorFileVersion: %u\n", header.minorFileVersion);
	printf_s("\tstringTableOffset: %I64x\n", header.stringTableOffset);
	printf_s("\tassemblyTableOffset: %I64x\n", header.assemblyTableOffset);
	printf_s("\ttypeNamespaceTableOffset: %I64x\n", header.typeNamespaceTableOffset);
	printf_s("\ttypeTableOffset: %I64x\n", header.typeTableOffset);
	printf_s("\tpropertyTableOffset: %I64x\n", header.propertyTableOffset);
	printf_s("\txmlNamespaceTableOffset: %I64x\n", header.xmlNamespaceTableOffset);
}

void xbf_dumper::dump_tables()
{
	printf_s("Strings Table: %u\n", m_data.strings.size());
	for (size_t i = 0; i < m_data.strings.size(); ++i) {
		printf_s("\t%u: %S\n", i, m_data.strings[i].c_str());
	}

	printf_s("Assemblies Table: %u\n", m_data.assemblies.size());
	for (size_t i = 0; i < m_data.assemblies.size(); ++i) {
		printf_s("\t%u: %u, %u (%S)\n", i, m_data.assemblies[i].providerKind, m_data.assemblies[i].stringId, m_data.strings[m_data.assemblies[i].stringId].c_str());
	}

	printf_s("Type Namespaces Table: %u\n", m_data.type_namespaces.size());
	for (size_t i = 0; i < m_data.type_namespaces.size(); ++i) {
		printf_s("\t%u: %u, %u (%S, %S)\n", i, m_data.type_namespaces[i].assemblyId, m_data.type_namespaces[i].stringId,
				 m_data.strings[m_data.assemblies[m_data.type_namespaces[i].assemblyId].stringId].c_str(),
				 m_data.strings[m_data.type_namespaces[i].stringId].c_str());
	}

	printf_s("Types Table: %u\n", m_data.types.size());
	for (size_t i = 0; i < m_data.types.size(); ++i) {
		printf_s("\t%u: %u, %u, %u (%S, %S)\n", i, m_data.types[i].typeFlags, m_data.types[i].typeNamespaceId, m_data.types[i].stringId,
				 m_data.strings[m_data.type_namespaces[m_data.types[i].typeNamespaceId].stringId].c_str(),
				 m_data.strings[m_data.types[i].stringId].c_str());
	}

	printf_s("Properties Table: %u\n", m_data.properties.size());
	for (size_t i = 0; i < m_data.properties.size(); ++i) {
		printf_s("\t%u: %u, %u, %u (%S, %S)\n", i, m_data.properties[i].propertyFlags, m_data.properties[i].typeId, m_data.properties[i].stringId,
				 m_data.strings[m_data.types[m_data.properties[i].typeId].stringId].c_str(),
				 m_data.strings[m_data.properties[i].stringId].c_str());
	}

	printf_s("Xml Namespaces Table: %u\n", m_data.xml_namespaces.size());
	for (size_t i = 0; i < m_data.xml_namespaces.size(); ++i) {
		printf_s("\t%u: %u (%S)\n", i, m_data.xml_namespaces[i].stringId, m_data.strings[m_data.xml_namespaces[i].stringId].c_str());
	}
}

void xbf_dumper::dump_nodes()
{
	uint32_t indent = 0;
	uint32_t line_num = 0, line_pos = 0;
	std::stack<std::wstring> object_names;

	for (auto& n : m_data.nodes) {
		switch (n->type) {
			case xntLineInfo: {
				auto node = (xbf_node_line_info*)n.get();
				line_num += node->line_num_delta;
				if (node->line_num_delta != 0) {
					line_pos = node->line_pos_delta;
				} else {
					line_pos += node->line_pos_delta;
				}
				printf_s("\n{%4u,%4u} %s", line_num, line_pos, std::string(indent, '\t').c_str());
				break;
			}
			case xntLineInfoAbsolute: {
				auto node = (xbf_node_line_info_abs*)n.get();
				line_num = node->line_num;
				line_pos = node->line_pos;
				printf_s("\n{%4u, %4u} %s", line_num, line_pos, std::string(indent, '\t').c_str());
				break;
			}
			case xntNamespace: {
				auto node = (xbf_node_namespace*)n.get();
				printf_s("xmlns:%S=\"%S\"", node->name.c_str(), m_data.strings[m_data.xml_namespaces[node->node_id].stringId].c_str());
				break;
			}
			case xntStartObject: {
				auto node = (xbf_node*)n.get();
				auto name = m_data.strings[m_data.types[node->node_id].stringId];
				object_names.push(name);
				++indent;
				printf_s("<%S>", name.c_str());
				break;
			}
			case xntEndObject: {
				auto name = object_names.top();
				object_names.pop();
				--indent;
				printf_s("</%S>", name.c_str());
				break;
			}
			case xntStartProperty: {
				auto node = (xbf_node*)n.get();
				printf_s("[%S", m_data.strings[m_data.properties[node->node_id].stringId].c_str());
				break;
			}
			case xntEndProperty:
				printf_s("]");
				break;
			case xntText:
				printf_s("=\"%S\"", get_node_text((xbf_node*)n.get()).c_str());
				break;
			case xntValue:
				printf_s("=\"%S\"", get_node_value((xbf_node_value*)n.get()).c_str());
				break;
			default:
				break;
		}
	}
	printf_s("\n");
}

std::wstring xbf_dumper::get_node_text(const xbf_node* node)
{
	return m_data.strings[node->node_id];
}

std::wstring xbf_dumper::get_node_value(const xbf_node_value* node)
{
	std::wstringstream ss;

	switch (node->value_type) {
		case xnvtBoolFalse:
			ss << L"False";
			break;
		case xnvtBoolTrue:
			ss << L"True";
			break;
		case xnvtFloat:
		case xnvtKeyTime:
		case xnvtLengthConverter:
		case xnvtDuration: {
			auto data = (xbf_node_value_data<float>*) node;
			ss << data->data;
			break;
		}
		case xnvtSigned: {
			auto data = (xbf_node_value_data<int32_t>*) node;
			ss << data->data;
			break;
		}
		case xnvtCString:{
			auto data = (xbf_node_value_data<std::wstring>*) node;
			ss << data->data;
			break;
		}
		case xnvtThickness: {
			auto data = (xbf_node_value_data<std::tuple<float, float, float, float>>*) node;
			ss << std::get<0>(data->data) << L"," << std::get<1>(data->data) << L","
				<< std::get<2>(data->data) << L"," << std::get<3>(data->data);
			break;
		}
		case xnvtGridLength: {
			auto data = (xbf_node_value_data<std::tuple<uint32_t, float>>*) node;
			ss << std::get<0>(data->data) << L"," << std::get<1>(data->data);
			break;
		}
		case xnvtColor: {
			auto data = (xbf_node_value_data<uint32_t>*) node;
			ss.flags(ss.uppercase | ss.hex);
			ss << data->data;
			break;
		}
		default:
			ss << L"ERROR???";
			break;
	}

	return ss.str();
}

xbf_dumper::xaml_node xbf_dumper::build_xaml_node(size_t& i) {
	xaml_node ret = {};
	bool started = false;
	for (; i < m_data.nodes.size(); ++i) {
		switch (m_data.nodes[i]->type) {
			case xntNamespace: {
				auto node = (xbf_node_namespace*)m_data.nodes[i].get();
				auto ns = std::make_tuple(node->name, m_data.strings[m_data.xml_namespaces[node->node_id].stringId]);
				ret.namespaces.push_back(ns);
				break;
			}
			case xntStartObject: {
				if (started) {
					ret.children.push_back(build_xaml_node(i));
				} else {
					auto node = (xbf_node*)m_data.nodes[i].get();
					ret.name = m_data.strings[m_data.types[node->node_id].stringId];
					started = true;
				}
				break;
			}
			case xntEndObject: {
				return ret;
			}
			case xntStartProperty: {
				auto node = (xbf_node*)m_data.nodes[i].get();
				auto name = m_data.strings[m_data.properties[node->node_id].stringId];
				switch (m_data.nodes[i + 1]->type) {
					case xntStartObject: {
						if (name != L"__implicit_items") {
							xaml_node child = {};
							child.name = ret.name + L"." + name;
							child.children.push_back(build_xaml_node(++i));
							ret.children.push_back(child);
						} else ret.children.push_back(build_xaml_node(++i));
						break;
					}
					case xntText: {
						auto attribute = std::make_tuple(name, get_node_text((xbf_node*)m_data.nodes[i + 1].get()));
						ret.attributes.push_back(attribute);
						++i;
						break;
					}
					case xntValue: {
						auto attribute = std::make_tuple(name, get_node_value((xbf_node_value*)m_data.nodes[i + 1].get()));
						ret.attributes.push_back(attribute);
						++i;
						break;
					}
					default:
						break;
				}
				break;
			}
			case xntEndProperty: {
				// 				if (prop_node) return ret;
			}
			default:
				break;
		}
	}
	return ret;
}

void xbf_dumper::dump_xaml()
{
	size_t i = 0;
	dump_xaml_node(build_xaml_node(i), 0);
}

void xbf_dumper::dump_xaml_node(const xaml_node& node, uint32_t indent)
{
	std::string prefix(indent, '\t');
	printf_s("%s<%S", prefix.c_str(), node.name.c_str());
	for (auto& a : node.attributes) {
		printf_s(" %S=\"%S\"", std::get<0>(a).c_str(), std::get<1>(a).c_str());
	}
	for (auto& n : node.namespaces) {
		printf_s("\n%s\txmlns%s%S=\"%S\"", prefix.c_str(), std::get<0>(n).empty() ? "" : ":", std::get<0>(n).c_str(), std::get<1>(n).c_str());
	}
	if (node.children.empty()) printf_s("/>\n");
	else {
		printf_s(">\n");
		for (auto& c : node.children) dump_xaml_node(c, indent + 1);
		printf_s("%s</%S>\n", prefix.c_str(), node.name.c_str());
	}
}


