#include "gramma.h"
#include <QStack>
#include "myutility.h"
#include <iostream>

using namespace std;

void GrammaTable::killBlank(QString &str) const
{
	QString result;
	for (auto c : str)
	{
		if (c != ' ' && c != '\t' && c != '\n')
			result += c;
	}
	str = result;
}

bool GrammaTable::format(QString &str) const
{
	killBlank(str);
	//check format
	int i = str.indexOf('-');
	if (str[i + 1] != '>')
		return false;
	return true;
}

void GrammaTable::killDuplicated(int index)
{
	if (index == -1)
	{
		// eliminate same candidate for all gramma
		for (int i = 0; i < grammas.size(); ++i)
		{
			killDuplicated(i);
		}
	}
	else
	{
		// eliminate same candidate for grammas[index]
		for (int i = 0; i < grammas[index].size(); ++i)
		{
			for (int j = i + 1; j < grammas[index].size(); ++j)
			{
				if (grammas[index][i] == grammas[index][j])
				{
					// eliminate grammas[index][j]
					grammas[index].erase(grammas[index].begin() + j);
					--j;
				}
			}
		}
	}
}

void GrammaTable::killEpsilon()
{
	for (auto &gramma : grammas)
	{
		for (auto &candidate : gramma)
		{
			for (int i = 0; i < candidate.size(); ++i)
			{
				if (candidate[i] == EPSILON && candidate.size() > 1)
				{
					candidate.erase(candidate.begin() + i);
					--i;
				}
			}
		}
	}
}

void GrammaTable::getFirsts()
{
	firsts.clear();
	// add empty set
	for (int i = 0; i < grammas.size(); ++i)
	{
		firsts.push_back(First());
	}

	bool changed = true;
	while (changed)
	{
		changed = false;
		for (int i = 0; i < grammas.size(); ++i) // for each gramma
		{

			for (int j = 0; j < grammas[i].size(); ++j) // for each candidate
			{
				for (auto symbol : grammas[i][j])
				{
					if (symbol.type == Symbol::SymbolType::T)
					{
						// add this terminator, even EPSILON
						if (firsts[i].find(symbol) == firsts[i].end())
						{
							changed = true;
							firsts[i].insert(symbol);
						}
						break; // to next candidate
					}
					else // symbol.type == NT
					{
						for (auto first : firsts[symbol.index])
						{
							if (firsts[i].find(first) == firsts[i].end())
							{
								changed = true;
								firsts[i].insert(first);
							}
						}
						if (firsts[symbol.index].find(EPSILON) == firsts[symbol.index].end())
						{
							break; // to next candidate
						}
					}
				}
			}
		}
	}
}

void GrammaTable::getFollows()
{
	follows.clear();
	// add empty set
	for (int i = 0; i < grammas.size(); ++i)
	{
		follows.push_back(Follow());
	}

	// add END
	follows[0].insert(END);

	bool changed = true;
	while (changed)
	{
		changed = false;
		for (int i = 0; i < grammas.size(); ++i) // for each gramma
		{
			for (int j = 0; j < grammas[i].size(); ++j) // for each candidate
			{
				for (int k = 0; k < grammas[i][j].size(); ++k) // for each symbol
				{
					if (grammas[i][j][k].type == Symbol::SymbolType::NT) // this is an NT
					{
						if (k + 1 < grammas[i][j].size()) // next symbol exist
						{
							if (grammas[i][j][k + 1].type == Symbol::SymbolType::T) // next symbol is a terminator(it will not be END or EPSILON)
							{
								if (follows[grammas[i][j][k].index].find(grammas[i][j][k + 1]) == follows[grammas[i][j][k].index].end())
								{
									changed = true;
									follows[grammas[i][j][k].index].insert(grammas[i][j][k + 1]);
								}
								break; // to next candidate
							}
							else // next symbol is a NT
							{
								bool hasEPSILON = false;
								for (int x = k + 1; x < grammas[i][j].size(); ++x)
								{
									for (auto first : firsts[grammas[i][j][k + 1].index])
									{
										if (first != EPSILON && follows[grammas[i][j][k].index].find(first) == follows[grammas[i][j][k].index].end())
										{
											changed = true;
											follows[grammas[i][j][k].index].insert(first);
										}
										else if (first == EPSILON)
										{
											hasEPSILON = true;
										}
									}
									if (!hasEPSILON)
										break;
								}
								if (hasEPSILON) // the last NT in this candidate has EPSILON in its FIRST
								{
									for (auto follow : follows[i])
									{
										if (follows[grammas[i][j][k].index].find(follow) == follows[grammas[i][j][k].index].end())
										{
											changed = true;
											follows[grammas[i][j][k].index].insert(follow);
										}
									}
								}
							}
						}
						else //next symbol not exist
						{
							for (auto follow : follows[i])
							{
								if (follows[grammas[i][j][k].index].find(follow) == follows[grammas[i][j][k].index].end())
								{
									changed = true;
									follows[grammas[i][j][k].index].insert(follow);
								}
							}
						}
					}
				}
			}
		}
	}
}

First GrammaTable::getFirst(const Candidate &candidate) const
{
	First result;
	for (auto symbol : candidate)
	{
		if (symbol.type == Symbol::SymbolType::T)
		{
			// add this terminator, even EPSILON
			if (result.find(symbol) == result.end())
			{
				result.insert(symbol);
			}
			return result;
		}
		else // symbol.type == NT
		{
			for (auto first : firsts[symbol.index])
			{
				if (result.find(first) == result.end())
				{
					result.insert(first);
				}
			}
			if (firsts[symbol.index].find(EPSILON) == firsts[symbol.index].end())
			{
				return result;
			}
		}
	}
	return result;
}

Candidate GrammaTable::parseInputToCandidate(const QString &str) const
{
	int i = 0;
	Candidate result;
	while (i < str.length())
	{
		QString sym;
		if (str[i] == '$')
		{
			while (i + 1 < str.length() && str[i + 1] != '$')
			{
				++i;
				sym += str[i];
			}
			if (i == str.length() || sym.length() < 2)
			{
				// no matched $
				result.clear();
				return result;
			}
			else // str[i + 1] == $
			{
				++i;
			}
			int index = tTable.getIndex(sym, false);
			if (index == -1)
			{
				result.clear();
				return result;
			}
			result.push_back(Symbol({Symbol::SymbolType::T, index}));
			sym = "";
		}
		else
		{
			// see other chars as terminator
			sym += str[i];
			while (i + 1 < str.length() && str[i] == '\'')
			{
				++i;
				sym += str[i];
			}
			int index = tTable.getIndex(sym, false);
			if (index == -1)
			{
				result.clear();
				return result;
			}
			result.push_back(Symbol({Symbol::SymbolType::T, index}));
		}
		++i;
	}
	return result;
}

int GrammaTable::insert(const QString &grammaLine)
{
	if (error)
		return lineCount;
	++lineCount;
	QString str = grammaLine;
	if (!format(str))
	{
		error = true;
		return lineCount;
	}
	if (!str.length())
	{
		error = true;
		return lineCount; // ERROR
	}

	// get left Symbol QString
	QString left;
	left += str[0];
	int i = 1; // index of str
	while (i < str.length() && str[i] == '\'')
	{
		left += str[i];
		++i;
	}

	// check left Symbol
	int grammaIndex = ntTable.getIndex(left);
	if (grammaIndex == grammas.size()) //new symbol
	{
		grammas.push_back(Gramma());
	}

	// get right
	i += 2; // read "->"
	Candidate candidate;
	QString sym; // current symbol QString
	while (i < str.length())
	{
		if (str[i] >= 'A' && str[i] <= 'Z')
		{
			sym += str[i];
			while (i + 1 < str.length() && str[i + 1] == '\'')
			{
				++i;
				sym += str[i];
			}
			//find this NT
			int index = ntTable.getIndex(sym);
			if (index == grammas.size()) // new NT
			{
				grammas.push_back(Gramma());
			}
			candidate.push_back({Symbol::SymbolType::NT, index});
			sym = "";
		}
		else if (str[i] == '$')
		{
			++i;
			while (i < str.length() && str[i] != '$')
			{
				sym += str[i];
				++i;
			}
			if (str[i] != '$')
			{
				// no matched $
				error = true;
				return lineCount;
			}
			else
			{
				// $ matched
				if (sym.length() < 2)
				{
					error = true;
					return lineCount;
				}
				else
				{
					// got a terminator
					int index = tTable.getIndex(sym);
					candidate.push_back({Symbol::SymbolType::T, index});
					sym = "";
				}
			}
		}
		else if (str[i] == '|')
		{
			if (candidate.size())
			{
				grammas[grammaIndex].push_back(candidate);
				candidate.clear();
			}
		}
		else
		{
			//other characters, inlcude '~', see them as terminator
			sym = str[i];
			while (i + 1 < str.length() && str[i + 1] == '\'')
			{
				sym += str[i + 1];
				++i;
			}
			int index = tTable.getIndex(sym);
			candidate.push_back({Symbol::SymbolType::T, index});
			sym = "";
		}
		++i;
	}
	if (candidate.size())
	{
		grammas[grammaIndex].push_back(candidate);
	}
	killDuplicated(grammaIndex);
	return 0;
}

bool GrammaTable::generate()
{
	if (error)
		return false;
	getFirsts();
	getFollows();
	return true;
}

void GrammaTable::outputSingleCandidate(int ntIndex, int candidateIndex) const
{
	cout << ntTable.getStr(ntIndex) << " -> ";
	for (auto symbol : grammas[ntIndex][candidateIndex])
	{
		if (symbol.type == Symbol::SymbolType::NT)
		{
			cout << ntTable.getStr(symbol.index);
		}
		else
		{
			cout << tTable.getStr(symbol.index);
		}
	}
}

void GrammaTable::output() const
{
	if (error)
	{
		cout << "Can NOT parse gramma to LR gramma\n";
		return;
	}

	cout << "Format gramma:\n";
	for (int i = 0; i < grammas.size(); ++i)
	{
		if (grammas[i].size())
			cout << ntTable.getStr(i) << " -> ";
		for (int j = 0; j < grammas[i].size(); ++j)
		{
			// each candidate
			for (int k = 0; k < grammas[i][j].size(); ++k)
			{
				// each symbol
				if (grammas[i][j][k].type == Symbol::SymbolType::NT)
				{
					cout << ntTable.getStr(grammas[i][j][k].index);
				}
				else // type == T
				{
					cout << tTable.getStr(grammas[i][j][k].index);
				}
			}
			if (j != grammas[i].size() - 1)
				cout << " | ";
		}
		if (grammas[i].size())
			cout << endl;
	}
	cout << endl;

	cout << "First sets:\n";
	for (int i = 0; i < firsts.size(); ++i)
	{
		cout << "First(" << ntTable.getStr(i) << "): ";
		for (auto first : firsts[i])
		{
			cout << tTable.getStr(first.index) << " ";
		}
		cout << "\n";
	}
	cout << endl;

	cout << "Follow sets:\n";
	for (int i = 0; i < firsts.size(); ++i)
	{
		cout << "Follow(" << ntTable.getStr(i) << "): ";
		for (auto follow : follows[i])
		{
			cout << tTable.getStr(follow.index) << " ";
		}
		cout << "\n";
	}
	cout << endl;
}

bool GrammaTable::parse(const QString &str) const
{
	return false;
}
