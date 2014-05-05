#pragma once
#include <cstdint>

#pragma pack(push, 1)
struct xbf_header
{
	uint32_t magicNumber;
	uint32_t metadataSize;
	uint32_t nodeSize;
	uint32_t majorFileVersion;
	uint32_t minorFileVersion;
	uint64_t stringTableOffset;
	uint64_t assemblyTableOffset;
	uint64_t typeNamespaceTableOffset;
	uint64_t typeTableOffset;
	uint64_t propertyTableOffset;
	uint64_t xmlNamespaceTableOffset;
	uint16_t hash[32];
};

struct xbf_assembly {
	uint32_t providerKind;
	uint32_t stringId;
};

struct xbf_type_namespace {
	uint32_t assemblyId;
	uint32_t stringId;
};

struct xbf_type {
	uint32_t typeFlags;
	uint32_t typeNamespaceId;
	uint32_t stringId;
};

struct xbf_property {
	uint32_t propertyFlags;
	uint32_t typeId;
	uint32_t stringId;
};

struct xbf_xml_namespace {
	uint32_t stringId;
};

#pragma pack(pop)

enum xbf_node_type : uint8_t {
	xntNone,
	xntStartObject,
	xntEndObject,
	xntStartProperty,
	xntEndProperty,
	xntText,
	xntValue,
	xntNamespace,
	xntEndOfAttributes,
	xntEndOfStream,
	xntLineInfo,
	xntLineInfoAbsolute
};

struct xbf_node_data {
	const xbf_node_type type;
	xbf_node_data(xbf_node_type _type) : type(_type) {}
};

struct xbf_node_line_info : xbf_node_data {
	xbf_node_line_info() : xbf_node_data(xntLineInfo) {}
	int16_t line_num_delta;
	int16_t line_pos_delta;
};

struct xbf_node_line_info_abs : xbf_node_data {
	xbf_node_line_info_abs() : xbf_node_data(xntLineInfoAbsolute) {}
	uint32_t line_num;
	uint32_t line_pos;
};

struct xbf_node : xbf_node_data {
	xbf_node(xbf_node_type _type) : xbf_node_data(_type) {}
	uint32_t node_id;
	uint32_t node_flags;
};

struct xbf_node_namespace : xbf_node {
	xbf_node_namespace() : xbf_node(xntNamespace) {}
	std::wstring name;
};

enum xbf_node_value_type : uint8_t {
	xnvtNone,
	xnvtBoolFalse,
	xnvtBoolTrue,
	xnvtFloat,
	xnvtSigned,
	xnvtCString,
	xnvtKeyTime,
	xnvtThickness,
	xnvtLengthConverter,
	xnvtGridLength,
	xnvtColor,
	xnvtDuration
};

struct xbf_node_value : xbf_node_data {
	const xbf_node_value_type value_type;
	xbf_node_value(xbf_node_value_type _value_type) : xbf_node_data(xntValue), value_type(_value_type) {}
};

template<typename T>
struct xbf_node_value_data : xbf_node_value {
	xbf_node_value_data(xbf_node_value_type _value_type) : xbf_node_value(_value_type) {}
	T data;
};

struct xbf_data {
	xbf_header header;
	std::vector<std::wstring> strings;
	std::vector<xbf_assembly> assemblies;
	std::vector<xbf_type_namespace> type_namespaces;
	std::vector<xbf_type> types;
	std::vector<xbf_property> properties;
	std::vector<xbf_xml_namespace> xml_namespaces;
	std::vector<std::unique_ptr<xbf_node_data>> nodes;
};