#pragma once

static void queryTableParameters(sqlite3* db, vector<TableParameters>& tables)
{
	char* zErrMsg = 0;
	string query = "SELECT * FROM SQLITE_MASTER ORDER BY name ASC";
	auto rc = sqlite3_exec(db, query.c_str(), getTableParameters, (void*)(&tables), &zErrMsg);
	sqlite3_free(zErrMsg);
	if (rc != SQLITE_OK)
	{
		fprintf(stderr, "Couldn't get TableParameters\n");
		exit(-2);
	}
}
static void queryColumnInfos(sqlite3* db, TableParameters& table)
{
	char* zErrMsg = 0;
	string query = "PRAGMA table_info(" + table.name + ")";
	auto rc = sqlite3_exec(db, query.c_str(), getColumnInfo, (void*) & (table.columnInfos), &zErrMsg);
	sqlite3_free(zErrMsg);
	if (rc != SQLITE_OK)
	{
		fprintf(stderr, "Couldn't get ColumnInfos\n");
		exit(-3);
	}
}
static void queryColumnMetadata(sqlite3* db, TableParameters& table)
{
	for (auto& column : table.columnInfos)
	{
		//get metadata of each column
		string zDbName = "NULL";   /* Database name or NULL */
		string zTableName = table.name;  /* Table name */
		string zColumnName = column.name; /* Column name */

		auto& md = column.columnMetadata;

		int rc = sqlite3_table_column_metadata(db, nullptr, zTableName.c_str(), zColumnName.c_str(),
			&md.pzDataType, &md.pzCollSeq, &md.notNull, &md.primaryKey, &md.autoinc);
		//TODO: catch (rc != SQLITE_OK)
	}
}
static void queryForeignKeys(sqlite3* db, TableParameters& table)
{
#ifdef _use_foreign_keys_
	char* zErrMsg = 0;
	string query = "PRAGMA foreign_key_list(" + table.name + ")";
	int rc = sqlite3_exec(db, query.c_str(), getForeignKey, (void*) & (table), &zErrMsg);
	sqlite3_free(zErrMsg);
	if (rc != SQLITE_OK)
	{
		fprintf(stderr, "Couldn't get ForeignKeys\n");
		exit(-5);
	}
#endif
}
static void queryIndexList(sqlite3* db, TableParameters& table)
{
	char* zErrMsg = 0;
	string query = "PRAGMA index_list(" + table.name + ")";
	auto rc = sqlite3_exec(db, query.c_str(), getIndexListItem, (void*) & (table.indexList), &zErrMsg);
	sqlite3_free(zErrMsg);
	if (rc != SQLITE_OK)
	{
		fprintf(stderr, "Couldn't get ColumnInfos\n");
		exit(-6);
	}
}
static void queryIndexInfos(sqlite3* db, TableParameters& table)
{
	for (IndexlistItem& ii : table.indexList)
	{
		char* zErrMsg = 0;
		string query = "PRAGMA index_info(" + ii.name + ")";
		int rc = sqlite3_exec(db, query.c_str(), getIndexInfo, (void*) & (ii), &zErrMsg);
		sqlite3_free(zErrMsg);
		if (rc != SQLITE_OK)
		{
			fprintf(stderr, "Couldn't get tableInfos\n");
			exit(-7);
		}
	}
}
