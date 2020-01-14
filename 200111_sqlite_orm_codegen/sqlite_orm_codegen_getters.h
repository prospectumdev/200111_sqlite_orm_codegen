#pragma once


static int getTableParameters(void* data, int argc, char** argv, char** azColName)
{
	vector<TableParameters>& temp = *((vector<TableParameters>*) data);

	if (string(argv[0]) == "table")
	{
		temp.push_back({ string(argv[0]), string(argv[1]), string(argv[2]), atoi(argv[3]), string(argv[4]) });
	}

	return 0;
};

static int getColumnInfo(void* data, int argc, char** argv, char** azColName)
{
	vector<ColumnInfo>& temp = *((vector<ColumnInfo>*) data);

	ColumnInfo ci;

	ci.cid = atoi(argv[0]);
	ci.name = string(argv[1]);
	ci.type = string(argv[2]);
	ci.notnull = atoi(argv[3]);
	argv[4] ? ci.dflt_value = string(argv[4]) : ci.dflt_value = "";
	ci.pk = atoi(argv[5]);

	temp.push_back(ci);
	return 0;
};

//column metadata does not reqire a callback function

static int getForeignKey(void* data, int argc, char** argv, char** azColName)
{
	TableParameters& temp = *((TableParameters*)data);

	ForeignKey fk = { atoi(argv[0]), atoi(argv[1]), string(argv[2]), string(argv[3]),
		string(argv[4]), string(argv[5]), string(argv[6]), string(argv[7]) };

	temp.foreignKeys.push_back(fk);
	return 0;
};

static int getIndexListItem(void* data, int argc, char** argv, char** azColName)
{
	vector<IndexlistItem>& temp = *((vector<IndexlistItem>*)data);

	IndexlistItem i = { atoi(argv[0]), string(argv[1]),  
		                atoi(argv[2]), string(argv[3]), 
		                atoi(argv[4]) };

	temp.push_back(i);
	return 0;
};

static int getIndexInfo(void* data, int argc, char** argv, char** azColName)
{
	IndexlistItem& temp = *((IndexlistItem*)data);

	temp.indexInfo.seqno = atoi(argv[0]);
	temp.indexInfo.cid   = atoi(argv[1]);
	temp.indexInfo.name  = string(argv[2]);

	return 0;
};