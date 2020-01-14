#pragma once

#include <string>
#include <vector>

using namespace std;

struct ColumnMetadata
{
	const char* pzDataType; 
	const char* pzCollSeq; 
	int notNull;
	int primaryKey;
	int autoinc;
};
struct ColumnInfo
{
	int		cid;
	string	name;
	string	type;
	int		notnull;
	string	dflt_value;
	int		pk;

	ColumnMetadata columnMetadata;
};
struct ForeignKey
{
	int id;
	int seq;
	string table;
	string from;
	string to;
	string un_update;
	string on_delete;
	string match;
};
struct IndexInfo
{
	int seqno;
	int cid;
	string name;
};
struct IndexlistItem
{
	int seq;
	string name;
	int unique;
	string origin;
	int partial;

	IndexInfo indexInfo;
};
struct TableParameters
{
	string type;
	string name;
	string tbl_name;
	int rootpage;
	string sql;

	vector<ColumnInfo> columnInfos;
	vector<ForeignKey> foreignKeys;
	vector<IndexlistItem> indexList;
};

