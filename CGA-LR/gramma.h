#pragma once

#include "symbol.h"
#include <QVector>
#include <QSet>
#include <QMap>

using Candidate = QVector<Symbol>;
using Gramma = QVector<Candidate>;
using First = QSet<Symbol>;
using Follow = QSet<Symbol>;

struct Project
{
	int tIndex;
	int candidateIndex;
	int index; // position of Point in a project
	bool operator==(const Project &ano)const{return tIndex == ano.tIndex && candidateIndex == ano.candidateIndex && index == ano.index;}
};

// generate a hash value for Project
inline uint qHash(const Project &key, uint seed)
{
	return qHash(key.tIndex * 1000 + key.candidateIndex * 100 + key.index, seed);
}

using State = QSet<Project>;

struct DFA_Key
{
	State state;
	Symbol s;
};

using DFA = QMap<DFA_Key, State>;

class GrammaTable
{
private:
	// input
	QVector<Gramma> grammas;
	T_Table tTable;
	NT_Table ntTable;

	// error handling
	int lineCount;
	bool error;

	// process
	QVector<First> firsts;
	QVector<Follow> follows;
	DFA dfa;

	void killBlank(QString &str) const; // discard blank chars
	bool format(QString &str) const;		// return false if format is wrong
	/**
	 * killDuplicated:
	 * eliminate same Candidate in grammas[index] if index != -1
	 * eliminate same Candidate in each Gramma when index == -1
	 */
	void killDuplicated(int index = -1);
	void killEpsilon();
	void getFirsts();
	First getFirst(const Candidate &candidate) const;
	void getFollows();
	void getDFA();							 // construct DFA
	void getState(State &state); // construct a state
	int getIndex(int ntIndex, int candidateIndex) const;
	int candidateCount() const;
	Candidate parseInputToCandidate(const QString &str) const; // return empty candidate if error
	void outputSingleCandidate(int ntIndex, int candidateIndex) const;

public:
	GrammaTable() : lineCount(0), error(false) {}

	int insert(const QString &grammaLine); // return 0 if ok, otherwise return lineCount
	/**
	 * generate first set, follow set, index of candidates and predict table
	 * return false if error
	 */
	bool generate();
	void output() const;

	bool ok() const { return !error; }
	int currentLineCount() const { return lineCount; }
	bool parse(const QString &str) const;
};
