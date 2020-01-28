#pragma once

void transformTypeForOrmStruct(string& type)
{
	char chars[] = " ()0123456789";
	for (unsigned int i = 0; i < strlen(chars); ++i)
	{
		type.erase(std::remove(type.begin(), type.end(), chars[i]), type.end());
	}
	std::for_each(type.begin(), type.end(), [](char& c) {c = ::tolower(c); });

	//TODO: Add all possible types
	map<string, string> typeLUT{
		{ "blob",		 "vector<char>" },
		{ "character",   "string" },
		{ "double",		 "double" },
		{ "int",		 "int" },
		{ "integer", 	 "int" },
		{ "real",		 "double" },
		{ "text",		 "string" },
		{ "varchar",	 "string" },
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

	o += "static "+ schemaName +" init" + schemaName + "(const string& path)\n{\n";
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

				if (transformedType == "string" && column.dflt_value.size() > 2)
				{
					//column.dflt_value.erase(std::remove(column.dflt_value.begin(), column.dflt_value.end(), '\''), column.dflt_value.end());
					string temp = column.dflt_value.substr(1, column.dflt_value.size() - 2);
					column.dflt_value = "\"" + temp + "\"";
				}

				if (transformedType != "string")
					columnAttributes.push_back("default_value<"+ transformedType +">(" + column.dflt_value + ")");
				else //no type definition for strings
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
	//o += "using " + schemaName + " = decltype(init" + schemaName + "(\"\"));\n";
	return o;
};
static string generateOrmStorageTypeDefinition(vector<TableParameters> tables, string schemaName = "DB")
{
	string o;

	o += "template<class O, class T, class ...Op>\n";
	o += "using Column = internal::column_t<O, T, const T & (O::*)() const, void (O::*)(T), Op...>;\n\n";
	
	o += "using " + schemaName + " = internal::storage_t <\n";

	for (auto table : tables)
	{
		o += "\tinternal::table_t < " + table.name + ",\n";
		for (auto column : table.columnInfos)
		{
			o += "\t\tColumn< " + table.name;
			o += ", decltype(" + table.name + "::" + column.name + ")";
			
			//constraints
			{
				string constraints;
				if (column.dflt_value != "")
				{
					string transformedType = column.type;
					transformTypeForOrmStruct(transformedType);
					if (transformedType == "string") transformedType = "const char*";
					constraints += (", constraints::default_t<"+ transformedType +">");
				}
				if (column.columnMetadata.autoinc)
					constraints += (", constraints::autoincrement_t");

				//primary key definition is added separately after column definitions
				//so it is omitted here
				//if (column.pk)
				//	constraints += (", constraints::primary_key_t<>");
				
				bool columnIsUniqueIndex = false;
				for (auto index : table.indexList)
				{
					if (index.unique && index.indexInfo.name == column.name)
						columnIsUniqueIndex = true;
				}
				if (columnIsUniqueIndex)
				{
					constraints += (", constraints::unique_t");
				}

				o += constraints;
			}
			o += ">";
			if (tables.size() > 0)
				if (column.name != table.columnInfos.back().name)
					o += ",";
			o += "\n";
		}

		//create primary key statement
		vector<string> primaryKeys;
		for (auto column : table.columnInfos)
		{
			if (column.pk) primaryKeys.push_back(column.name);
		}
		if (primaryKeys.size() > 0)
		{
			o += "\t\t, constraints::primary_key_t<";
			for (auto i = 0; i < primaryKeys.size(); i++)
			{
				o += "decltype(&" + table.name + "::" + primaryKeys[i] +")";
				if (i < primaryKeys.size() - 1)
					o += ", ";
			}
			o += ">\n";
		}

		o += "\t>";
		if (tables.size() > 0)
			if (table.name != tables.back().name)
				o += ",";
		o+="\n";
	}
	o += ">;\n";
	return o;
}