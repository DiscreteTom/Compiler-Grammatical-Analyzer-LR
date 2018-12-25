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
	bool operator==(const Project &ano) const { return tIndex == ano.tIndex && candidateIndex == ano.candidateIndex && index == ano.index; }
	bool operator<(const Project &ano) const
	{
		if (tIndex != ano.tIndex)
			return tIndex < ano.tIndex;
		else if (candidateIndex != ano.candidateIndex)
			return candidateIndex < ano.candidateIndex;
		else
			return index < ano.index;
	}
	bool operator!=(const Project &ano) const { return !(*this == ano); }
};

// generate a hash value for Project
inline uint qHash(const Project &key, uint seed)
{
	return qHash(1000 * key.tIndex + 100 * key.candidateIndex + key.index, seed);
}

using State = QVector<Project>;

struct DFA_Key
{
	State state;
	Symbol s;
	bool operator<(const DFA_Key &ano) const
	{
		if (state.size() != ano.state.size())
			return state.size() < ano.state.size();
		else
		{
			for (int i = 0; i < state.size(); ++i)
			{
				if (state[i] != ano.state[i])
					return state[i] < ano.state[i];
			}
		}
		return s < ano.s;
	}
	bool operator==(const DFA_Key &ano) { return state == ano.state && s == ano.s; }
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
	QVector<State> states;
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
	void outputProject(const Project &p) const;

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
