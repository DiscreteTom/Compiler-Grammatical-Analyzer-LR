#pragma once

#include <QVector>
#include <QString>
#include <QHash>

struct Symbol
{
	enum SymbolType
	{
		T, // terminator
		NT // non-terminator
	};
	SymbolType type;
	int index; // index in SymbolTable

	bool operator==(const Symbol &ano) const { return ano.type == type && ano.index == index; }
	bool operator!=(const Symbol &ano) const { return !(*this == ano); }
};

inline uint qHash(const Symbol &key, uint seed)
{
	return qHash(key.index * (key.type == Symbol::SymbolType::T ? 1 : -1), seed);
}

template <bool isTerminatorTable>
class SymbolTable
{
private:
	Symbol::SymbolType type;
	QVector<QString> symbols;

public:
	SymbolTable();
	int getIndex(const QString &str);							// if str not exist, push it into symbols
	int getIndex(const QString &str, bool) const; // return -1 if str not exist
	QString getStr(int i) const;									// return blank QString if i is invalid
	int size() const { return symbols.size(); }
};

const Symbol EPSILON = {Symbol::SymbolType::T, 0};
const Symbol END = {Symbol::SymbolType::T, 1};

using T_Table = SymbolTable<true>;
using NT_Table = SymbolTable<false>;

template <>
inline SymbolTable<true>::SymbolTable()
{
	getIndex("~"); // insert EPSILON to terminatorTable
	getIndex("$"); // insert END to terminatorTable
}

template <bool b>
SymbolTable<b>::SymbolTable() {}

template <bool b>
int SymbolTable<b>::getIndex(const QString &str)
{
	for (int i = 0; i < symbols.size(); ++i)
	{
		if (symbols[i] == str) // str exist
			return i;
	}
	// str does NOT exist, add it to table
	symbols.push_back(str);
	return symbols.size() - 1;
}

template <bool b>
int SymbolTable<b>::getIndex(const QString &str, bool) const
{
	for (int i = 0; i < symbols.size(); ++i)
	{
		if (symbols[i] == str) // str exist
			return i;
	}
	return -1;
}

template <bool b>
QString SymbolTable<b>::getStr(int i) const
{
	if (i >= 0 && i < symbols.size())
		return symbols[i];
	else
		return "";
}
