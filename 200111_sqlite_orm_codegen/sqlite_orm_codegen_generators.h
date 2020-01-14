#pragma once

void transformTypeForOrmStruct(string& type)
{
	char chars[] = " ()0123456789";
	for (unsigned int i = 0; i < strlen(chars); ++i)
	{
		type.erase(std::remove(type.begin(), type.end(), chars[i]), type.end());
	}
	std::for_each(type.begin(), type.end(), [](char& c) {
		c = ::tolower(c); });

	//TODO: Add all possible types
	map<string, string> typeLUT{
		{ "blob", "vector<char>" },
		{ "character", "char" },
		{ "double", "double" },
		{ "int", "int" },
		{ "integer", "int" },
		{ "real", "double" },
		{ "text", "string" },
		{ "varchar", "string" },
		{ "unsignedint", "unsigned int" },
	};

	if (typeLUT.count(type) == 1)
	{
		type = typeLUT[type];
	}
	else
	{
		type = "!!! ERROR: UNKNOWN TYPE '" + type + "' !!!";
		exit(-100);
	}
}
static string generateOrmStructDefinition(TableParameters tableParameters)
{
	string o;
	o += "struct " + tableParameters.name + "\n{\n";

	for (auto columnInfo : tableParameters.columnInfos)
	{
		//transform type
		string type = columnInfo.type;
		transformTypeForOrmStruct(type);

		//make it a unique_ptr if it is allowed to be NULL
		if (!columnInfo.notnull) type = "unique_ptr<" + type + ">";

		//write column specification
		o += "\t" + type + " " + columnInfo.name + ";\n";
	}

	o += "};\n";
	return o;
};
static string generateOrmTableDefinition(vector<TableParameters> tables, string schemaName = "DB")
{
	string o;

	o += "auto init" + schemaName + "(const string& path)\n{\n";
	o += "\treturn make_storage(path,\n";

	for (auto table : tables)
	{
		o += "\t\tmake_table\n\t\t(\n";
		o += "\t\t\t\"" + table.name + "\",\n";
		for (auto column : table.columnInfos)
		{
			vector<string> columnAttributes;
			if (column.dflt_value != "") //default???;
			{
				string transformedType = column.type;
				transformTypeForOrmStruct(transformedType);
				//adding "" to string type most likey correct but untested
				//if (transformedType =="string")
				//	column.dflt_value = "\"" + column.dflt_value + "\"";
				if (transformedType == "char")
				{
					column.dflt_value.erase(std::remove(column.dflt_value.begin(), column.dflt_value.end(), '\''), column.dflt_value.end());
					column.dflt_value = "\"" + column.dflt_value + "\"";
				}
				//columnAttributes.push_back("default_value<" + transformedType + ">(" + column.dflt_value + ")");
				columnAttributes.push_back("default_value(" + column.dflt_value + ")");
			}
			if (column.columnMetadata.autoinc)
				columnAttributes.push_back("autoincrement()");
			
			//primary key definition is added separately after column definitions
			//so it is omitted here
			//if (column.pk)
			//	columnAttributes.push_back("primary_key()");
			
			string colName = column.name;
			bool columnIsUniqueIndex = false;
			for (auto index : table.indexList)
			{
				if (index.unique && index.indexInfo.name == colName)
					columnIsUniqueIndex = true;
			}
			if (columnIsUniqueIndex)
			{
				columnAttributes.push_back("unique()");
			}
			
			string preparedColumnAttributes;
			if (columnAttributes.size() > 0) preparedColumnAttributes += ",";
				for (auto columnAttribute : columnAttributes)
				{
					preparedColumnAttributes += " " + columnAttribute;
					if (columnAttribute != columnAttributes.back())
						preparedColumnAttributes += ",";
				}
			string comma;
			if (table.columnInfos.size() > 0)
				if (column.name != table.columnInfos[table.columnInfos.size() - 1].name || table.foreignKeys.size() > 0)
					comma = ",";
			o += "\t\t\tmake_column(\"" + column.name + "\", &" + table.name + "::" + column.name + preparedColumnAttributes + ")" + comma + "\n";

		}

#ifdef _use_foreign_keys_
		for (auto foreignKey : table.foreignKeys)
		{
			string comma;
			if (table.columnInfos.size() > 0)
				if (foreignKey.id != table.foreignKeys[table.foreignKeys.size() - 1].id)
					comma = ",";
			o += "\t\t\tforeign_key(&" + table.name + "::" + foreignKey.from + ").references(&" + foreignKey.table + "::" + foreignKey.to + ")" + comma + "\n";
		}
#endif

		//create primary key statement
		vector<string> primaryKeys;
		for (auto column : table.columnInfos)
		{
			if (column.pk) primaryKeys.push_back(column.name);
		}
		if (primaryKeys.size() > 0)
		{
			o += "\t\t\t,primary_key(";
			for (auto i = 0; i < primaryKeys.size(); i++)
			{
				o += "&"+table.name + "::" + primaryKeys[i];
				if (i < primaryKeys.size() - 1)
					o += ", ";
			}
			o +=")\n";
		}

		string comma;
		if (tables.size() > 0)
			if (table.name != tables.back().name)
				comma = ",";
		o += "\t\t)" + comma + "\n";
	}
	o += "\t);\n";
	o += "}\n";
	o += "using " + schemaName + " = decltype(init" + schemaName + "(\"\"));\n";
	return o;
};
