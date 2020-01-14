//preprocessor directives for configuration

//#define _test_
//#define _use_foreign_keys_

#ifdef _DEBUG
	#pragma comment (lib, "sqlite3d.lib")
#else
	#pragma comment (lib, "sqlite3.lib")
#endif

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <fstream>
#include <map>
using namespace std;

#include "sqlite3.h"

#include "sqlite_orm_codegen_datastructures.h"
#include "sqlite_orm_codegen_getters.h"
#include "sqlite_orm_codegen_queries.h"
#include "sqlite_orm_codegen_generators.h"


#ifdef _test_
	#include "myDB.h"
#endif

sqlite3* openDB(string fn)
{
	sqlite3* db;
	char* zErrMsg = 0;
	int rc;
	rc = sqlite3_open(fn.c_str(), &db);
	if (rc)
	{
		fprintf(stderr, "ERROR: Can't open database: %s\n", sqlite3_errmsg(db));
		exit(-1);
	}
	printf("DB open!\n");
	return db;
}
void readDBStructure(sqlite3 *db, vector<TableParameters>& tables)
{
	queryTableParameters(db, tables);
	//tables.resize(1); //debug
	for (auto& table : tables)
	{
		queryColumnInfos(db, table);
		queryColumnMetadata(db, table);
		queryForeignKeys(db, table); //can be switched off by undefining _use_foreign_keys_
		queryIndexList(db, table);
		queryIndexInfos(db, table);
	}
}
void writeHeaderFile(string& fnHeader, vector<TableParameters>& tables, string& schemaName)
{
	//now output the obtained schema in sqlite_orm format
	ofstream h(fnHeader);
	h << "#pragma once\n\n";
	h << "#include <string>\n";
	h << "#include <vector>\n";
	h << "using namespace std;\n\n";
	h << "#include \"sqlite_orm.h\"\nusing namespace sqlite_orm;\n\n";
	h << "namespace db\n{\n";
	for (auto table : tables)
	{
		h << generateOrmStructDefinition(table);
	}
	h << generateOrmTableDefinition(tables, schemaName);
	h << "}";
	h.close();
}

int main()
{
	//read db structure and generate sqlite_orm header file
#ifndef _test_
	//inits
	string fnSourceDB("preexistingDatabase.sqlite3");
	string fnHeader("myDB.h");
	string schemaName("myDB");

	//obtain db structure
	static vector<TableParameters> tables;
	auto db = openDB(fnSourceDB);
	readDBStructure(db, tables);
	sqlite3_close(db);

	//generate header file
	writeHeaderFile(fnHeader, tables, schemaName);
#endif

	//create a test db using the generated header file
#ifdef _test_
	string fnTestDB("newDatabase.sqlite3");
	unique_ptr<db::myDB>  testdb = make_unique<db::myDB>(db::initmyDB(fnTestDB));
	testdb->sync_schema(true);
#endif
}