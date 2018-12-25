#pragma once

#include "symbol.h"
#include <vector>
#include <set>
#include <map>

using namespace std;

using Candidate = vector<Symbol>;
using Gramma = vector<Candidate>;
using First = set<Symbol>;
using Follow = set<Symbol>;

bool operator==(const Candidate &c1, const Candidate &c2);

struct TableItem // item in predict analysis table M
{
	int ntIndex = -1;				 // non-terminator index in grammas
	int candidateIndex = -1; // candidate index in grammas[ntIndex]
	bool operator==(const TableItem &ano) const { return ntIndex == ano.ntIndex && candidateIndex == ano.candidateIndex; }
};

struct MapKey
{
	int ntIndex;
	int tIndex;
	bool operator<(const MapKey &ano) const { return ntIndex + tIndex * 100 < ano.ntIndex + ano.tIndex * 100; }
};

class GrammaTable
{
private:
	// input
	vector<Gramma> grammas;
	T_Table tTable;
	NT_Table ntTable;

	// error handling
	int lineCount;
	bool error;

	// process
	vector<First> firsts;
	vector<Follow> follows;
	map<MapKey, TableItem> M; // predict analysis table

	void killBlank(string &str) const; // discard blank chars
	bool format(string &str) const;		 // return false if format is wrong
	/**
	 * killDuplicated:
	 * eliminate same Candidate in grammas[index] if index != -1
	 * eliminate same Candidate in each Gramma when index == -1
	 */
	void killDuplicated(int index = -1);
	void killExplicitLeftRecursion(int index);
	void killEpsilon();
	void killLeftRecursion();
	void getFirsts();
	First getFirst(const Candidate &candidate) const;
	void getFollows();
	bool getM();
	Candidate parseInputToCandidate(const string &str) const; // return empty candidate if error
	void outputSingleCandidate(int ntIndex, int candidateIndex) const;

public:
	GrammaTable() : lineCount(0), error(false){};

	int insert(const string &grammaLine); // return 0 if ok, otherwise return lineCount
	bool generate();											// return false if error
	void output() const;

	bool ok() const { return !error; }
	int currentLineCount() const { return lineCount; }
	bool parse(const string &str) const;
};
